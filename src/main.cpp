#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sstream>

#include "TrackHandler.h"
#include "Control.h"
#include "SignalInterpretation.h"
#include "Utils.h"
#include "SerialPort.h"
#include "UEyeOpenCV.hpp"
#include "Dualshock3.h"

#define NUM_THREADS 2

//blob defines
#define h 480
#define w 640

using namespace std;
using namespace cv;

extern SerialPort serialPort;
extern VideoCapture webcamT;
extern UeyeOpencvCam UEye;
extern pthread_mutex_t access_change;
extern bool wanna_change;

CvCapture* capture=cvCreateCameraCapture(0);
IplImage *fram=cvCreateImage(cvSize(w,h),8,3);   //Original Image
IplImage *hsvframe=cvCreateImage(cvSize(w,h),8,3);//Image in HSV color space
IplImage *threshy=cvCreateImage(cvSize(w,h),8,1); //Threshold image of yellow color
IplImage* img=cvCreateImage(cvSize(w,h),8,3);

void *DS3Controller(void *){
	stringstream speed_message;

	Dualshock3 joystick;
	joystick.connect();
	while(1){
		joystick.getData();
		if(joystick.axis[13] != 0){
			speed_message.str("");
			speed_message << "f" << DS3_SPEED << endl;
			//speed_message << "f" << joystick.axis[13]/135 << endl;
			serialPort.sendArray(speed_message.str());
		}else{
			serialPort.sendArray("n\n");
		}
	}
	pthread_exit(NULL);
}

int main()
{
	int rc;
	pthread_t threads[NUM_THREADS];
	string buffer;

	signal (SIGINT, SIG_handler);
	signal (SIGSEGV, SIG_handler);

	int err=serialPort.connect(ARDUINO_CONTROL);

	//	 TODO Uncomment bellow
	if(err==-1){
		cout << "Serial communication not established!" << endl;
		exit(-1);
	}

	//cout << "Creating Signal Interpretation thread" << endl;
	//rc = pthread_create(&threads[1], NULL, signalInterpretation, (void *)NULL);
	//sleep(1);
	//if (rc){
		//cout << "ERROR; return code from pthread_create() is "<< rc << endl;
		//exit(-1);
	//}
#ifdef CONTROL_WITH_DS3
	cout << "Creating Controller thread" << endl;
	rc = pthread_create(&threads[0], NULL, DS3Controller, (void *)NULL);
	if (rc){
		cout << "ERROR; return code from pthread_create() is "<< rc << endl;
		exit(-1);
	}
#endif

	cout << "Creating Track Handler thread" << endl;
	rc = pthread_create(&threads[0], NULL, trackHandler, (void *)NULL);
	if (rc){
		cout << "ERROR; return code from pthread_create() is "<< rc << endl;
		exit(-1);
	}

	//	cout << " Waiting ..." << endl;


	//	while(true){
	//		if(blob_tracing()){
	//			pthread_mutex_lock(&access_change);
	//
	//			wanna_change=true;
	//
	//			pthread_mutex_unlock(&access_change);
	//
	//			sleep(7);
	//			pthread_mutex_lock(&access_change);
	//
	//			wanna_change=true;
	//
	//			pthread_mutex_unlock(&access_change);
	//		}
	//	}


	//	sleep(3);
	//	pthread_mutex_lock(&access_change);
	//
	//	wanna_change=true;
	//
	//	pthread_mutex_unlock(&access_change);
	//
	//	sleep(7);
	//	pthread_mutex_lock(&access_change);
	//
	//	wanna_change=true;
	//
	//	pthread_mutex_unlock(&access_change);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	UEye.close();
	webcamT.release();
	
	serialPort.sendArray("f0\n");

	cout << "Terminating" << endl;
	exit(0);
}
