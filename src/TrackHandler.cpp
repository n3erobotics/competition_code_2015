/*
 * TrackHandler.cpp
 *
 *  Created on: May 9, 2014
 *      Author: ruben
 */
#include "TrackHandler.h"
#include "Control.h"
#include "Utils.h"
#include "SerialPort.h"
#include "SignalInterpretation.h"
#include "UEyeOpenCV.hpp"
#include <string>

using namespace cv;
using namespace std;

// Events
bool completed_lap = false;

//Glabal Variables
int stoped_detect_zebra, detected_n_zebra, laps;
bool detected_zebra=false;
int distance_from_last_lane=0, straight_counter;
vector<double> line2follow, last_line2follow;
vector<vector<double> > objects;
bool present_lane = RIGHT;
stringstream message;
Mat drawing, image;

//Global variables externally declared
extern SerialPort serialPort;
extern UeyeOpencvCam UEye;
extern int display_detected, signal_detected;
extern pthread_mutex_t access_webcam, access_displays, access_change;
extern bool wanna_change;

bool verify_inside_zebra(int upper_limit, int lower_limit){
	vector<double> object_inside;
	bool one_inside=false;
	int N=objects.size()-1;

	for(size_t i=0; i<objects.size(); i++){
		// checks if one objects was found inside, if true checks if the object found now is paralel ( has aprox. same Y)
		if( one_inside && ( abs(objects.at(i).at(Y)-object_inside.at(Y))<30 ) ){
			// checks if they are too close to limits (false zebra)
			return true;
		}
		// otherwise it'll see if
		if( (objects.at(N-i).at(Y) < upper_limit-30) && (objects.at(N-i).at(Y) > lower_limit+30) ){
			one_inside=true;
			object_inside=objects.at(N-i);
		}
	}
	return false;
}

// Routine that checks if there are 2 horizontal lines
//TODO check if objects are inside those lines
//to make sure it's a crossroad
void check_crossroad(){
	vector<double> zebra_lower, zebra_higher;
	size_t no_objects=objects.size();
	int o_t, o_y;

	// ONly makes sense if we detected 8 objects already
	if(no_objects>8){
		int count_horizontal_line=0;
		bool found_zebra=false;
		for( size_t i = 0; i< no_objects; i++ ){
			//checks if it is inside the track
			if(!object_on_right(objects.at(i))){
				// checks horizontal line
				o_t=objects.at(i).at(TETA);
				if(( o_t<HORIZONTAL_LINE_THRESHOLD ) && ( o_t> -(HORIZONTAL_LINE_THRESHOLD) )){
					count_horizontal_line++;
					// saves values
					if( count_horizontal_line==1 ){
						zebra_lower=objects.at(i);
						zebra_higher=objects.at(i);
					}else{
						o_y=objects.at(i).at(Y);
						// if we have more than 1 line, let's compare
						// compare if we have an horizontal line with bigger Y than another horizontal line
						if( o_y>zebra_higher.at(Y) ){
							// if the distance between those two zebra lines is too big or too small don't keep values
							if( (o_y-zebra_lower.at(Y)<MAX_ZEBRA_HEIGHT) && (o_y-zebra_lower.at(Y)>MIN_ZEBRA_HEIGHT) ){
								if( verify_inside_zebra( o_y,zebra_lower.at(Y) ) ){
									zebra_higher=objects.at(i);
									found_zebra=true;
								}
							}
						}
						// compare if we have an horizontal line with lesser Y than another horizontal line
						if( o_y<zebra_lower.at(Y) ){
							// if the distance between those two zebra lines is too big or too small don't keep values
							if( (zebra_higher.at(Y)-o_y<MAX_ZEBRA_HEIGHT) && (zebra_higher.at(Y)-o_y>MIN_ZEBRA_HEIGHT) ){
								if( verify_inside_zebra( zebra_higher.at(Y), o_y ) ){
									zebra_lower=objects.at(i);
									found_zebra=true;
								}
							}
						}
					}
				}
			}
		}
		if( found_zebra ){
			detected_n_zebra++;
			if(detected_n_zebra>=2 && !detected_zebra){
				drawLane(zebra_lower,drawing, YELLOW );
				drawLane(zebra_higher,drawing, YELLOW );
				detected_zebra=true;
				stoped_detect_zebra=0;
				laps++;
			}
			for( size_t i = 0; (i < no_objects); i++ ){
				// if the object match one horizontal zebra line it's removed
				if( objects.at(i).at(Y)==zebra_lower.at(Y) || objects.at(i).at(Y)==zebra_higher.at(Y) ){
					objects.erase (objects.begin()+i);
					no_objects--;
					i--;
				}
			}
			stoped_detect_zebra=0;
		}else{
			if(stoped_detect_zebra>=3 && detected_zebra){
				detected_zebra=false;
				//just_passed_zebra=true;
			}
			detected_n_zebra=0;
		}
	}else{
		stoped_detect_zebra++;
		if(stoped_detect_zebra>=3 && detected_zebra){
			detected_zebra=false;
			detected_n_zebra=0;
		}
	}
}

void check_display_update(){
	pthread_mutex_lock(&access_displays);

	if(display_detected!=5){
		serialPort.sendArray("r15\n",5);
		usleep(1000000);
		completed_lap=true;
	}

	pthread_mutex_unlock(&access_displays);
}
void controller(bool direction){

	finding_objects(image);
	move_in_lane(direction);

	if(present_lane == OUTSIDE){
		putText(drawing, "OUTSIDE", Point2f(050,050), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
	}else{
		putText(drawing, "INSIDE", Point2f(050,050), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
	}
	imshow("Drawing", drawing);
	waitKey(1);

}

//TODO
void find_first_object(bool position){
	bool first_line=false;
	int min_dist, dist;

	if(position){
		while(!first_line){
			message.str("");
			message << "l50" << endl;
			serialPort.sendArray(message.str(), message.tellp());
			image=UEye.getFrame();
			flip(image,image, 0);
			finding_objects(image);
			waitKey(1);
			//First recognition of line2follow
			for(size_t i=0; i<height/3; i++){
				cout << i << endl;
				Vec3b color = drawing.at<Vec3b>(Point(580,i));
				//if he detects white
				if(color.val[0]==255){
					int N=objects.size()-1;
					// the object in the right up corner gets caught
					for( size_t j = 0; j<objects.size(); j++ ){
						cout << "for: " << N-j << endl;
						if(objects.at(N-j).at(Y)<(height/3)){
							last_line2follow=objects.at(N-j);
							line2follow=objects.at(N-j);
							first_line=true;
							break;
						}
					}
				}
			}
		}
	}else{
		image=UEye.getFrame();
		cout << "Finding Objects for the first time!" << endl;
		finding_objects(image);
		//First recognition of line2follow
		last_line2follow=objects.at(0);
		line2follow=objects.at(0);
		min_dist=abs( calculateDistanceToMidline(objects.at(0).at(X), objects.at(0).at(Y), RIGHT) );
		for( size_t i = 1; i< objects.size(); i++ ){
			if(objects.at(i).at(AREA)<400){
				dist=abs( calculateDistanceToMidline(objects.at(i).at(X), objects.at(i).at(Y), RIGHT) );
				if(dist<min_dist){
					min_dist=dist;
					last_line2follow=objects.at(i);
					line2follow=objects.at(i);
				}
			}
		}
	}
}

//TODO
int wait_signal(){
	int aux;


	while(true){
		pthread_mutex_lock(&access_displays);

		aux = display_detected;
		display_detected=5;

		pthread_mutex_unlock(&access_displays);

		if(aux==GREEN_FRONT){
			return GREEN_FRONT;

		}else if(aux==YELLOW_LEFT){
			return YELLOW_LEFT;

		}else if(aux==YELLOW_RIGHT){
			return YELLOW_RIGHT;
		}
	}
}

//TODO
bool object_on_right(vector<double> line_object){

	if( (int)line_object.at(X) <= (int)(line2follow.at(X)+(cos(line2follow.at(TETA)*PI/180)*(line_object.at(Y)-line2follow.at(Y))))){
		return false;
	}else{
		return true;
	}
}

bool object_on_left(vector<double> line_object){

	if( (int)line_object.at(X) >= (int)(line2follow.at(X)+(cos(line2follow.at(TETA)*PI/180)*(line_object.at(Y)-line2follow.at(Y))))){
		return false;
	}else{
		return true;
	}
}

bool only_objects_on_right(){
	for(size_t i=0; i<objects.size(); i++){
		//Condition to have objects on left and bellow
		if( !object_on_right(objects.at(i)) ){
			return false;
		}
	}
	return true;
}

bool only_objects_on_left(){
	for(size_t i=0; i<objects.size(); i++){
		//Condition to have objects on left and bellow
		if( object_on_right(objects.at(i)) ){
			return false;
		}
	}
	return true;
}

//TODO
// Routine to change to "change_lane"
void change_lane(bool direction){
	int N=objects.size()-1;

	// if in the right way (straight ahead)
	if(!direction){
		if(present_lane == OUTSIDE){
			// If we are on right and we discover an object on left we follow it
			for(size_t i=0; i<objects.size(); i++){
				if( object_on_left(objects.at(N-i)) && objects.at(N-i).at(X)>width/3){
					wanna_change=false;
					last_line2follow=objects.at(N-i);
					line2follow=objects.at(N-i);
					present_lane=INSIDE;
					return ;
				}
			}
			putText(drawing, "CHANGING", Point2f(350,250), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
			message.str("");
			message << "l" << CHANGE_2LEFT_LANE_ANGLE << endl;
			serialPort.sendArray(message.str(), message.tellp());
		}else{
			// If we are on left and we discover an object on right we follow it
			for(size_t i=0; i<objects.size(); i++){
				if( object_on_right(objects.at(i)) ){
					wanna_change=false;
					last_line2follow=objects.at(i);
					line2follow=objects.at(i);
					present_lane=OUTSIDE;
					return ;
				}
			}
			putText(drawing, "CHANGING", Point2f(350,250), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
			message.str("");
			message << "r" << CHANGE_2RIGHT_LANE_ANGLE << endl;
			serialPort.sendArray(message.str(), message.tellp());
		}
	}else{
		if(present_lane == OUTSIDE){
			// If we are on right and we discover an object on left we follow it
			for(size_t i=0; i<objects.size(); i++){
				if( object_on_left(objects.at(N-i)) && objects.at(N-i).at(X)>width/3){
					wanna_change=false;
					last_line2follow=objects.at(N-i);
					line2follow=objects.at(N-i);
					present_lane=INSIDE;
					return ;
				}
			}
			putText(drawing, "CHANGING", Point2f(350,250), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
			message.str("");
			message << "r" << CHANGE_2LEFT_LANE_ANGLE << endl;
			serialPort.sendArray(message.str(), message.tellp());
		}else{
			// If we are on left and we discover an object on right we follow it
			for(size_t i=0; i<objects.size(); i++){
				if( object_on_right(objects.at(i)) ){
					wanna_change=false;
					last_line2follow=objects.at(i);
					line2follow=objects.at(i);
					present_lane=OUTSIDE;
					return ;
				}
			}
			putText(drawing, "CHANGING", Point2f(350,250), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
			message.str("");
			message << "l" << CHANGE_2LEFT_LANE_ANGLE << endl;
			serialPort.sendArray(message.str(), message.tellp());
		}
	}
}

//TODO
// Routine that calculates servo rotation from distance of the line
void simple_distance_lines(vector<double> lane, bool direction){
	int distance, teta;
	char dir;

	distance = calculateDistanceToMidline(lane.at(X), lane.at(Y), direction);
	if(distance > 0){
		dir='l';
		teta=(int) ( (float)(distance)/TURN_COEFICIENT);
	}else{
		dir='r';
		teta= (int) ( (float)(-distance)/TURN_COEFICIENT);
	}

	message.str("");
	message << dir << teta << endl;
	putText(drawing, message.str(), Point2f(200,050), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
	serialPort.sendArray(message.str(), message.tellp());
	
	message.str("");
	message << "d" << distance;
	putText(drawing, message.str(), Point2f(350,050), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);


	if(teta<=5&&teta>=-5)
		straight_counter++;

}

//TODO
// Routine that calculates wich is the lane to follow based on last followed lane
// Returns the distance change
int get_line2follow(){
	int l_x,l_y, o_x, o_y, least_distance, calculated_distance;

	//Let's assume that
	line2follow=objects.front();

	drawLane(last_line2follow, drawing, GREEN);

	l_x=last_line2follow.at(X);
	l_y=last_line2follow.at(Y);

	// 1st attempt let's assume least distance equals the first (front) object
	least_distance = sqrt( pow(l_x-line2follow.at(X),2) + pow(l_y-line2follow.at(Y),2) );

	for( size_t i = 1; i< objects.size(); i++ ){
		o_x = objects.at(i).at(X);
		o_y = objects.at(i).at(Y);
		if( (o_y>Y_MIN_TO_DETECT_LANE) && (o_y<Y_MAX_TO_DETECT_LANE) ){

			calculated_distance = sqrt( pow(l_x-o_x, 2) + pow(l_y-o_y,2) );
			if( calculated_distance < least_distance ){
				line2follow=objects.at(i);
				least_distance=calculated_distance;
			}
		}
	}
	drawLane(line2follow, drawing, RED);

	last_line2follow=line2follow;
	message.str("");
	message << (int)line2follow.at(AREA) << " (" << (int)line2follow.at(X) << ", " << (int)line2follow.at(Y) << ")" << endl;
	putText(drawing, message.str(), Point2f(50,75), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0), 2);
	

	if(line2follow.at(AREA)>175000){
		//wanna_change=true;
	}
	return least_distance;
}

// Routine that decides wich routines to call based on events
void move_in_lane(bool direction){
	vector<Point> discontinuousPoints;

	if(objects.size()>=1){

		distance_from_last_lane=get_line2follow();

		if(wanna_change){
			change_lane(direction);
		}else{
			simple_distance_lines(line2follow, direction);
		}

	}else{
		cout << "No objects found" << endl;
	}
}

//TODO
// Finds objects that matter
// Returns: Matriz of double (OBJECTS) - each line is an object
// For each column:
// [0] - X of the object
// [1] - Y of the object
// [2] - angle of the projection of the object
// [3] - area of the object > AREA_TO_DETECT_LANE
void finding_objects(Mat frame){
	vector<float> linReg;
	vector<vector<Point> > contours;
	std::stringstream s;
	size_t i;
	double area;

	cvtColor(frame, frame, CV_BGR2GRAY);
	threshold( frame, frame, BINAY_THRESHOLD,255,THRESH_BINARY);
	//imshow("Binary", frame);
	//waitKey(1);

	//finds the contours of all white objects that appear in the image
	// outputs a matrix ( we called "contours") - each line contains the edges of an object
	findContours( frame, contours, CV_RETR_EXTERNAL, CV_LINK_RUNS, Point(0, 0) );

	// Clear used variables
	drawing = Mat::zeros( frame.size(), CV_8UC3 );
	Mat test = Mat::zeros( frame.size(), CV_8UC3 );
	objects.clear();
	
	for( i = 0; i< contours.size(); i++ )
	{
		//Filters garbage
		area = contourArea(contours.at(i), false );
		if(area > AREA_TO_DETECT){
			fitLine(contours.at(i), linReg, CV_DIST_L2, 0, 0.1, 0.1);
			//fill the objects matrix
			objects.push_back( vector<double>(4, 0.0) );
			objects.back().at(X)=(double)linReg.at(2);
			objects.back().at(Y)=(double)linReg.at(3);
			objects.back().at(TETA)=(atan(linReg[1]/linReg[0]) * 180.0/PI);
			objects.back().at(AREA)=area;

			drawFitLine(linReg, drawing, BLUE);
			drawContours( drawing, contours, i, WHITE );
			
		}
	}
	// Sorts objects from parameter AREA
	stable_sort( objects.begin(), objects.end(), compare_t( objects, X ) );

		//for(  i = 0; i< objects.size(); i++ ){
			//cout << "#" << i << " ( " << objects.at(i).at(X) << " , " << objects.at(i).at(Y) << " ) teta:" <<
					//objects.at(i).at(TETA) << "; area: " << objects.at(i).at(AREA) << endl;
		//}
}


//TODO
void *trackHandler(void*){
	Timer timer;
	std::stringstream s;
	int signal;
	
	message.str("");
	message << "n" << endl;
	serialPort.sendArray(message.str(), message.tellp());
	sleep(1);
	// As the thing starts it will
	while(true){
		//signal=wait_signal();
		/*TODO*/
		signal=GREEN_FRONT;
		completed_lap=false;
		if(signal==GREEN_FRONT){
			
			find_first_object(RIGHT);

			present_lane=OUTSIDE;
			message.str("");
			message << "f" << SPEED << endl;
			serialPort.sendArray(message.str(), message.tellp());	
			while(!completed_lap){
				//timer.reset();
				image=UEye.getFrame();
				imshow("IMAGE",image);
				//cout << endl << "-------------------------" << endl << endl << "Capture frame time: " << timer.elapsed() << endl;
				
				//timer.reset();
				controller(RIGHT);
				//cout << endl << "-------------------------" << endl << endl << "Controller procedure time: " << timer.elapsed() << endl;
				//				cout << "Time elapsed: " << timer.elapsed() << endl;
				if(wait_ESC()){
					serialPort.sendArray("n",6);
					destroyAllWindows();
					pthread_exit(NULL);
				}
				//TODO stop at ZEBRAAAAAAAAA
				//check_display_update();
			}
		}else if(signal==YELLOW_LEFT){

			straight_counter=0;
			find_first_object(LEFT);
			message.str("");
			message << "f" << SPEED << endl;
			serialPort.sendArray(message.str(), message.tellp());

			present_lane=OUTSIDE;
			while(!completed_lap){

//				timer.reset();
				image=UEye.getFrame();
				flip(image,image, 0);
//				cout << endl << "-------------------------" << endl << endl << "Capture frame time: " << timer.elapsed() << endl;

				controller(LEFT);

//				cout << "Time elapsed: " << timer.elapsed() << endl;
//				if(wait_ESC()){
//					destroyAllWindows();
//					pthread_exit(NULL);
//				}
				//TODO stop at ZEBRAAAAAAAAA
				//check_display_update();
				if(present_lane==OUTSIDE && straight_counter>=10){
					wanna_change=true;
				}
			}
		}else{
			cout << "PARKING MANEUVRE" <<endl;
			//TODO
		}
		//TODO go to zebra
		//cout<< "ZEBRAAAAAAAAAAAAAAAAAAAAAA"<<endl;
	}

	destroyAllWindows();
	pthread_exit(NULL);
}

