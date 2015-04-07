/*
 * Utils.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: ruben
 */
#include "Utils.h"
#include "SerialPort.h"
#include "UEyeOpenCV.hpp"

using namespace cv;
using namespace std;

SerialPort serialPort;
VideoCapture webcamT;
UeyeOpencvCam UEye = UeyeOpencvCam();
int display_detected, signal_detected;
pthread_mutex_t access_webcam = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t access_displays = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t access_HUB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t access_change = PTHREAD_MUTEX_INITIALIZER;
bool wanna_change;

Mat get_frame(VideoCapture webcam){
	Mat frame;

	while(true){
		pthread_mutex_lock(&access_webcam);

		webcam.read(frame);

		pthread_mutex_unlock(&access_webcam);

		if(!frame.data ){ // Check for invalid input
			cout << "Could not open image" << std::endl ;
			continue;
		}else{
			return frame;
		}
	}
}

bool wait_ESC(){

	if(waitKey(3)==ESC){
		cout << "ESC key pressed by user" << endl;
		serialPort.sendArray("n\n");
		serialPort.sendArray("r0\n");
		UEye.close();
		return true;
	}
	return false;
}
void waiting(int key){
	int keyPressed;
	
	cout << "Waiting key..." << endl;
	while(true){
		keyPressed=waitKey(100);
		if( keyPressed == key)
			break;
	}
	cout << "Key pressed!" << endl;
}

void SIG_handler(int signo){

	//Martelada d'ouro
	serialPort.sendArray("n\n");
	serialPort.sendArray("r0\n");
	UEye.close();

	cout << "Exiting with signal " << signo << endl;
	
	UEye.close();
	webcamT.release();

	pthread_exit(NULL);
}


void drawLane(vector<double> Lane, Mat &frame, const Scalar& color){

	circle(frame,cvPoint(Lane.at(X), Lane.at(Y)),5, color,-1);
	line(frame, cvPoint(Lane.at(X), Lane.at(Y)), cvPoint(Lane.at(X)+20*cos(Lane.at(TETA)*PI/180), Lane.at(Y)+20*sin(Lane.at(TETA)*PI/180) ), color,2);
}

void drawFitLine(vector<float> linReg, Mat &frame, const Scalar& color){

	circle(frame,cvPoint(linReg.at(2), linReg.at(3)),5, color,-1);
	line(frame, cvPoint(linReg.at(2), linReg.at(3)), cvPoint(linReg.at(2)+20*(linReg.at(0)), linReg.at(3)+20*(linReg.at(1))), color,2);
}

int calculateDistanceToMidline(int xx, int yy, bool dir){
	double num, den, d;
	
	if(dir){
		//LEFT
		num = (A*xx) + (B*yy) + OFFSET_LEFT;
	}else{
		//RIGHT
		num = (-A*xx) + (B*yy) + OFFSET_RIGHT;		
	}
	den = sqrt( (A*A) + (B*B) );
	d=num/den;
	return (int)d;
}
