/*
 * 1394-Based Digital Camera Display Program
 *
 * Written by Alexnader Skoglund <alexander.skoglund@member.fsf.org>
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

#include <time.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/extensions/Xvlib.h>
//#include <X11/keysym.h>
#define _GNU_SOURCE_
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#include <gsl/gsl_math.h> // Shuffle function
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
/* SDL and OpenGL */

#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#include <SDL.h>

//#include "tools.h"
#include "dc1394/dc1394.h"
//#include "globals.h"
//#include "displayvideo.h"
#include "questionare.h"
#include "conversion.h"

/* OpenGL defs*/


/* uncomment the following to drop frames to prevent delays */
//#define MAX_PORTS   4
#define MAX_CAMERAS 8
#define NUM_BUFFERS 8

#define NUM_Q 10

// ok the following constant should be by right included thru in Xvlib.h 
#ifndef XV_YV12
#define XV_YV12 0x32315659
#endif

#ifndef XV_YUY2
#define XV_YUY2 0x32595559
#endif

#ifndef XV_UYVY
#define XV_UYVY 0x59565955
#endif


void cleanup(uint32_t lnumCameras, dc1394camera_t **cam, char * fb) {
  int i;
  for (i=0; i < lnumCameras; i++) {
    dc1394_video_set_transmission(cam[i], DC1394_OFF);
    dc1394_capture_stop(cam[i]);
  }
  if (fb != NULL)
  free( fb );

}


int questions(int num, SDL_Surface* screen){

  int h = (screen->h)-20;
  switch(num){
  case 0:
    glColor3f( 1.0f , 1.0f , 1.0f );
    glPrintStereo(screen->w, screen->h, 0 , h,     "A. Right now I feel..", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-20,  "   1. Extremely dissatisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-40,  "   2. Mostly dissatisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-60,  "   3. Moderately dissatisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-80,  "   4. Slightly dissatisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-100, "   5. Neither dissatisfied nor satisfied with my physical", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-120, "      appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-140, "   6. Slightly satisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-160, "   7. Moderately satisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-180, "   8. Mostly satisfied with my physical appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-200, "   9. Extremely satisfied with my physical appearance", 0 );    
    break;
  case 1:
    glColor3f( 1.0f , 0.0f , 0.0f );
    glPrintStereo(screen->w, screen->h, 0 , h,     "B. Right now I feel..", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-20,  "   1. Extremely dissatisfied with my virtual appearance", 0 );
    glPrintStereo(screen->w, screen->h, 0 , h-40,  "   2. Mostly dissatisfied with my mental appearance", 0 );
    //  default:
      
  }

}

int main(int argc, char *argv[])
{
  int  i, j;


  /* declarations for libdc1394 */
  uint32_t numCameras = 0;
  dc1394camera_t *cameras[MAX_CAMERAS];
  //dc1394featureset_t features;
  
  /* declarations for video1394 */
  char *device_name=NULL;
  
  long format=0;

  /* Other declarations */
  unsigned long frame_length;
  long frame_free;
  int frame=0;

  unsigned long width,height;
  unsigned long device_width,device_height;

  int fps;
  int res;

  char *lfb=NULL;
  dc1394video_frame_t * frames[MAX_CAMERAS];
  GLuint video_texture[2];			// This is a handle to our texture object

  float mid = 1.0;
  float stretch = 1.0;

  SDL_Surface *screen;
  int done;
  Uint8 *keys;

  
  dc1394_t * d;
  dc1394camera_list_t * list;

  //bool video = false;
  int video = 0;


  fps = DC1394_FRAMERATE_30;
  res = DC1394_VIDEO_MODE_640x480_RGB8;
  device_width=640;
  device_height=480;
  format=XV_YUY2;

  // Camera setup
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
    DC1394_ERR_CLN_RTN(err,cleanup(numCameras,cameras,lfb),"Could not set ISO speed");

    err=dc1394_video_set_mode(cameras[i], res);
    DC1394_ERR_CLN_RTN(err,cleanup(numCameras,cameras,lfb),"Could not set video mode");

    err=dc1394_video_set_framerate(cameras[i], fps);
    DC1394_ERR_CLN_RTN(err,cleanup(numCameras,cameras,lfb),"Could not set framerate");

    err=dc1394_capture_setup(cameras[i],NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,cleanup(numCameras,cameras,lfb),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    err=dc1394_video_set_transmission(cameras[i], DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup(numCameras,cameras,lfb),"Could not start camera iso transmission");

    // Camera settings 
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_SHUTTER,1400);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_BRIGHTNESS,800);
    err = dc1394_feature_set_value(cameras[i],DC1394_FEATURE_EXPOSURE,150);
    err = dc1394_feature_whitebalance_set_value(cameras[i],500,400);

  }

  fflush(stdout);
  if (numCameras < 1) {
    perror("no cameras found :(\n");
    cleanup(numCameras,cameras,lfb);
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

  SDLinit(screen->w, screen->h);
  initTexture(video_texture,2);
  reshape(screen->w, screen->h);
  done = 0;
  int drawVideo = 1;

  while ( ! done ) {
    SDL_Event event;

    for (i = 0; i < numCameras; i++) {
      if (dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &frames[i])!=DC1394_SUCCESS)
	dc1394_log_error("Failed to capture from camera %d", i);
    }


    if( drawVideo == 1)
      drawTexture(screen->w, screen->h, device_width, device_height, stretch, video_texture, frames[0]->image, frames[1]->image);
    else
      drawBlank();

    for (i = 0; i < numCameras; i++) {
      if (frames[i])
	dc1394_capture_enqueue (cameras[i], frames[i]);
    }
    
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
	case SDL_KEYDOWN:
	switch(event.key.keysym.sym){
	  case SDLK_b:
	    if( drawVideo == 1)
	      drawVideo = 0;
	    else
	      drawVideo = 1;
          break;
	  }
	break;
      case SDL_QUIT:
	done = 1;
	cleanup(numCameras,cameras,lfb);
	break;
      }
    }

    keys = SDL_GetKeyState(NULL);

    if ( keys[SDLK_ESCAPE] ) {
      done = 1;
      cleanup(numCameras,cameras,lfb);
    }
    if ( keys[SDLK_0] ) {
      stretch = mid;
      printf("Strech 1.0: %f\n", stretch);
    }
    if ( keys[SDLK_8] ) {
      stretch = 0.8*mid;
      printf("Strech 0.8: %f\n", stretch);
    }
    if ( keys[SDLK_9] ) {
      stretch = 0.9*mid;
      printf("Strech 0.9: %f\n", stretch);
    }
    if ( keys[SDLK_1] ) {
      stretch = 1.1*mid;
      printf("Strech 1.1: %f\n", stretch);
    }
    if ( keys[SDLK_2] ) {
      stretch = 1.2*mid;
      printf("Strech 1.2: %f\n", stretch);
    }
    if ( keys[SDLK_UP] ) {
      stretch = 1.0;
      mid = stretch;
      printf("Reset to 1.0");
    }
    if ( keys[SDLK_DOWN] ) {
      mid = stretch;
      printf("Normal is: %f\n", mid);
    }
    if ( keys[SDLK_LEFT] ) {
      stretch = stretch + 0.01;
    }
    if ( keys[SDLK_RIGHT] ) {
      stretch = stretch - 0.01;
    }
    if ( keys[SDLK_q] ) {
      done = 1;      
      cleanup(numCameras,cameras,lfb);
    }
   
    SDL_GL_SwapBuffers();

  }
  // Clean up our textures
  glDeleteTextures( NUM_TEXTURES, &video_texture[0] );
  initGLFont();
  done = 0;

  const gsl_rng_type * T;
  gsl_rng * r;
  int  n = 10;
  double mu = 3.0;
  struct timeval tv;
     
  gsl_rng_env_setup();
  gettimeofday(&tv, NULL); 
  gsl_rng_default_seed = tv.tv_usec;

  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
     
  int a[NUM_Q];
  for (i = 0; i < NUM_Q; i++)
      a[i] = i;
  
  gsl_ran_shuffle (r, a, NUM_Q, sizeof (int));
  gsl_rng_free (r);

  // Open data file for printing
  time_t t = tv.tv_sec;
  //  struct tm* lt = 

  char time_str[256];
  char buffer[30];
  strftime(buffer,30,"%Y-%m-%d_%T",localtime(&t));
  sprintf(time_str, "data/qlog_%s.log",buffer);

  FILE *fp;
  
  fp = fopen(time_str,"w"); // open for writing

  j=0;
  while ( ! done ) {
    SDL_Event event;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    questions(a[j],screen);

    
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
      case SDL_KEYDOWN:
	/* Check the SDLKey values and move change the coords */
	switch( event.key.keysym.sym ){
	case SDLK_1:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 1);
	  break;
	case SDLK_2:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 2);
	  break;
	case SDLK_3:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 3);
	  break;
	case SDLK_4:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 4);
	  break;
	case SDLK_5:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 5);
	  break;
	case SDLK_6:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 6);
	  break;
	case SDLK_7:
	  fprintf(fp,"Question number %d response is %d\n", a[j], 7);
	  break;

	default:
	  break;
	}
	break;

      }
    }

   keys = SDL_GetKeyState(NULL);

    if ( keys[SDLK_ESCAPE] ) {
      done = 1;
    }
    if ( keys[SDLK_RETURN] ) {
      j++;
      usleep(200000);       
    }
    if(j >= NUM_Q)
      done = 1;
    SDL_GL_SwapBuffers();
  }

  fclose(fp); /* close the file before ending program */


  SDL_Quit();
  return 0;             /* ANSI C requires main to return int. */

}

