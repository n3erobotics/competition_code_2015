#include <iostream>
#include <iomanip>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <cv.h>
#include <highgui.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <cvblob.h>

#include "SemaphoreTrigger.h"

int semaphore_status = STARTING;

RNG rng(12345);

void my_handler(int s){

           cout << "GOT CTRL + C -> Exiting" << endl;
           exit(1); 

}

class Timer
{
public:
    Timer() { clock_gettime(CLOCK_REALTIME, &beg_); }

    double elapsed() {
        clock_gettime(CLOCK_REALTIME, &end_);
        return end_.tv_sec - beg_.tv_sec +
            (end_.tv_nsec - beg_.tv_nsec) / 1000000000.;
    }

    void reset() { clock_gettime(CLOCK_REALTIME, &beg_); }

private:
    timespec beg_, end_;
};


void waitForIt(){
//int main(){
  CvTracks tracks;
	Mat cropped, mat_converted, seg_mat;
	
	
	
	Timer timer;

	double minx, miny, maxx, maxy, area;
	
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL); // captures ctrl + c
	
	cvNamedWindow("capture", CV_WINDOW_AUTOSIZE);

	CvCapture *capture = cvCaptureFromCAM(1);
  
  	// let's set the camera capture size
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 480 );
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 320 );
  
  	cvGrabFrame(capture);
  	IplImage *img = cvRetrieveFrame(capture);

	CvSize imgSize = cvGetSize(img); //cvSize(480,320);

  	IplImage *frame = cvCreateImage(imgSize, img->depth, img->nChannels);
  	IplImage *hsvframe=cvCreateImage(imgSize, img->depth, img->nChannels); //Image in HSV color space

  	IplConvKernel* morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);

  	//unsigned int frameNumber = 0;
  	unsigned int blobNumber = 0;

 	bool quit = false;
  	while (!quit&&cvGrabFrame(capture)){
		cout << semaphore_status << " | ";
		
		timer.reset();
		
	    IplImage *img = cvRetrieveFrame(capture);

    	cvConvertScale(img, frame, 1, 0);

    	IplImage *segmentated = cvCreateImage(imgSize, 8, 1);


    	cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);
	
    	//cvShowImage("segmentated", segmentated);

    	IplImage *labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);
    
    	//Changing the color space
		cvCvtColor(frame,hsvframe,CV_BGR2HSV);
		
		//Thresholding the frame for different colors | HUE - SATURATION - VALUE/BRIGHTNESS
		if (semaphore_status == STARTING){
			cvInRangeS(hsvframe,cvScalar(0,80,120),cvScalar(180,255,255),segmentated); // red
		}else if(semaphore_status == READY && MODE == RACE){
			cout << "RACE MODE";
			cvInRangeS(hsvframe,cvScalar(45,49,120),cvScalar(81,255,255),segmentated); // green
		}else if(semaphore_status == READY && MODE == PARKING){
			cout << "PARKING MODE";
			cvInRangeS(hsvframe,cvScalar(20,30,140),cvScalar(55,220,255),segmentated); // yellow
		}
		

		
		//Filtering the frame
		cvSmooth(segmentated,segmentated,CV_MEDIAN,7,7);
    
	 	cvShowImage("capture", frame);
    
    //cvShowImage("segmentated", segmentated);
    /*std::stringstream filename;
    filename << "redobject_" << std::setw(5) << std::setfill('0') << frameNumber << ".png";
    cvSaveImage(filename.str().c_str(), frame);*/

    
    
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		mat_converted = Mat(frame,false);
		seg_mat = Mat(segmentated,false);
	
	
		Canny( seg_mat, seg_mat, 5, 15, 3 );
		imshow("Canny", seg_mat);
		moveWindow("Canny", 500, 0);
		findContours( seg_mat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
		float area=0., max_area=0.;
		int position;
	
		for( int i = 0; i< contours.size(); i++ ){
			area = contourArea(contours[i], false);
		
			if(area >= max_area){
				max_area=area;
				position = i;
			}
		
		}
	
		cout << "Max area: " << max_area; 
		if (semaphore_status == STARTING && max_area > 2000){
			semaphore_status = READY;
		}else if (semaphore_status == READY && max_area > 2000){
			semaphore_status = GO;
			cout << endl << endl << "GO GO GO !!!" << endl << endl;
			break;
		}
	
		//cout << "BIG DADDY BLOB:" << position << " | Area: " << max_area << endl;
	
	
		
		/// Get the moments
 		 vector<Moments> mu(contours.size());
 		 for( int i = 0; i < contours.size(); i++ )
    		 { mu[i] = moments( contours[i], false ); }
     
     	//mu[0] = moments( contours[position], false );

 		 ///  Get the mass centers:
		  vector<Point2f> mc( contours.size() );
		  for( int i = 0; i < contours.size(); i++ )
			 { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }
			 
			// mc[0] = Point2f( mu[position].m10/mu[position].m00 , mu[position].m01/mu[position].m00 );
			
		// Draw contours
		Mat drawing = Mat::zeros( seg_mat.size(), CV_8UC3 );
		for( int i = 0; i< contours.size(); i++ ){
			   Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
			   drawContours( drawing, contours, position,  Scalar( 0, 0, 255 ), 2, 8, hierarchy, 0, Point());
			   circle( drawing, mc[i], 4, color, -1, 8, 0 );
			   //cout << "CM_X:" << mc[0].x << "--CM_Y:" << mc[0].y << endl;
		}
     
     	//Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       	//drawContours( drawing, contours, position, color, 2, 8, hierarchy, 0, Point() );
       	//circle( drawing, mc[0], 4, color, -1, 8, 0 );
     
		imshow("drawing", drawing);
		moveWindow("drawing", 950, 0);
	
		cvReleaseImage(&labelImg);
		cvReleaseImage(&segmentated);
		//CvBlobs::const_iterator indicator=0;
		//float max_area=0.;
		int i=0;

/*
	if (minx >= 0 && maxx >= 0 && miny >= 0 && maxy >= 0 && maxx >= minx && maxy>=miny){
	
		//imshow("mat",mat_converted);
		cv::Rect myROI(minx, miny, maxx-minx, maxy-miny);
		//sleep(1);
	// Crop the full image to that image contained by the rectangle myROI
	// Note that this doesn't copy the data
		cv::Mat image(mat_converted);
		cv::Mat croppedImage = image(myROI);
	
		//imshow("croppedImage",croppedImage);
	}
	
	*/
    char k = cvWaitKey(10)&0xff;

	cout << endl;

	cout << "Time: " << timer.elapsed() << endl;

    //frameNumber++;
  }

  cvReleaseStructuringElement(&morphKernel);
  cvReleaseImage(&frame);

  cvDestroyWindow("red_object_tracking");

}
