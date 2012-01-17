/*
 * 1394-Based Digital Camera Control Library
 *
 * Written by Damien Douxchamps <ddouxchamps@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>
#include <X11/keysym.h>
#define _GNU_SOURCE_
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

/* SDL and OpenGL */

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL.h>


#include "dc1394/dc1394.h"

/* OpenGL defs*/

#define IMAGE_WIDTH 2*640
#define IMAGE_HEIGHT 480

/* uncomment the following to drop frames to prevent delays */
#define MAX_PORTS   4
#define MAX_CAMERAS 8
#define NUM_BUFFERS 8

/* ok the following constant should be by right included thru in Xvlib.h */
#ifndef XV_YV12
#define XV_YV12 0x32315659
#endif

#ifndef XV_YUY2
#define XV_YUY2 0x32595559
#endif

#ifndef XV_UYVY
#define XV_UYVY 0x59565955
#endif

/* declarations for opengl */
GLuint g_texture[2];			// This is a handle to our texture object

/* declarations for libdc1394 */
uint32_t numCameras = 0;
dc1394camera_t *cameras[MAX_CAMERAS];
dc1394featureset_t features;
dc1394video_frame_t * frames[MAX_CAMERAS];

/* declarations for video1394 */
char *device_name=NULL;

/* declarations for Xwindows */
Display *display=NULL;
Window window=(Window)NULL;

unsigned long width,height;
unsigned long device_width,device_height;
int connection=-1;
XvImage *xv_image=NULL;
XvAdaptorInfo *info;
long format=0;
GC gc;


/* Other declarations */
unsigned long frame_length;
long frame_free;
int frame=0;
int adaptor=-1;

int freeze=0;
int average=0;
int fps;
int res;
char *frame_buffer=NULL;


static struct option long_options[]={
	{"device",1,NULL,0},
	{"fps",1,NULL,0},
	{"res",1,NULL,0},
	{"pp",1,NULL,0},
	{"help",0,NULL,0},
	{NULL,0,0,0}
};



float g_fstretch = 1.0;
float g_mid = 1.0;

void get_options(int argc,char *argv[])
{
  int option_index=0;
  fps=7;
  res=0;

  while(getopt_long(argc,argv,"",long_options,&option_index)>=0){
    if(optarg){
      switch(option_index){
	/* case values must match long_options */
      case 0:
	device_name=strdup(optarg);
	break;
      case 1:
	fps=atoi(optarg);
	break;
      case 2:
	res=atoi(optarg);
	break;
      case 3:
	g_mid=atof(optarg);
	break;
      }
    }
    if(option_index==4){
      printf( "\n"
	      "        %s - multi-cam monitor for libdc1394 and XVideo\n\n"
	      "Usage:\n"
	      "        %s [--fps=[1,3,7,15,30]] [--res=[0,1,2]] [--device=/dev/video1394/x]\n"
	      "             --fps    - frames per second. default=7,\n"
	      "                        30 not compatible with --res=2\n"
	      "             --res    - resolution. 0 = 320x240 (default),\n"
	      "                        1 = 640x480 YUV4:1:1, 2 = 640x480 RGB8\n"
	      "             --device - specifies video1394 device to use (optional)\n"
	      "                        default = automatic\n"
	      "             --pp     - specifies the initial scaling (optional)\n"
	      "                        default = 1.0\n"
	      "             --help   - prints this message\n\n"
	      "Keyboard Commands:\n"
	      "        q = quit\n"
	      "        < -or- , = scale -50%%\n"
	      "        > -or- . = scale +50%%\n"
	      "        0 = pause\n"
	      "        1 = set framerate to 1.875 fps\n"
	      "        2 = set framerate tp 3.75 fps\n"
	      "        3 = set framerate to 7.5 fps\n"
	      "        4 = set framerate to 15 fps\n"
	      "        5 = set framerate to 30 fps\n"
	      ,argv[0],argv[0]);
      exit(0);
    }
  }

}

/* image format conversion functions */
/* The function iyu12yuy2 takes upp half of the time in execution -> use GPU? */

static inline
void iyu12yuy2 (unsigned char *src, unsigned char *dest, uint32_t NumPixels) {
  int i=0,j=0;
  register int y0, y1, y2, y3, u, v;
  while (i < NumPixels*3/2) {
    u = src[i++];
    y0 = src[i++];
    y1 = src[i++];
    v = src[i++];
    y2 = src[i++];
    y3 = src[i++];

    dest[j++] = y0;
    dest[j++] = u;
    dest[j++] = y1;
    dest[j++] = v;

    dest[j++] = y2;
    dest[j++] = u;
    dest[j++] = y3;
    dest[j++] = v;
  }
}


static inline
void rgb2yuy2 (unsigned char *RGB, unsigned char *YUV, uint32_t NumPixels) {
  int i, j;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  for (i = 0, j = 0; i < 3 * NumPixels; i += 6, j += 4) {
    r = RGB[i + 0];
    g = RGB[i + 1];
    b = RGB[i + 2];
    RGB2YUV (r, g, b, y0, u0 , v0);
    r = RGB[i + 3];
    g = RGB[i + 4];
    b = RGB[i + 5];
    RGB2YUV (r, g, b, y1, u1 , v1);
    YUV[j + 0] = y0;
    YUV[j + 1] = (u0+u1)/2;
    YUV[j + 2] = y1;
    YUV[j + 3] = (v0+v1)/2;
  }
}

// helper functions 

void set_frame_length(unsigned long size, int numCameras)
{
  frame_length=size;
  dc1394_log_debug("Setting frame size to %ld kb",size/1024);
  frame_free=0;
  frame_buffer = malloc( size * numCameras);
}

void display_frames()
{
  uint32_t i;

  if(!freeze && adaptor>=0){
    for (i = 0; i < numCameras; i++) {
      if (!frames[i])
	continue;
      switch (res) {
      case DC1394_VIDEO_MODE_640x480_YUV411:
	iyu12yuy2( frames[i]->image,
		   (unsigned char *)(frame_buffer + (i * frame_length)),
		   (device_width*device_height) );
	break;

      case DC1394_VIDEO_MODE_320x240_YUV422:
      case DC1394_VIDEO_MODE_640x480_YUV422:
	memcpy( frame_buffer + (i * frame_length),
		frames[i]->image, device_width*device_height*2);
	break;

      case DC1394_VIDEO_MODE_640x480_RGB8:
	rgb2yuy2( frames[i]->image,
		  (unsigned char *) (frame_buffer + (i * frame_length)),
		  (device_width*device_height) );
	break;
      }
    }


    xv_image=XvCreateImage(display,info[adaptor].base_id,format,frame_buffer,
			   device_width,device_height* numCameras);

    //xv_image=XvCreateImage(display,info[adaptor].base_id,format,frame_buffer,
    //			   device_width, device_height* numCameras);

    XvPutImage(display,info[adaptor].base_id,window,gc,xv_image,
	       0,0,device_width ,device_height * numCameras,
	       0,0,width,height);
    
    /*        XvPutImage(display,info[adaptor].base_id,window,gc,xv_image,
	      0,0,device_width * numCameras, device_height ,
	      0,0,width, height);*/
    /*XvPutImage(display,info[adaptor].base_id,window,gc,xv_image,
      0,0,device_width , device_height * numCameras,
      0,0,width, height);*/
	   
    xv_image=NULL;
  }
}

/*
void QueryXv()
{
  uint32_t num_adaptors;
  int num_formats;
  XvImageFormatValues *formats=NULL;
  int i,j;
  char xv_name[5];

  XvQueryAdaptors(display,DefaultRootWindow(display),&num_adaptors,&info);

  for(i=0;i<num_adaptors;i++) {
    formats=XvListImageFormats(display,info[i].base_id,&num_formats);
    for(j=0;j<num_formats;j++) {
      xv_name[4]=0;
      memcpy(xv_name,&formats[j].id,4);
      if(formats[j].id==format) {
	dc1394_log_error("using Xv format 0x%x %s %s",formats[j].id,xv_name,(formats[j].format==XvPacked)?"packed":"planar");
	if(adaptor<0)adaptor=i;
      }
    }
  }
  XFree(formats);
  if(adaptor<0)
    dc1394_log_error("No suitable Xv adaptor found");

}
*/

void cleanup(void) {
  int i;
  for (i=0; i < numCameras; i++) {
    dc1394_video_set_transmission(cameras[i], DC1394_OFF);
    dc1394_capture_stop(cameras[i]);
  }
  /*if ((void *)window != NULL)
    XUnmapWindow(display,window);
    if (display != NULL)
    XFlush(display);*/
  if (frame_buffer != NULL)
    free( frame_buffer );

}


// trap ctrl-c
/*
void signal_handler( int sig) {
  signal( SIGINT, SIG_IGN);
  cleanup();
  exit(0);
}
*/

/*
int mainold(int argc,char *argv[])
{
  XEvent xev;
  XGCValues xgcv;
  long background=0x010203;
  int i, j;
  dc1394_t * d;
  dc1394camera_list_t * list;

  get_options(argc,argv);
  // process options 
  switch(fps) {
  case 1: fps =        DC1394_FRAMERATE_1_875; break;
  case 3: fps =        DC1394_FRAMERATE_3_75; break;
  case 15: fps = DC1394_FRAMERATE_15; break;
  case 30: fps = DC1394_FRAMERATE_30; break;
  case 60: fps = DC1394_FRAMERATE_60; break;
  default: fps = DC1394_FRAMERATE_7_5; break;
  }
  switch(res) {
  case 1:
    res = DC1394_VIDEO_MODE_640x480_YUV411;
    device_width=640;
    device_height=480;
    format=XV_YUY2;
    break;
  case 2:
    res = DC1394_VIDEO_MODE_640x480_RGB8;
    device_width=640;
    device_height=480;
    format=XV_YUY2;
    break;
  case 3:
    res = DC1394_VIDEO_MODE_800x600_YUV422;
    device_width=800;
    device_height=600;
    format=XV_UYVY;
    break;
  default:
    res = DC1394_VIDEO_MODE_320x240_YUV422;
    device_width=320;
    device_height=240;
    format=XV_UYVY;
    break;
  }

  dc1394error_t err;

  d = dc1394_new ();
  if (!d)
    return 1;
  err=dc1394_camera_enumerate (d, &list);
  DC1394_ERR_RTN(err,"Failed to enumerate cameras");

  if (list->num == 0) {
    dc1394_log_error("No cameras found");
    return 1;
  }

  j = 0;
  for (i = 0; i < list->num; i++) {
    if (j >= MAX_CAMERAS)
      break;
    cameras[j] = dc1394_camera_new (d, list->ids[i].guid);
    if (!cameras[j]) {
      dc1394_log_warning("Failed to initialize camera with guid %llx", list->ids[i].guid);
      continue;
    }
    j++;
  }
  numCameras = j;
  dc1394_camera_free_list (list);

  if (numCameras == 0) {
    dc1394_log_error("No cameras found");
    exit (1);
  }

  // setup cameras for capture 
  for (i = 0; i < numCameras; i++) {
      
    //err=dc1394_video_set_iso_speed(cameras[i], DC1394_ISO_SPEED_400);
    //dc1394_video_set_operation_mode(my_camera_ptr[i], DC1394_OPERATION_MODE_1394B);
    err= dc1394_video_set_operation_mode(cameras[i], DC1394_OPERATION_MODE_1394B);
    err=dc1394_video_set_iso_speed(cameras[i], DC1394_ISO_SPEED_800);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set ISO speed");

    err=dc1394_video_set_mode(cameras[i], res);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set video mode");

    err=dc1394_video_set_framerate(cameras[i], fps);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set framerate");

    err=dc1394_capture_setup(cameras[i],NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    err=dc1394_video_set_transmission(cameras[i], DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not start camera iso transmission");

    // Camera settings
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_SHUTTER,1400);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_BRIGHTNESS,800);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_EXPOSURE,150);
    err = dc1394_feature_whitebalance_set_value(cameras[i],500,400);

  }

  fflush(stdout);
  if (numCameras < 1) {
    perror("no cameras found :(\n");
    cleanup();
    exit(-1);
  }

  switch(format){
  case XV_YV12:
    set_frame_length(device_width*device_height*3/2, numCameras);
    break;
  case XV_YUY2:
  case XV_UYVY:
    set_frame_length(device_width*device_height*2, numCameras);
    break;
  default:
    dc1394_log_error("Unknown format set (internal error)");
    exit(255);
  }

  // make the window 
  display=XOpenDisplay(getenv("DISPLAY"));
  if(display==NULL) {
    dc1394_log_error("Could not open display \"%s\"",getenv("DISPLAY"));
    cleanup();
    exit(-1);
  }

  QueryXv();

  if ( adaptor < 0 ) {
    cleanup();
    exit(-1);
  }

  width=device_width;
  height=device_height * numCameras;
  //width=device_width * numCameras;
  //height=device_height;

  window=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,width,height,0,
			     WhitePixel(display,DefaultScreen(display)),
			     background);

  XSelectInput(display,window,StructureNotifyMask|KeyPressMask);
  XMapWindow(display,window);
  connection=ConnectionNumber(display);

  gc=XCreateGC(display,window,0,&xgcv);

  // main event loop 
  while(1){

    for (i = 0; i < numCameras; i++) {
      if (dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &frames[i])!=DC1394_SUCCESS)
	dc1394_log_error("Failed to capture from camera %d", i);
    }

    display_frames();
    XFlush(display);

    while(XPending(display)>0){
      XNextEvent(display,&xev);
      switch(xev.type){
      case ConfigureNotify:
	width=xev.xconfigure.width;
	height=xev.xconfigure.height;
	display_frames();
	break;
      case KeyPress:
	switch(XKeycodeToKeysym(display,xev.xkey.keycode,0)){
	case XK_q:
	case XK_Q:
	  cleanup();
	  exit(0);
	  break;
	case XK_comma:
	case XK_less:
	  //width=width/2;
	  //height=height/2;
	  width--;
	  XResizeWindow(display,window,width,height);
	  display_frames();
	  break;
	case XK_period:
	case XK_greater:
	  //width=2*width;
	  //height=2*height;
	  width++;
	  XResizeWindow(display,window,width,height);
	  display_frames();
	  break;
	case XK_0:
	  freeze = !freeze;
	  break;
	case XK_1:
	  fps =        DC1394_FRAMERATE_1_875;
	  for (i = 0; i < numCameras; i++)
	    dc1394_video_set_framerate(cameras[i], fps);
	  break;
	case XK_2:
	  fps =        DC1394_FRAMERATE_3_75;
	  for (i = 0; i < numCameras; i++)
	    dc1394_video_set_framerate(cameras[i], fps);
	  break;
	case XK_4:
	  fps = DC1394_FRAMERATE_15;
	  for (i = 0; i < numCameras; i++)
	    dc1394_video_set_framerate(cameras[i], fps);
	  break;
	case XK_5:
	  fps = DC1394_FRAMERATE_30;
	  for (i = 0; i < numCameras; i++)
	    dc1394_video_set_framerate(cameras[i], fps);
	  break;
	case XK_3:
	  fps = DC1394_FRAMERATE_7_5;
	  for (i = 0; i < numCameras; i++)
	    dc1394_video_set_framerate(cameras[i], fps);
	  break;
	}
	break;
      }
    } // XPending 

    for (i = 0; i < numCameras; i++) {
      if (frames[i])
	dc1394_capture_enqueue (cameras[i], frames[i]);
    }

  } // while not interrupted 

  exit(0);
}
*/

static void SDLinit(int width, int height)
{

  glEnable( GL_TEXTURE_2D );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glViewport( 0, 0, width, height );
  glClear( GL_COLOR_BUFFER_BIT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glGenTextures(1, &g_texture[0]);
  glGenTextures(1, &g_texture[1]);
  //Tstart = SDL_GetTicks();

}

void initTexture(void) //int w, int h)
{
  //nothing
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);

  int i;
  for( i = 0; i<2;i++){
    //glBindTexture(GL_TEXTURE_2D, texture[i]);
    glGenTextures(1, &g_texture[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //printf ("Image width %d, height %d\n",gframe[i]->size[0],gframe[i]->size[1]);
      

    /*   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w,
	 h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
	 gframe[i]->image); */

    glClear( GL_COLOR_BUFFER_BIT );

  }

}

static void
idle(void)
{
  usleep(1000); // 1 ms
  //angle += 2.0;
}

/* new window size or exposure */
static void
reshape(int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;

  glViewport(0, 0, (GLint) width, (GLint) height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -40.0);
}


static void drawTexture(int width, int height)
{


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  
  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  int i;
  int frame_ready = 1;
  GLfloat p0,p1;

  /*GLint t0 = SDL_GetTicks();
    for(i =0; i <2; i++)
    {
    frame_ready = 0;
    if ( (dc1394_capture_dequeue(my_camera_ptr[i], gpolicy, &gframe[i]) == DC1394_SUCCESS)) 
    frame_ready = 1;
    else
    printf("Error when in dc1394_capture_dequeue");         
    }
  */

  for (i = 0; i < numCameras; i++) {
    if (!frames[i])
      continue;
    switch (res) {
    case DC1394_VIDEO_MODE_640x480_YUV411:
      iyu12yuy2( frames[i]->image,
		 (unsigned char *)(frame_buffer + (i * frame_length)),
		 (device_width*device_height) );
      break;
      
    case DC1394_VIDEO_MODE_320x240_YUV422:
    case DC1394_VIDEO_MODE_640x480_YUV422:
      memcpy( frame_buffer + (i * frame_length),
	      frames[i]->image, device_width*device_height*2);
      break;
      
    case DC1394_VIDEO_MODE_640x480_RGB8:
      rgb2yuy2( frames[i]->image,
		(unsigned char *) (frame_buffer + (i * frame_length)),
		(device_width*device_height) );
      break;
    }
    
    for (i = 0; i < numCameras; i++) {
      if(frame_ready == 1)
	{
	  glBindTexture( GL_TEXTURE_2D, g_texture[i] );
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gframe[i]->size[0],gframe[i]->size[1],0, GL_RGB, GL_UNSIGNED_BYTE, gframe[i]->image);
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, device_width,device_height,0, GL_RGB, GL_UNSIGNED_BYTE, frames[i]->image);	    
	  if(i ==0)
	    {
	      if( g_fstretch > 1.0)
		{
		  p0 = (g_fstretch -1.0)/2.0;
		  p1 = 1.0 - (g_fstretch -1.0)/2.0;
		  // Draw a textured quad	
		  glBegin(GL_QUADS);
		  glTexCoord2f(p0, 0.0f); glVertex2f(0.0f, 0.0f);
		  glTexCoord2f(p1, 0.0f); glVertex2f((GLfloat)width/2, 0.0f);
		  glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)width/2, (GLfloat)height );
		  glTexCoord2f(p0, 1.0f); glVertex2f(0.0f, (GLfloat)height );
		  glEnd();
		}
	      if( g_fstretch <= 1.0)
		{
		  float pix = g_fstretch*width/2.0;
		  p0 = width/2.0-pix; //0.0+(GLfloat)gstretch;
		  p1 = pix; //(GLfloat)width/2-(GLfloat)gstretch;
		  // Draw a textured quad
		  glBegin(GL_QUADS);	
		  glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
		  glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
		  glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)height );
		  glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)height );
		  glEnd();

		}
	    }
	  else
	    {	      
	      if( g_fstretch > 1.0)
		{
		  p0 = (g_fstretch -1.0)/2.0;
		  p1 = 1.0 - (g_fstretch -1.0)/2.0;
		  // Draw a textured quad	
		  glBegin(GL_QUADS);
		  glTexCoord2f(p0, 0.0f); glVertex2f((GLfloat)width/2, 0.0f);
		  glTexCoord2f(p1, 0.0f); glVertex2f((GLfloat)width, 0.0f);
		  glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)width, (GLfloat)height );
		  glTexCoord2f(p0, 1.0f); glVertex2f((GLfloat)width/2, (GLfloat)height );
		  glEnd();
		}
	      if( g_fstretch <= 1.0)
		{
		  float pix = g_fstretch*width/2.0;
		  p0 = (GLfloat)width/2+width/2.0-pix; //0.0+(GLfloat)gstretch;
		  p1 = (GLfloat)width/2+pix; //(GLfloat)width/2-(GLfloat)gstretch;
		  // Draw a textured quad
		  glBegin(GL_QUADS);	
		  glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
		  glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
		  glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)height );
		  glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)height );
		  glEnd();

		}
	    }
	  //dc1394_capture_enqueue(my_camera_ptr[i],gframe[i]); // release frame
	}
     
    }
  
  }

}

int main(int argc, char *argv[])
{

  //int was_init = TTF_WasInit();
 

  SDL_Surface *screen;
  int done;
  Uint8 *keys;


  int i, j;
  dc1394_t * d;
  dc1394camera_list_t * list;

  get_options(argc,argv);
  /* process options */
  switch(fps) {
  case 1: fps =        DC1394_FRAMERATE_1_875; break;
  case 3: fps =        DC1394_FRAMERATE_3_75; break;
  case 15: fps = DC1394_FRAMERATE_15; break;
  case 30: fps = DC1394_FRAMERATE_30; break;
  case 60: fps = DC1394_FRAMERATE_60; break;
  default: fps = DC1394_FRAMERATE_7_5; break;
  }
  switch(res) {
  case 1:
    res = DC1394_VIDEO_MODE_640x480_YUV411;
    device_width=640;
    device_height=480;
    format=XV_YUY2;
    break;
  case 2:
    res = DC1394_VIDEO_MODE_640x480_RGB8;
    device_width=640;
    device_height=480;
    format=XV_YUY2;
    break;
  case 3:
    res = DC1394_VIDEO_MODE_800x600_YUV422;
    device_width=800;
    device_height=600;
    format=XV_UYVY;
    break;
  default:
    res = DC1394_VIDEO_MODE_320x240_YUV422;
    device_width=320;
    device_height=240;
    format=XV_UYVY;
    break;
  }

  dc1394error_t err;

  d = dc1394_new ();
  if (!d)
    return 1;
  err=dc1394_camera_enumerate (d, &list);
  DC1394_ERR_RTN(err,"Failed to enumerate cameras");

  if (list->num == 0) {
    dc1394_log_error("No cameras found");
    return 1;
  }

  j = 0;
  for (i = 0; i < list->num; i++) {
    if (j >= MAX_CAMERAS)
      break;
    cameras[j] = dc1394_camera_new (d, list->ids[i].guid);
    if (!cameras[j]) {
      dc1394_log_warning("Failed to initialize camera with guid %llx", list->ids[i].guid);
      continue;
    }
    j++;
  }
  numCameras = j;
  dc1394_camera_free_list (list);

  if (numCameras == 0) {
    dc1394_log_error("No cameras found");
    exit (1);
  }

  /* setup cameras for capture */
  for (i = 0; i < numCameras; i++) {
    // Set speed to 800 Mbits/s
    err= dc1394_video_set_operation_mode(cameras[i], DC1394_OPERATION_MODE_1394B);
    err=dc1394_video_set_iso_speed(cameras[i], DC1394_ISO_SPEED_800);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set ISO speed");

    err=dc1394_video_set_mode(cameras[i], res);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set video mode");

    err=dc1394_video_set_framerate(cameras[i], fps);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set framerate");

    err=dc1394_capture_setup(cameras[i],NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    err=dc1394_video_set_transmission(cameras[i], DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not start camera iso transmission");

    // Camera settings 
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_SHUTTER,1400);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_BRIGHTNESS,800);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_EXPOSURE,150);
    err = dc1394_feature_whitebalance_set_value(cameras[i],500,400);

  }

  fflush(stdout);
  if (numCameras < 1) {
    perror("no cameras found :(\n");
    cleanup();
    exit(-1);
  }

  switch(format){
  case XV_YV12:
    set_frame_length(device_width*device_height*3/2, numCameras);
    break;
  case XV_YUY2:
  case XV_UYVY:
    set_frame_length(device_width*device_height*2, numCameras);
    break;
  default:
    dc1394_log_error("Unknown format set (internal error)");
    exit(255);
  }

  width=device_width* numCameras;
  height=device_height ;

  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*
  
  //SDL_Surface* screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL | SDL_FULLSCREEN ); // *changed*

  //screen = SDL_SetVideoMode(IMAGE_WIDTH, IMAGE_HEIGHT, 16, SDL_OPENGL|SDL_FULLSCREEN| SDL_HWACCEL ); 
  screen = SDL_SetVideoMode(width, height, 16, SDL_OPENGL|SDL_RESIZABLE| SDL_HWACCEL ); 
  if ( ! screen ) {
    fprintf(stderr, "Couldn't set 300x300 GL video mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  SDL_WM_SetCaption("Gears", "gears");

  //init(argc, argv);
  SDLinit(screen->w, screen->h);
  initTexture();
  reshape(screen->w, screen->h);
  done = 0;
  while ( ! done ) {
    SDL_Event event;

    for (i = 0; i < numCameras; i++) {
      if (dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &frames[i])!=DC1394_SUCCESS)
	dc1394_log_error("Failed to capture from camera %d", i);
    }

    //draw_stuff();
    drawTexture(screen->w, screen->h);

    idle();
    while ( SDL_PollEvent(&event) ) {
      switch(event.type) {
      case SDL_VIDEORESIZE:
	screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 16,
				  SDL_OPENGL|SDL_RESIZABLE);
	if ( screen ) {
	  reshape(screen->w, screen->h);
	} else {
	  /* Uh oh, we couldn't set the new video mode?? */;
	}
	break;

      case SDL_QUIT:
	done = 1;
	break;
      }
    }

    keys = SDL_GetKeyState(NULL);

    if ( keys[SDLK_ESCAPE] ) {
      done = 1;
      cleanup();
    }
    if ( keys[SDLK_0] ) {
      g_fstretch = g_mid;
      printf("Strech 1.0: %f\n", g_fstretch);
    }
    if ( keys[SDLK_8] ) {
      g_fstretch = 0.8*g_mid;
      printf("Strech 0.8: %f\n", g_fstretch);
    }
    if ( keys[SDLK_9] ) {
      g_fstretch = 0.9*g_mid;
      printf("Strech 0.9: %f\n", g_fstretch);
    }
    if ( keys[SDLK_1] ) {
      g_fstretch = 1.1*g_mid;
      printf("Strech 1.1: %f\n", g_fstretch);
    }
    if ( keys[SDLK_2] ) {
      g_fstretch = 1.2*g_mid;
      printf("Strech 1.2: %f\n", g_fstretch);
    }
    if ( keys[SDLK_UP] ) {
      g_fstretch = 1.0;
      g_mid = g_fstretch;
      printf("Reset to 1.0");
    }
    if ( keys[SDLK_DOWN] ) {
      g_mid = g_fstretch;
      printf("Normal is: %f\n", g_mid);
    }
    if ( keys[SDLK_LEFT] ) {
      g_fstretch = g_fstretch + 0.01;
      //printf("Screen width: %d\n", screen->w);
    }
    if ( keys[SDLK_RIGHT] ) {
      g_fstretch = g_fstretch - 0.01;
      // printf("Screen width: %d\n", screen->w);
    }
    //draw();
    //drawTexture(screen->w, screen->h);
    SDL_GL_SwapBuffers();

    for (i = 0; i < numCameras; i++) {
      if (frames[i])
	dc1394_capture_enqueue (cameras[i], frames[i]);
    }

  }

  SDL_Quit();
  return 0;             /* ANSI C requires main to return int. */

}

