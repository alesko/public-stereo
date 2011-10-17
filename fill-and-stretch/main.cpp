#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core.hpp>
#include <math.h>
//#include <OpenCV/OpenCV.h>
#include <cassert>

IplImage* GetThresholdedImage(IplImage* img)
{
	// Convert the image into an HSV image
	IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
	cvCvtColor(img, imgHSV, CV_BGR2HSV);
	IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);
	cvInRangeS(imgHSV, cvScalar(30, 100, 100), cvScalar(50, 255, 255), imgThreshed);//play with these numbers and different colours in the right environment
	cvReleaseImage(&imgHSV);
	return imgThreshed;
}


int main (int argc, char ** const argv[]) {
    // insert code here...
	int pp_size = 80;// percentage size of participant compared to mannequin x2
	int maxstretch = 400;
//	float tl = -0.00;
//	float tr = 0;
//	float factor = 0;
//	float factor1 = 0;
//	float div = 0;
	float l = 0.00;
	float r = 0.00;
	// Init the text:
	CvScalar red = CV_RGB(250,0,0); 
	double hscale = 1.0;
	double vscale = 0.8;
	double shear = 0.2;
	int thickness = 2;
	int line_type = 8;
	float percent;
	//int pp_size = 100;// percentage size of participant compared to mannequin x2
	int trackpos = maxstretch; 
		
	
	
	
	// Create a buffer to put the text:
	char text[100];
	
	
	
	// Init the font:
	CvFont font1;
	cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);
	
	// Location of text:
	CvPoint pt = cvPoint(10,30); // (0,0) is uppel left corner, figures are pixels
	
	
	cvNamedWindow( "live", 0 );
	cvNamedWindow("liveout", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("filter", CV_WINDOW_AUTOSIZE);
	
	
	
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
	//frame1 = cvQueryFrame( c_capture );
	//IplImage* frame = GetThresholdedImage(frame1);
	
	cvCreateTrackbar("tl", "live", &trackpos, maxstretch);
	//cvSetTrackbarPos("tl", "live", 400 );
	
	while(1) {
		
		
		frame = cvQueryFrame( c_capture );
		IplImage* warp = cvCreateImage (cvGetSize(frame), IPL_DEPTH_8U, 3);
		
		
		
		float inc = (((float)pp_size*2)/maxstretch)/2;
		//float inc1 = inc/2;
		l = 0.5 - ((inc/100) *trackpos);
		r = 0.5 + ((inc/100)* trackpos);
		percent = trackpos/2;
		int percent1 = (long) percent;
		
	
		
		
		
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
		sprintf( text, "%d%s", percent1, "%");
		
		
		// Before you display the image, put the text in the image
		cvPutText(frame,  text, pt, &font1, red);
		
		IplImage* frame1 = GetThresholdedImage(warp);
		
		
		if(!frame ) break;
		cvShowImage ( "live", frame );
		cvShowImage("liveout", warp);
		cvShowImage("filter", frame1);
		char c = cvWaitKey(33);
		if( c == 27 )break;
		
	}
	
	
	
	cvReleaseCapture(&c_capture );
	cvDestroyWindow( "live" );
	cvDestroyWindow("liveout");
	cvDestroyWindow("filter");
	
    std::cout << "Hello, World!\n";
    return 0;
}
