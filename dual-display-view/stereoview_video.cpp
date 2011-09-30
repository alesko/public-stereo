/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Alexander Skoglund, Karolinska Institute
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Karolinska Institute nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#ifdef DARWIN
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#endif 
#ifdef LINUX
#include <cv.h>
#include <highgui.h>
#endif 



using namespace std;
using namespace cv;

double GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
}


int main (int argc, char * const argv[]) {
	
	cvNamedWindow( "Left", CV_WINDOW_AUTOSIZE );
	cvMoveWindow("Left",00,00);
	cvNamedWindow( "Right", CV_WINDOW_AUTOSIZE );
	cvMoveWindow("Right",1280,00);
	CvCapture* capture_left = cvCreateFileCapture( argv[1] );
	CvCapture* capture_right = cvCreateFileCapture( argv[2] );
	double fps = cvGetCaptureProperty(capture_left,CV_CAP_PROP_FPS);
	int wait_time= (int)1000.0/fps;
	double tstart, t_elaps;
	
	IplImage* frame_l;
	IplImage* frame_r;
	IplImage* dest_l;
	IplImage* dest_r;
	
	
	CvScalar red = CV_RGB(250,0,0);
	CvScalar blue = CV_RGB(0,0,250);
	CvFont font1;
	double hscale = 1.0;
	double vscale = 0.8;
	double shear = 0.2;
	int thickness = 2;
	int line_type = 8;
	cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);
	//char text[100];
	int i=0;
	CvPoint pt = cvPoint(10,30);
	double scale=3.6;
	
	while(1) {		
	  //const char* text = "Left video";
	  //sprintf( text, "Frame number: %d", i );
	  tstart = GetRunTime();
	  
	  frame_l = cvQueryFrame( capture_left );
	  frame_r = cvQueryFrame( capture_right );
	  if (i == 0)
	    {
	      // Create a new 3 channel image
	      dest_l = cvCreateImage( cvSize(scale*(frame_l->width),scale*(frame_l->height)), 8, 3 );
	      dest_r = cvCreateImage( cvSize(scale*(frame_r->width),scale*(frame_r->height)), 8, 3 );
	    }
	  cvResize(frame_l, dest_l);
	  cvResize(frame_r, dest_r);
	  //cvPutText(frame_l, text, pt, &font1, red);
	  //cvPutText(frame_r, text, pt, &font1, blue);
	  if( !frame_l ) break;
	  cvShowImage( "Left", dest_l );
	  if( !frame_r ) break;
	  cvShowImage( "Right", dest_r );
	  t_elaps = 0.001*(GetRunTime()-tstart); // Convert to msec
	  cout << "Fps:" << fps << "\tWaittime: " << wait_time << "\telaps:" << t_elaps<< endl;
	  char c = cvWaitKey(wait_time-t_elaps); // Determines the framerate
	  if( c == 27 ) break;
	  if( c == 32 )
	    {
	      c = 0;
	      while(c != 32 && c != 27){
		c = cvWaitKey(250);
	      }
	    }
	  if( c == 43 )
	    {
	      scale = scale + 0.1;
	    }
	  if( c == 45 )
	    {
	      scale = scale - 0.1;
	    }
	  
	  i++;
	}
	cvReleaseCapture( &capture_left );
	cvReleaseCapture( &capture_right );
	cvDestroyWindow( "Left" );
	cvDestroyWindow( "Right" );

}
