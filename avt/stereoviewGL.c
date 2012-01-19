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
//#include "globals.h"
#include "displayvideo.h"
#include "questionare.h"
#include "conversion.h"

/* OpenGL defs*/

//#define IMAGE_WIDTH 2*640
//#define IMAGE_HEIGHT 480
//#define NUM_TEXTURES 2

/* uncomment the following to drop frames to prevent delays */
//#define MAX_PORTS   4
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
//GLuint g_texture[2];			// This is a handle to our texture object

/* declarations for libdc1394 */
uint32_t numCameras = 0;
dc1394camera_t *cameras[MAX_CAMERAS];
//dc1394featureset_t features;


/* declarations for video1394 */
char *device_name=NULL;

/* declarations for Xwindows */
//Display *display=NULL;
//Window window=(Window)NULL;

unsigned long width,height;
unsigned long device_width,device_height;
//int connection=-1;
//XvImage *xv_image=NULL;
//XvAdaptorInfo *info;
long format=0;
//GC gc;


/* Other declarations */
unsigned long frame_length;
long frame_free;
int frame=0;
//int adaptor=-1;

//int freeze=0;
//int average=0;
int fps;
int res;
//char *frame_buffer=NULL;


float g_fstretch = 1.0;
float g_mid = 1.0;

static struct option long_options[]={
  {"device",1,NULL,0},
  {"fps",1,NULL,0},
  {"res",1,NULL,0},
  {"pp",1,NULL,0},
  {"help",0,NULL,0},
  {NULL,0,0,0}
};

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


// helper functions 

/*unsigned long set_frame_length(unsigned long size, int numCameras)
  {
  frame_length=size;
  dc1394_log_debug("Setting frame size to %ld kb",size/1024);
  frame_free=0;
  frame_buffer = malloc( size * numCameras);
  return 
  }*/

/*void set_frame_length(unsigned long size, int numCameras)
  {
  frame_length=size;
  dc1394_log_debug("Setting frame size to %ld kb",size/1024);
  frame_free=0;
  frame_buffer = malloc( size * numCameras);
  }*/


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
  /*  if (frame_buffer != NULL)
    free( frame_buffer );
  */
}

void ncleanup(uint32_t lnumCameras, dc1394camera_t **cam, char * fb) {
  int i;
  for (i=0; i < lnumCameras; i++) {
    dc1394_video_set_transmission(cam[i], DC1394_OFF);
    dc1394_capture_stop(cam[i]);
  }
  /*if ((void *)window != NULL)
    XUnmapWindow(display,window);
    if (display != NULL)
    XFlush(display);*/
  if (fb != NULL)
  free( fb );

  /*if (frame_buffer != NULL)
    free( frame_buffer );
  */
}



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

  //glGenTextures(1, &g_texture[0]);
  // glGenTextures(1, &g_texture[1]);
  //Tstart = SDL_GetTicks();

}

void initTexture(GLuint* tex, int num) //int w, int h)
{

  int i;
  
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);

  for( i = 0; i<2;i++){
    glGenTextures(1, &tex[i]);
    glBindTexture(GL_TEXTURE_2D, tex[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glClear( GL_COLOR_BUFFER_BIT );

  }

}

static void idle(void)
{
  usleep(1000); 
}

// new window size or exposure
static void reshape(int width, int height)
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

void left(GLint w, GLint h, float s)
{
  GLfloat p0,p1;

  if( s > 1.0)
    {
      p0 = (s -1.0)/2.0;
      p1 = 1.0 - (s -1.0)/2.0;
      // Draw a textured quad	
      glBegin(GL_QUADS);
      glTexCoord2f(p0, 0.0f); glVertex2f(0.0f, 0.0f);
      glTexCoord2f(p1, 0.0f); glVertex2f((GLfloat)w/2, 0.0f);
      glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glTexCoord2f(p0, 1.0f); glVertex2f(0.0f, (GLfloat)h );
      glEnd();
    }
  if( s <= 1.0)
    {
      float pix = s*w/2.0;
      p0 = w/2.0-pix; 
      p1 = pix; 
      // Draw a textured quad
      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();
      
    }
  
}

void right(GLint w, GLint h, float s)
{
  GLfloat p0,p1;

  if( s > 1.0)
    {
      p0 = (s -1.0)/2.0;
      p1 = 1.0 - (s -1.0)/2.0;
      // Draw a textured quad	
      glBegin(GL_QUADS);
      glTexCoord2f(p0, 0.0f); glVertex2f((GLfloat)w/2, 0.0f);
      glTexCoord2f(p1, 0.0f); glVertex2f((GLfloat)w, 0.0f);
      glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)w, (GLfloat)h );
      glTexCoord2f(p0, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glEnd();
    }
  if( s <= 1.0)
    {
      float pix = s*w/2.0;
      p0 = (GLfloat)w/2+w/2.0-pix; 
      p1 = (GLfloat)w/2+pix; 
      // Draw a textured quad
      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();
    }
}


static void drawTexture(int width, int height, float s, GLint* tex, unsigned char* data_left, unsigned char* data_right )
{
  int i;
  int frame_ready = 1;
  GLfloat p0,p1;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  
  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glBindTexture( GL_TEXTURE_2D, tex[0] );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, device_width,device_height,0, GL_RGB, GL_UNSIGNED_BYTE, data_left); //frames[i]->image);
  left(width,height, s);
  glBindTexture( GL_TEXTURE_2D, tex[1] );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, device_width,device_height,0, GL_RGB, GL_UNSIGNED_BYTE, data_right); //frames[i]->image);
  right(width,height, s);
  

}



int main(int argc, char *argv[])
{
  char *lfb=NULL;
  dc1394video_frame_t * frames[MAX_CAMERAS];
  GLuint video_texture[2];			// This is a handle to our texture object

  float mid = 1.0;
  float stretch = 1.0;

  SDL_Surface *screen;
  int done;
  Uint8 *keys;

  int i, j;
  dc1394_t * d;
  dc1394camera_list_t * list;

  //bool video = false;
  int video = 0;

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
    DC1394_ERR_CLN_RTN(err,ncleanup(numCameras,cameras,lfb),"Could not set ISO speed");

    err=dc1394_video_set_mode(cameras[i], res);
    DC1394_ERR_CLN_RTN(err,ncleanup(numCameras,cameras,lfb),"Could not set video mode");

    err=dc1394_video_set_framerate(cameras[i], fps);
    DC1394_ERR_CLN_RTN(err,ncleanup(numCameras,cameras,lfb),"Could not set framerate");

    err=dc1394_capture_setup(cameras[i],NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,ncleanup(numCameras,cameras,lfb),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    err=dc1394_video_set_transmission(cameras[i], DC1394_ON);
    DC1394_ERR_CLN_RTN(err,ncleanup(numCameras,cameras,lfb),"Could not start camera iso transmission");

    // Camera settings 
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_SHUTTER,1400);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_BRIGHTNESS,800);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_EXPOSURE,150);
    err = dc1394_feature_whitebalance_set_value(cameras[i],500,400);

  }

  fflush(stdout);
  if (numCameras < 1) {
    perror("no cameras found :(\n");
    ncleanup(numCameras,cameras,lfb);
    exit(-1);
  }

  unsigned long size;
  switch(format){
  case XV_YV12:
    //set_frame_length(device_width*device_height*3/2, numCameras);
    //void set_frame_length(unsigned long size, int numCameras)
    size = device_width*device_height*3/2;
    frame_length=size;
    dc1394_log_debug("Setting frame size to %ld kb",size/1024);
    frame_free=0;
    //    frame_buffer = malloc( size * numCameras);
    lfb = malloc( size * numCameras);
    break;
  case XV_YUY2:
  case XV_UYVY:
    //set_frame_length(device_width*device_height*2, numCameras);
    size = device_width*device_height*3/2;
    frame_length=size;
    dc1394_log_debug("Setting frame size to %ld kb",size/1024);
    frame_free=0;
    //frame_buffer = malloc( size * numCameras);
    lfb = malloc( size * numCameras);

    break;
  default:
    dc1394_log_error("Unknown format set (internal error)");
    exit(255);
  }

  width=device_width* numCameras;
  height=device_height ;
  video = 1;

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
  initTexture(video_texture,2);
  reshape(screen->w, screen->h);
  done = 0;
  while ( ! done ) {
    SDL_Event event;

    for (i = 0; i < numCameras; i++) {
      if (dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &frames[i])!=DC1394_SUCCESS)
	dc1394_log_error("Failed to capture from camera %d", i);
    }

    //draw_stuff();
    if( video == 1 )
      drawTexture(screen->w, screen->h, stretch, video_texture, frames[0]->image, frames[1]->image);
    else
      {      
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glColor3f( 1.0f , 1.0f , 1.0f );
	//glPrintStereo(screen->w, screen->h, (screen->w)/2 , (screen->h)/2, "Is this question visibile and white?", 0 );
	glPrint(screen->w, screen->h, (screen->w)/4 , (screen->h)/2, "A question?", 0 );
	glPrint(screen->w, screen->h, 3*(screen->w)/4 , (screen->h)/2, "A question?", 0 );
      }
    //drawGLQuestion();
	
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
      ncleanup(numCameras,cameras,lfb);
    }
    if ( keys[SDLK_0] ) {
      stretch = g_mid;
      printf("Strech 1.0: %f\n", stretch);
    }
    if ( keys[SDLK_8] ) {
      stretch = 0.8*g_mid;
      printf("Strech 0.8: %f\n", stretch);
    }
    if ( keys[SDLK_9] ) {
      stretch = 0.9*g_mid;
      printf("Strech 0.9: %f\n", stretch);
    }
    if ( keys[SDLK_1] ) {
      stretch = 1.1*g_mid;
      printf("Strech 1.1: %f\n", stretch);
    }
    if ( keys[SDLK_2] ) {
      stretch = 1.2*g_mid;
      printf("Strech 1.2: %f\n", stretch);
    }
    if ( keys[SDLK_UP] ) {
      stretch = 1.0;
      g_mid = stretch;
      printf("Reset to 1.0");
    }
    if ( keys[SDLK_DOWN] ) {
      g_mid = stretch;
      printf("Normal is: %f\n", g_mid);
    }
    if ( keys[SDLK_LEFT] ) {
      stretch = stretch + 0.01;
      //printf("Screen width: %d\n", screen->w);
    }
    if ( keys[SDLK_RIGHT] ) {
      stretch = stretch - 0.01;
      // printf("Screen width: %d\n", screen->w);
    }
    if ( keys[SDLK_q] ) {
      video = 0;
      initGLFont();
    }

    //draw();
    //drawTexture(screen->w, screen->h);
    SDL_GL_SwapBuffers();

    for (i = 0; i < numCameras; i++) {
      if (frames[i])
	dc1394_capture_enqueue (cameras[i], frames[i]);
    }

  }

  /* Clean up our textures */
  //glDeleteTextures( NUM_TEXTURES, &g_texture[0] );
  glDeleteTextures( NUM_TEXTURES, &video_texture[0] );

  SDL_Quit();
  return 0;             /* ANSI C requires main to return int. */

}

