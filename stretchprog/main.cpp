#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core.hpp>
#include <math.h>
//#include <OpenCV/OpenCV.h>
#include <cassert>



int main (int argc, char ** const argv[]) {
  // insert code here...
  int maxstretch = 400;
  float tl = -0.00;
  float tr = 0;
  float factor = 0;
  float l = 0.00;
  float r = 0.00;
  // Init the text:
  CvScalar red = CV_RGB(250,0,0); 
  double hscale = 1.0;
  double vscale = 0.8;
  double shear = 0.2;
  int thickness = 2;
  int line_type = 8;
  int percent;
  int pp_size = 100;// percentage size of participant compared to mannequin x2
  int trackpos = pp_size*2; 
  float tp;
  int cond = 1;
	
  // Create a buffer to put the text:
  char text[100];

  // Init the font:
  CvFont font1;
  cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);
	
  // Location of text:
  CvPoint pt = cvPoint(10,30); // (0,0) is uppel left corner, figures are pixels
	
	
  cvNamedWindow( "live", 0 );
  cvNamedWindow("liveout", CV_WINDOW_AUTOSIZE);
	
	
  // Set up variables
  CvPoint2D32f frameTri[3], warpTri[3];
  //CvMat* rot_mat = cvCreateMat(2,3,CV_32FC1);
  CvMat* warp_mat = cvCreateMat(2,3,CV_32FC1);
  //IplImage *src, *dst;
	
  CvCapture* c_capture = cvCreateCameraCapture(0);
  if(!c_capture){
    printf("No camera found");
    return -1;
  }
  assert( c_capture );
	
  cvSetCaptureProperty( c_capture, CV_CAP_PROP_FRAME_WIDTH, 320 );
  cvSetCaptureProperty( c_capture, CV_CAP_PROP_FRAME_HEIGHT, 240 );
	
	
	
  IplImage* frame;
	
	
	
  while(1) {
    cvCreateTrackbar("tl", "live", &trackpos, maxstretch);
    cvCreateTrackbar("condition", "live", &cond, 3 );
    frame = cvQueryFrame( c_capture );
    IplImage* warp = cvCreateImage (cvGetSize(frame), IPL_DEPTH_8U, 3);
	  
    switch (cond) {
    case 2:
      tp = (120/(100/(float)pp_size));
      trackpos = (long)tp *2;
      cvSetTrackbarPos("tl", "live", trackpos );				
      break;
    case 1:
      trackpos = pp_size *2;
      cvSetTrackbarPos("tl", "live", trackpos );			
      break;
    case 0:
      tp = (80/(100/(float)pp_size));
      trackpos = (long)tp *2;
      cvSetTrackbarPos("tl", "live", trackpos );
      break;
    default:
      /*factor = (float)trackpos/400;
	tl = 0 - factor;
	tr = 1 + factor;
	l = tl + 0.5;
	r = tr - 0.5;*/
      break;
    }
    factor = (float)trackpos/400;
    tp = - factor;
    //tr = 
    l = - factor + 0.5;
    r = factor + 1.5;
    //percent = ((pp_size*2)/maxstretch)*trackpos;						
							
    //cvSetTrackbarPos("tl", "live", trackpos );
    percent = (100/(float)pp_size)* ((float)trackpos/2);

    //cvGetTrackbarPos("tl", "live");
		
    frameTri[0].x = 0;
    frameTri[0].y = 0;
    frameTri[1].x = frame->width - 1;
    frameTri[1].y = 0;
    frameTri[2].x = 0;
    frameTri[2].y = frame->height - 1;
		
    warpTri[0].x = frame->width*l; //topleft
    warpTri[0].y = frame->height*0.00;
    warpTri[1].x = frame->width*r; //topright
    warpTri[1].y = frame->height*0.00;
    warpTri[2].x = frame->width*l;//bottom;left
    warpTri[2].y = frame->height*1.00;
		
    cvGetAffineTransform( frameTri, warpTri, warp_mat );
    cvWarpAffine( frame, warp, warp_mat );
    //cvCopy ( warp, frame );
		
		
    // Print to the buffer, eg if you want to display an integer:
    sprintf( text, "%d%s", percent, "%" );
		
		
    // Before you display the image, put the text in the image
    cvPutText(frame,  text, pt, &font1, red);
		
		
    if(!frame ) break;
    cvShowImage ( "live", frame );
    cvShowImage("liveout", warp);
    char c = cvWaitKey(33);
    if( c == 27 )break;
		
  }
	
  cvReleaseCapture(&c_capture );
  cvDestroyWindow( "live" );
  cvDestroyWindow("liveout");
	
  std::cout << "Hello, World!\n";
  return 0;

}
