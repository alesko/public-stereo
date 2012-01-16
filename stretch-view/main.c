


#include <stdio.h>
//#include <libdc1394/>
#include <stdlib.h>
//#include <dc1394/linux/control.h>

#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <dc1394/dc1394.h>
//#include <stdint.h>

#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>


// SDL lib
//#ifdef HAVE_SDLLIB
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
//#include "SDLEvent.h"
//#endif

// X11 includes for the simplified XVinfo
#ifdef HAVE_XV
#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>
#include <ctype.h>
#endif

#ifdef HAVE_FFMPEG
#include <ffmpeg/avformat.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/swscale.h>
#endif

//#include "camera.h"

//#define IMAGE_FILE_NAME "Image.pgm"

#include "globals.h"

#include <stdio.h>
#include <libraw1394/raw1394.h>
//#include <libdc1394/dc1394_control.h>
#include <stdlib.h>                   

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define TEX_WIDTH 640 //1024
#define TEX_HEIGHT  480 //1024

#define IMAGE_WIDTH 2*640
#define IMAGE_HEIGHT 480

static GLint Tstart = 0;
static GLint T0 = 0;
static GLint Frames = 0;

GLuint texture[2];			// This is a handle to our texture object
SDL_Surface *surface;	// This surface will tell us the details of the image
GLenum texture_format;
GLint  nOfColors;
long gcount_t=0;
long gcount_n=0;
int gstretch = 0;
float g_fstretch = 1.0;
/*
typedef struct _Camera_Info {
  dc1394camera_t *camera_;
  //dc1394featureset_t feature_set;
  int guid_;
} my_camera_t;
*/

  raw1394handle_t tmp_handle;
  int port_num;
  raw1394handle_t *handles;
  int err;

//static dc1394_cameracapture g_cameracapture;
dc1394video_frame_t* gframe[2];
dc1394video_frame_t* gimage[2];

dc1394capture_policy_t gpolicy = DC1394_CAPTURE_POLICY_WAIT; 
//dc1394capture_policy_t gpolicy = DC1394_CAPTURE_POLICY_POLL;
dc1394camera_t* my_camera_ptr[2];

//int main (int argc, char *argv[])d
/*struct my_camera_t* MyNewCamera(void) {
  
  struct my_camera_t* cam;
  cam=(struct my_camera_t*)malloc(sizeof(my_camera_t));
  return cam;

}
*/

/* libraw1394 executes this when there is a bus reset. We'll just keep it 
   simple and quit */
int reset_handler(raw1394handle_t handle, unsigned int generation)
{
	raw1394_update_generation(handle, generation);
	//g_alldone = 1;
    return 0;
}


void
MyGetCameraData(dc1394camera_t* cam) {
  
  if (cam->bmode_capable>0) {
    // set b-mode and reprobe modes,... (higher fps formats might not be reported as available in legacy mode)
    dc1394_video_set_operation_mode(cam, DC1394_OPERATION_MODE_1394B);
  }
  
  /*if (dc1394_feature_get_all(camera_info, &cam->feature_set)!=DC1394_SUCCESS)
    printf("Could not get camera feature information!");*/
    //Error("Could not get camera feature information!");

  //fprintf(stderr,"Grabbing F7 stuff\n");
  //GetFormat7Capabilities(cam);
  /*
  cam->image_pipe=NULL;
  pthread_mutex_lock(&cam->uimutex);
  cam->want_to_display=0;
  cam->bayer=-1;
  cam->stereo=-1;
  cam->bpp=8;
*/
  //pthread_mutex_unlock(&cam->uimutex);

}


int ieee1394init()
{
  int err;
  //dc1394camera_t** my_camera_ptr;
  
  //my_camera_t* my_camera_ptr;
  dc1394camera_list_t *camera_list;
  int i=0;
  dc1394video_modes_t modes;
  dc1394framerate_t fps;
  dc1394video_frame_t* frame;
  int fps_modes;
  int w,h;
  

  tmp_handle=raw1394_new_handle();
  if (tmp_handle==NULL) {
    //err_window=create_no_handle_window();

    //gtk_widget_show(err_window);
    //gtk_main();
    printf("Not OK\n");
    return(1);
  }
  port_num=raw1394_get_port_info(tmp_handle, NULL, 0);
  raw1394_destroy_handle(tmp_handle);
  printf("OK, found %d ports\n", port_num);

  g_dc1394=dc1394_new();

  //dc1394_t *g_dc1394;
  //err=GetCameraNodes();
  //dc1394camera_t *my_camera_info;
  //my_camera_info=(dc1394camera_t*)malloc(sizeof(dc1394camera_t));
  err=dc1394_camera_enumerate(g_dc1394,&camera_list);
  //  printf("OK, found %d ports in list\n", camera_list->num);
  //  camera_list->num
  for (i=0;i<camera_list->num;i++) {
    printf("0 i %d\n", i);
    //camera_ptr=NewCamera();
    //my_camera_ptr[i] = MyNewCamera();
    //my_camera_ptr[i] = (dc1394camera_t *)malloc(sizeof(dc1394camera_t));   
    //printf("1 i %d\n", i);
    // copy the info in the dc structure into the coriander struct.
    my_camera_ptr[i] = dc1394_camera_new(g_dc1394,camera_list->ids[i].guid);
    dc1394_reset_bus(my_camera_ptr[i]);
    //MyGetCameraData(my_camera_ptr);
    //AppendCamera(my_camera_ptr);
  }
  printf("OK, found %d ports in list\n", camera_list->num);
  
  printf("Debug 0 \n");
  //SetCurrentCamera(g_cameras->camera_info->guid);

  if (err!=DC1394_SUCCESS) {
    printf("Unknown error getting cameras on the bus.\nExiting\n");
    exit(1);
  }
  //handles=(raw1394handle_t*)malloc(port_num*sizeof(raw1394handle_t));

  int port;
  handles=(raw1394handle_t*)malloc(port_num*sizeof(raw1394handle_t));
  // Set bus reset handlers:
  for (port=0;port<port_num;port++) {
    // get a handle to the current interface card
    handles[port]=raw1394_new_handle();
    raw1394_set_port(handles[port],port);
    // set bus reset handler
    raw1394_set_bus_reset_handler(handles[port], reset_handler);
    
    //raw1394_destroy_handle(handle);
  }


  //  g_cameras->camera_info

  for (i=0;i<camera_list->num;i++) {
    err = dc1394_video_get_supported_modes(my_camera_ptr[i], &modes); // How to use this info???
    //err = dc1394_video_get_supported_framerates(g_cameras->camera_info, fps_modes, &fps);
    //err = dc1394_video_set_mode(g_cameras->camera_info, DC1394_VIDEO_MODE_640x480_YUV422);
    dc1394_video_set_operation_mode(my_camera_ptr[i], DC1394_OPERATION_MODE_1394B);
    err = dc1394_video_set_mode(my_camera_ptr[i], DC1394_VIDEO_MODE_640x480_RGB8);
    //err = dc1394_video_set_mode(my_camera_ptr[i], DC1394_VIDEO_MODE_800x600_RGB8);
    err = dc1394_video_set_iso_speed(my_camera_ptr[i],DC1394_ISO_SPEED_800);
    err = dc1394_video_set_framerate(my_camera_ptr[i], DC1394_FRAMERATE_30);
    err = dc1394_feature_set_value(my_camera_ptr[i],DC1394_FEATURE_SHUTTER,1400);
    err = dc1394_feature_set_value(my_camera_ptr[i],DC1394_FEATURE_BRIGHTNESS,800);
    err = dc1394_feature_set_value(my_camera_ptr[i],DC1394_FEATURE_EXPOSURE,150);
    err = dc1394_feature_whitebalance_set_value(my_camera_ptr[i],500,400);
    dc1394_capture_setup(my_camera_ptr[i],10, DC1394_CAPTURE_FLAGS_DEFAULT); //DC1394_CAPTURE_FLAGS_BANDWIDTH_ALLOC);
    dc1394_video_set_transmission(my_camera_ptr[i],DC1394_ON);
  }

  /*
  err = dc1394_video_get_supported_modes(g_cameras->camera_info, &modes); // How to use this info???
  //err = dc1394_video_get_supported_framerates(g_cameras->camera_info, fps_modes, &fps);
  //err = dc1394_video_set_mode(g_cameras->camera_info, DC1394_VIDEO_MODE_640x480_YUV422);
  err = dc1394_video_set_mode(g_cameras->camera_info, DC1394_VIDEO_MODE_640x480_RGB8);
  err = dc1394_video_set_iso_speed(g_cameras->camera_info,DC1394_ISO_SPEED_800);
  err = dc1394_video_set_framerate(g_cameras->camera_info, DC1394_FRAMERATE_30);
  err = dc1394_feature_set_value(g_cameras->camera_info,DC1394_FEATURE_SHUTTER,800);
  dc1394_capture_setup(g_cameras->camera_info,4, DC1394_CAPTURE_FLAGS_DEFAULT); //DC1394_CAPTURE_FLAGS_BANDWIDTH_ALLOC);
  dc1394_video_set_transmission(g_cameras->camera_info,DC1394_ON);
  */

  dc1394_camera_free_list(camera_list);
  printf("1394 is started\n");
  return 0;

}


void ieee1394stop()
{
  int i; 

  //dc1394_video_set_transmission(g_cameras->camera_info,DC1394_OFF);
  for(i=0; i < 2; i++){
    dc1394_video_set_transmission(my_camera_ptr[i],DC1394_OFF);
    dc1394_capture_stop(my_camera_ptr[i]);
  }
  free(handles);
  dc1394_free(g_dc1394);
  //dc1394_iso_release_bandwidth(g_dc1394,DC1394_ISO_SPEED_800 );
  //dc1394_free_iso_channel_and_bandwidth(g_dc1394);//capdev->camera);

  printf("1394 stopped\n");


}

void inittexture(void)
{
    //nothing
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    //glShadeModel(GL_SMOOTH);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //don't need to do a lot of stuff because we're just drawing pixels anyway 
    //it's just video :)  
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);

    int i;
    for( i = 0; i<2;i++){
      //glBindTexture(GL_TEXTURE_2D, texture[i]);
      glGenTextures(1, &texture[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

      //dc1394_dma_single_capture(&camera); //grab a frame
    
      //err = dc1394_capture_dequeue(g_cameras->camera_info, gpolicy, &gframe);
      err = dc1394_capture_dequeue(my_camera_ptr[i], gpolicy, &gframe[i]);
      printf ("Image width %d, height %d\n",gframe[i]->size[0],gframe[i]->size[1]);
      //gimage[i] = (dc1394video_frame_t *)malloc(sizeof(dc1394video_frame_t));   
      //dc1394_dma_single_capture(&g_cameracapture); //grab a frame
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH,
      	   TEX_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
      	   gframe[i]->image); 
      //memcpy(gimage[i]->image,gframe[i]->image,sizeof(gimage[i]->image)); // copy image from frame to texture holder
      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH,
      //		   TEX_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
      //		   gimage[i]->image); 
      //dc1394_dma_done_with_buffer(&camera);	
      dc1394_capture_enqueue(my_camera_ptr[i],gframe[i] ); // release frame
      glClear( GL_COLOR_BUFFER_BIT );

    }
    /*
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        fprintf (stderr, "OpenGL Error: %s\n", errString);
        exit(1);
    }*/

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



static void mySDLinit(int width, int height)
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

  glGenTextures(1, &texture[0]);
  glGenTextures(1, &texture[1]);
  Tstart = SDL_GetTicks();

}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     //printf("%s \n", message);
}

void *getImage( void *id)
{
  int frame_ready = 0;
  float p0,p1;
  int i;
  int width = 800*2;//640*2;
  int height = 600; //480;
  i = (int)id;
  
  if ( (dc1394_capture_dequeue(my_camera_ptr[i], gpolicy, &gframe[i]) == DC1394_SUCCESS)) 
    frame_ready = 1;
  else
    printf("Error when in dc1394_capture_dequeue");
  if(frame_ready == 1)
    {
      glBindTexture( GL_TEXTURE_2D, texture[i] );
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gframe[i]->size[0],gframe[i]->size[1],0, GL_RGB, GL_UNSIGNED_BYTE, gframe[i]->image);
      if(i ==0)
	{
	  p0 = 0.0;
	  p1 = (float)width/2;
	}
      else
	{
	  p0 = (float)width/2;
	  p1 = (float)width;
	}
      
      // Draw a textured quad	
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(p0, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(p1 , 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f(p1 , height );
      glTexCoord2f(0.0f, 1.0f); glVertex2f(p0, height );
      glEnd();
      dc1394_capture_enqueue(my_camera_ptr[i],gframe[i]); // release frame
    }

}



static void drawTexture(int width, int height)
{

  pthread_t thread1, thread2;
  //char *message1 = "Thread 1";
  //char *message2 = "Thread 2";
  int  iret1, iret2;
  int id;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  //gluOrtho2D(0, width , height, 0);
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  
  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  int i;
  int frame_ready = 0;
  GLfloat p0,p1;

  // Create independent threads each of which will execute function 
  //iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
  //iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);
  /*id = 0;
  iret1 = pthread_create( &thread1, NULL, getImage, (void*) id);
  id = 1;
  iret1 = pthread_create( &thread1, NULL, getImage, (void*) id);
  // Wait till threads are complete before main continues. Unless we  
  // wait we run the risk of executing an exit which will terminate   
  // the process and all threads before the threads have completed.   
 
  pthread_join( thread1, NULL);
  pthread_join( thread2, NULL);
  */

  GLint t0 = SDL_GetTicks();
  for(i =0; i <2; i++)
    {
      frame_ready = 0;
      if ( (dc1394_capture_dequeue(my_camera_ptr[i], gpolicy, &gframe[i]) == DC1394_SUCCESS)) 
	frame_ready = 1;
      else
	printf("Error when in dc1394_capture_dequeue");         
    }
  GLint t1 = SDL_GetTicks();
  gcount_t += (long)(t1-t0);
  gcount_n++;
  //printf("Ticks: %d avg %d\n", (t1-t0), (gcount_t/gcount_n) );   

  for(i =0; i <2; i++)
    {
      //err = dc1394_capture_dequeue(g_cameras->camera_info, gpolicy, &gframe);
      //      frame_ready = 0;
      //if ( (dc1394_capture_dequeue(g_cameras->camera_info, gpolicy, &gframe) == DC1394_SUCCESS)) {

      if(frame_ready == 1)
	{
	  //dc1394video_frame_t* lframe;
	  //      dc1394_convert_to_RGB8(gframe->image, lframe->image, gframe->size[0],
	  //			     gframe->size[1], DC1394_BYTE_ORDER_UYVY, gframe->color_coding, 8);
	  glBindTexture( GL_TEXTURE_2D, texture[i] );
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gframe[i]->size[0],gframe[i]->size[1],0, GL_RGB, GL_UNSIGNED_BYTE, gframe[i]->image);
	    
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
		  //printf("%f %f\n",p0,p1);
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
		  //printf("%f %f\n",p0,p1);
		  // Draw a textured quad
		  glBegin(GL_QUADS);	
		  glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
		  glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
		  glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)height );
		  glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)height );
		  glEnd();

		}
	      /*p0 = (GLfloat)width/2+(GLfloat)gstretch;
	      p1 = (GLfloat)width-(GLfloat)gstretch;
	      // Draw a textured quad	
	      glBegin(GL_QUADS);
	      glTexCoord2f(0.0f, 0.0f); glVertex2f(p0, 0.0f);
	      glTexCoord2f(1.0f, 0.0f); glVertex2f(p1 , 0.0f);
	      glTexCoord2f(1.0f, 1.0f); glVertex2f(p1 , (GLfloat)height );
	      glTexCoord2f(0.0f, 1.0f); glVertex2f(p0, (GLfloat)height );
	      glEnd();*/
	    }
	  //printf("%f %f\t",(GLfloat)width,(GLfloat)height);
	 
	  dc1394_capture_enqueue(my_camera_ptr[i],gframe[i]); // release frame
	}
     
    }
  
  //printf("\n");
  if(frame_ready == 1)
    {
      //SDL_GL_SwapBuffers();
      Frames++;
    }
  
  GLint t = SDL_GetTicks();
  if (t - T0 >= 5000) {
    GLfloat seconds = (t - T0) / 1000.0;
    GLfloat fps = Frames / seconds;
    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
    T0 = t;
    Frames = 0;
    
  }
  
}



int main(int argc, char *argv[])
{
  SDL_Surface *screen;
  int done;
  Uint8 *keys;

  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*
  
  //SDL_Surface* screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL | SDL_FULLSCREEN ); // *changed*

  //screen = SDL_SetVideoMode(IMAGE_WIDTH, IMAGE_HEIGHT, 16, SDL_OPENGL|SDL_FULLSCREEN| SDL_HWACCEL ); 
  screen = SDL_SetVideoMode(IMAGE_WIDTH, IMAGE_HEIGHT, 16, SDL_OPENGL|SDL_RESIZABLE| SDL_HWACCEL ); 
  if ( ! screen ) {
    fprintf(stderr, "Couldn't set 300x300 GL video mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  SDL_WM_SetCaption("Gears", "gears");

  //init(argc, argv);
  mySDLinit(screen->w, screen->h);
  reshape(screen->w, screen->h);
  done = 0;
  //TTF_Init();
  //dispText();
  //sleep(5);

  /*

  // Load bitmap
  if ( (surface = SDL_LoadBMP("image.bmp")) ) { 
    
    // Check that the image's width is a power of 2
    if ( (surface->w & (surface->w - 1)) != 0 ) {
      printf("warning: image.bmp's width is not a power of 2\n");
    }
 
    // Also check if the height is a power of 2
    if ( (surface->h & (surface->h - 1)) != 0 ) {
      printf("warning: image.bmp's height is not a power of 2\n");
    }
    
    // get the number of channels in the SDL surface
    nOfColors = surface->format->BytesPerPixel;
    if (nOfColors == 4)     // contains an alpha channel
      {
	if (surface->format->Rmask == 0x000000ff)
	  texture_format = GL_RGBA;
	else
	  texture_format = GL_BGRA;
      } else if (nOfColors == 3)     // no alpha channel
      {
	if (surface->format->Rmask == 0x000000ff)
	  texture_format = GL_RGB;
	else
	  texture_format = GL_BGR;
      } else {
      printf("warning: the image is not truecolor..  this will probably break\n");
      // this error should not go unhandled
    }
    
    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &texture );
    
    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, texture );
    
    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
  } 
  else {
    printf("SDL could not load image.bmp: %s\n", SDL_GetError());
    SDL_Quit();
	return 1;
  }    
  
  // Free the SDL_Surface only if it was successfully created
  if ( surface ) { 
    SDL_FreeSurface( surface );
  }
*/

  ieee1394init();
  inittexture();
  drawTexture(screen->w, screen->h);
  while ( ! done ) {
    SDL_Event event;

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
    }
    
    if ( keys[SDLK_UP] ) {
      //view_rotx += 5.0;
    }
    if ( keys[SDLK_DOWN] ) {
      //view_rotx -= 5.0;
    }
    if ( keys[SDLK_LEFT] ) {
      //view_roty += 5.0;
      //screen->w++;
      //gstretch++;
      g_fstretch = g_fstretch + 0.01;
      /*      reshape(screen->w, screen->h);
      screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 16,
				SDL_OPENGL|SDL_RESIZABLE);
      if ( screen ) {
	reshape(screen->w, screen->h);
      } else {
	// Uh oh, we couldn't set the new video mode??
      }*/
      printf("Screen width: %d\n", screen->w);
    }
    if ( keys[SDLK_RIGHT] ) {
      //view_roty -= 5.0;
      //screen->w--;
      //gstretch--;
      g_fstretch = g_fstretch - 0.01;
      /*screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 16,
				SDL_OPENGL|SDL_RESIZABLE);
      if ( screen ) {
	reshape(screen->w, screen->h);
      } else {
	// Uh oh, we couldn't set the new video mode?? 
      }*/
      //reshape(screen->w, screen->h);
      printf("Screen width: %d\n", screen->w);
    }
    /*    if ( keys[SDLK_z] ) {
      if ( SDL_GetModState() & KMOD_SHIFT ) {
        view_rotz -= 5.0;
      } else {
        view_rotz += 5.0;
      }
    }
    */
    //draw();
    drawTexture(screen->w, screen->h);
    SDL_GL_SwapBuffers();

  }

  glDeleteTextures( 1, &texture[0] );
  glDeleteTextures( 1, &texture[1] );
  free(gimage[0]);
  free(gimage[1]);
  ieee1394stop();

  SDL_Quit();
  return 0;             /* ANSI C requires main to return int. */
}
