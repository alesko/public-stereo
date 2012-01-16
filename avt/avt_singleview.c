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
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>
#include <X11/keysym.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#include "dc1394/dc1394.h"
#include "dc1394/vendor/avt.h"

/* uncomment the following to drop frames to prevent delays */
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


/* declarations for libdc1394 */
dc1394camera_t *camera;
dc1394featureset_t features;
dc1394video_frame_t * frame_received;
dc1394video_frame_t frame_raw8;
dc1394video_frame_t frame_debayered;
dc1394video_frame_t frame_yuv422;

/* declarations for video1394 */
unsigned long device_width,device_height, 
              device_left,device_top; 


/* declarations for Xwindows */
Display *display=NULL;
Window window=(Window)NULL;
unsigned long width,height;
int connection=-1;
XvImage *xv_image=NULL;
XvAdaptorInfo *info;
GC gc;

/* Other declarations */
unsigned long frame_length;
int adaptor=-1;

int freeze=0;
int color_coding;
uint32_t Camera_ID;

uint32_t hsize, vsize, hstep, vstep;


/* helper functions */
void display_frame()
{
    if(!freeze && adaptor>=0){
        if (!frame_received)
            return;

        dc1394_avt_adjust_frames(Camera_ID, frame_received);

        switch (frame_received->color_coding) {
            /* color codes that can be converted to YUV422 by dc1394_convert_frames */
            case DC1394_COLOR_CODING_YUV411:
            case DC1394_COLOR_CODING_YUV422:
            case DC1394_COLOR_CODING_YUV444:
            case DC1394_COLOR_CODING_RGB8:
            case DC1394_COLOR_CODING_MONO8:
            case DC1394_COLOR_CODING_MONO16:
            case DC1394_COLOR_CODING_AVT_MONO12:
                dc1394_convert_frames(frame_received, &frame_yuv422);
                break;
 
            /* color codes that need to be debayered to rgb8/16 first */
            case DC1394_COLOR_CODING_RAW8:
                frame_debayered.color_coding = DC1394_COLOR_CODING_RGB8;
                dc1394_debayer_frames(frame_received, &frame_debayered, DC1394_BAYER_METHOD_SIMPLE);
                dc1394_convert_frames(&frame_debayered, &frame_yuv422);
                break;

            case DC1394_COLOR_CODING_RAW16:
                frame_debayered.color_coding = DC1394_COLOR_CODING_RGB16;
                dc1394_debayer_frames(frame_received, &frame_debayered, DC1394_BAYER_METHOD_SIMPLE);
                dc1394_convert_frames(&frame_debayered, &frame_yuv422);
                break;

            /* raw12 needs to be converted to raw8/raw16 first 
               (call dc1394_convert_packed12_to_16 to retrieve raw16 instead of raw8) */
            case DC1394_COLOR_CODING_AVT_RAW12:
                dc1394_convert_frames(frame_received, &frame_raw8);
                frame_debayered.color_coding = DC1394_COLOR_CODING_RGB8;
                dc1394_debayer_frames(&frame_raw8, &frame_debayered, DC1394_BAYER_METHOD_SIMPLE);
                dc1394_convert_frames(&frame_debayered, &frame_yuv422);
                break;

            default:
                break;
        }

        xv_image=XvCreateImage(display,info[adaptor].base_id,XV_YUY2,(char*)frame_yuv422.image,
                            device_width,device_height);
        XvPutImage(display,info[adaptor].base_id,window,gc,xv_image,
                0,0,device_width,device_height,
                0,0,width,height);
        xv_image=NULL;
    }
}


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
            if(formats[j].id==XV_YUY2) {
                dc1394_log_error("using Xv format 0x%x %s %s",formats[j].id,xv_name,(formats[j].format==XvPacked)?"packed":"planar");
                if(adaptor<0) adaptor=i;
            }
        }
    }
    XFree(formats);
    if(adaptor<0)
        dc1394_log_error("No suitable Xv adaptor found");
}


void cleanup(void) {
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_capture_stop(camera);

    if ((void *)window != NULL)
        XUnmapWindow(display,window);
    if (display != NULL)
        XFlush(display);

    if(frame_raw8.image) {
        free(frame_raw8.image);
        frame_raw8.image = NULL;
    }
    if(frame_debayered.image) {
        free(frame_debayered.image);
        frame_debayered.image = NULL;
    }
    if(frame_yuv422.image) {
        free(frame_yuv422.image);
        frame_yuv422.image = NULL;
    }
}


/* trap ctrl-c */
void signal_handler( int sig) {
    signal( SIGINT, SIG_IGN);
    cleanup();
    exit(0);
}


int main(int argc,char *argv[])
{
    XEvent xev;
    XGCValues xgcv;
    long background=0x010203;
    dc1394_t * d;
    dc1394camera_list_t * list;
    dc1394error_t err;
    uint32_t dummy1, dummy2, dummy3;
    dc1394color_codings_t supported_color_codings;
    int i;

    /* check parameters */
    switch( argc ) {
        case 1:
            printf("No color coding specified, mono8 is used as default\n");
            color_coding=DC1394_COLOR_CODING_MONO8;
            break;
        case 2:
            if(0==strcmp( argv[1], "-h" ) ||
               0==strcmp( argv[1], "--help" ) ||
               0==strcmp( argv[1], "-?" )) {
                printf(" usage: avt_singleview color_code\n    supported color codes:\n      mono8\n      yuv411\n      yuv422\n      yuv444\n"
                       "      rgb8\n      mono16\n      raw8\n      raw16\n      avt_mono12\n      avt_raw12\n");
                exit(0); 
            }
            else if(0==strcmp( argv[1], "mono8" ))
                color_coding=DC1394_COLOR_CODING_MONO8;
            else if(0==strcmp( argv[1], "yuv411" ))
                color_coding=DC1394_COLOR_CODING_YUV411;
            else if(0==strcmp( argv[1], "yuv422" ))
                color_coding=DC1394_COLOR_CODING_YUV422;
            else if(0==strcmp( argv[1], "yuv444" ))
                color_coding=DC1394_COLOR_CODING_YUV444;
            else if(0==strcmp( argv[1], "rgb8" ))
                color_coding=DC1394_COLOR_CODING_RGB8;
            else if(0==strcmp( argv[1], "mono16" ))
                color_coding=DC1394_COLOR_CODING_MONO16;
            else if(0==strcmp( argv[1], "raw8" ))
                color_coding=DC1394_COLOR_CODING_RAW8;
            else if(0==strcmp( argv[1], "raw16" ))
                color_coding=DC1394_COLOR_CODING_RAW16;
            else if(0==strcmp( argv[1], "avt_mono12" ))
                color_coding=DC1394_COLOR_CODING_AVT_MONO12;
            else if(0==strcmp( argv[1], "avt_raw12" ))
                color_coding=DC1394_COLOR_CODING_AVT_RAW12;
            else {
                printf(" unknown color code, \n \"avt_singleview -?\" for help\n");
                exit(0); 
            }
            break;
        default:
            printf(" too many parameters, \n \"avt_singleview -?\" for help\n");
            exit(0);
    }

    /* init frame structs */
    frame_received = NULL;
    memset( &frame_debayered, 0, sizeof(dc1394video_frame_t) );
    memset( &frame_yuv422, 0, sizeof(dc1394video_frame_t) );
    memset( &frame_raw8, 0, sizeof(dc1394video_frame_t) );
    frame_yuv422.color_coding = DC1394_COLOR_CODING_YUV422;
    frame_yuv422.yuv_byte_order = DC1394_BYTE_ORDER_YUYV;
    frame_raw8.color_coding = DC1394_COLOR_CODING_RAW8;

    /* init libdc, open first camera */
    d = dc1394_new ();
    if (!d)
        return 1;
    err=dc1394_camera_enumerate (d, &list);
    DC1394_ERR_RTN(err,"Failed to enumerate cameras");

    if (list->num == 0) {
        dc1394_log_error("No cameras found");
        return 1;
    }

    camera = dc1394_camera_new (d, list->ids[0].guid);
    if (!camera) 
        dc1394_log_warning("Failed to initialize camera with guid %llx", list->ids[0].guid);
    dc1394_camera_free_list (list);

    srand(time(NULL));

    /* get camera information (we need to know Camera_ID for dc1394_avt_adjust_frames) */
    dc1394_avt_get_version(camera, &dummy1, &dummy2, &Camera_ID, &dummy3);

    /* init camera */
    err=dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set ISO speed");

    err=dc1394_format7_get_max_image_size(camera, DC1394_VIDEO_MODE_FORMAT7_0, &hsize, &vsize);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not get max image size");

    err=dc1394_format7_get_unit_size(camera, DC1394_VIDEO_MODE_FORMAT7_0, &hstep, &vstep);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not get unit size");

    err=dc1394_format7_get_color_codings(camera, DC1394_VIDEO_MODE_FORMAT7_0, &supported_color_codings);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not get supported color codings");

    for( i=0; i<supported_color_codings.num; i++ ) {
        if( supported_color_codings.codings[i] == color_coding )
            break;
        if( i==supported_color_codings.num-1 ) {
            dc1394_log_error("Color Coding not supported by camera");
	    exit(-1);
        }
    }

    /* setup camera for capture */    
    device_width  = hsize;
    device_height = vsize;
    device_left   = 0;
    device_top    = 0;

    printf("Trying to setup mode %ldx%ld+%ld+%ld\n", 
        device_width, device_height, device_left, device_top );

    err=dc1394_format7_set_roi(camera, DC1394_VIDEO_MODE_FORMAT7_0, color_coding,
                               DC1394_USE_MAX_AVAIL, 
                               device_left, device_top, device_width, device_height);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");

    err=dc1394_video_set_mode(camera, DC1394_VIDEO_MODE_FORMAT7_0);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not set camera mode");

    err= dc1394_capture_setup(camera,NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Final capture setup failed!");

    err=dc1394_video_set_transmission(camera, DC1394_ON);
    DC1394_ERR_CLN_RTN(err,cleanup(),"Could not start camera iso transmission");


    /* make the window */
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

    if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame_received)!=DC1394_SUCCESS)
    dc1394_log_error("Failed to capture from camera");


    display_frame();

    width=device_width;
    height=device_height;

    window=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,width,height,0,
                               WhitePixel(display,DefaultScreen(display)),
                               background);

    XSelectInput(display,window,StructureNotifyMask|KeyPressMask);
    XMapWindow(display,window);
    connection=ConnectionNumber(display);

    gc=XCreateGC(display,window,0,&xgcv);

    /* main event loop */
    while(1){
        if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame_received)!=DC1394_SUCCESS)
                dc1394_log_error("Failed to capture from camera");

        display_frame();
        XFlush(display);

        while(XPending(display)>0){
            XNextEvent(display,&xev);
            switch(xev.type){
            case ConfigureNotify:
                width=xev.xconfigure.width;
                height=xev.xconfigure.height;
                display_frame();
                break;
            case KeyPress:
                switch(XKeycodeToKeysym(display,xev.xkey.keycode,0)){
                case XK_q:
                case XK_Q:
                    cleanup();
                    exit(0);
                    break;
                case XK_0:
                    freeze = !freeze;
                    break;
                case XK_w:
                case XK_W:
                    dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE,  DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
                    DC1394_ERR_CLN_RTN(err,cleanup(),"Count not one-push WB");
                    break;
                default:
                    break;
                }
                break;
            }
        } /* XPending */

        if (frame_received && frame_received->image)
            dc1394_capture_enqueue (camera, frame_received);

    } /* while not interrupted */

    exit(0);
}
