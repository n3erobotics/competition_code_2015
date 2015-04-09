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

#define DEBUG
#define SHOW_IMAGE
#define SHOW_DRAWING
#define END_TURN

using namespace cv;
using namespace std;

// Events
bool completed_lap = false;

//Glabal Variables
int stoped_detect_zebra, laps;
bool detected_zebra=false;

//Tracking variables
int distance_from_last_lane=0;
vector<double> line2follow, last_line2follow;
vector< vector<double> > objects;
bool present_lane = RIGHT; // boolean to represent which lane the car is
bool end_of_turn=false; // boolean to sinalize if the turn ended (because of the big jump)
vector<int> lastNturns; // stores the information of the inclination of the last 5 movements 
int distanceMiddle; // actual distance to middle lane
int teta; // actual angle of turn
char dir; // actual direction of turn
char end_turn_dir; // 

//Global usage
stringstream message;
stringstream turn_message;
stringstream speed_message;
Mat drawing, image;

//Global variables externally declared
extern SerialPort serialPort;
extern UeyeOpencvCam UEye;
extern int display_detected, signal_detected;
extern pthread_mutex_t access_webcam, access_displays, access_change;
extern bool wanna_change;

// Routine that checks if there are 2 horizontal lines
//TODO check if objects are inside those lines
//to make sure it's a crossroad
void check_crossroad(){
	size_t no_objects=objects.size();
	vector<double> zebra_low_part, zebra_high_part;
	vector<size_t> indexes;
	vector< vector<double> > vertical_lines;
	size_t index_best_distance=0;
	detected_zebra = false;
	// ONly makes sense if we detected 9 objects already (2 vertical lines and 7 horizontal) + 2 linhas
	if( (no_objects>=11) && (!end_of_turn)){
		for( size_t i = 0; i< objects.size(); i++ ){
			//if object is horizontal check if there's another
			if( abs( objects.at(i).at(TETA) ) < HORIZONTAL_ANGLE ){
				//let's check if there's another horizontal line
				for( size_t j = i+1; j< objects.size(); j++ ){
					vertical_lines.clear();
					indexes.clear();
					if( (abs( objects.at(j).at(TETA) ) < HORIZONTAL_ANGLE) ){
						//check if both objects are close enough to be in the zebra, but not too close
						if( (abs(objects.at(i).at(Y) - objects.at(j).at(Y)) < ZEBRA_Y_MAX_DISTANCE) && (abs(objects.at(i).at(X) - objects.at(j).at(X)) < ZEBRA_X_MAX_DISTANCE) && (abs(objects.at(i).at(Y) - objects.at(j).at(Y)) > ZEBRA_Y_MIN_DISTANCE) ){
							// who's the lower limit
							if(objects.at(i).at(Y) < objects.at(j).at(Y)){
								zebra_low_part = objects.at(i);
								zebra_high_part = objects.at(j);
							}else{
								zebra_low_part = objects.at(j);
								zebra_high_part = objects.at(i);
							} 
							vertical_lines.clear();
							// check how many lines inside "zebra" hipothesis
							int o_y;
							for(size_t k=0; k<no_objects; k++){
								if( (k!=j) && (k!=i) ){
									o_y = objects.at(k).at(Y);
									if( (o_y > zebra_low_part.at(Y)) && (o_y < zebra_high_part.at(Y)) ){
										if( (abs(o_y-zebra_low_part.at(Y)) > HORIZONTAL) || (abs(o_y-zebra_high_part.at(Y)) > HORIZONTAL) ){
											vertical_lines.push_back( objects.at(k) );
											indexes.push_back(k);
										}
									}
								}
							}
							//if 7 vertical lines are detecteted
							if( vertical_lines.size() >= 7 ){
								detected_zebra = true;
								//calculate the supposed x mid point
								// delete the most distant points from midpoint
								int mid_point = ((zebra_high_part.at(X)+zebra_low_part.at(X))/2);
								while( vertical_lines.size() > 7 ){
									// assume first is the worst distance
									int index_worst_distance = 0, d;
									int worst_distance = abs(vertical_lines.at(0).at(X)-mid_point);
									// check if any of those points has worst distance
									for(size_t index = 1; index < vertical_lines.size(); index++){
										d = abs(vertical_lines.at(index).at(X)-mid_point);
										if( d > worst_distance){
											index_worst_distance = index;
											worst_distance = d;
										}
									}
									// delete that shit nigga
									vertical_lines.erase(vertical_lines.begin() + index_worst_distance);
									indexes.erase(indexes.begin() + index_worst_distance);
								}
							}else{
								//go to next iteration if not 7 objects found
								continue;
							}
							// since objects are sorted by X, 4 is the middle one
							// compare the distance with mid points
							int mid_point = ((zebra_high_part.at(X)+zebra_low_part.at(X))/2);
							if( abs(mid_point-vertical_lines.at(3).at(X)) < (ZEBRA_X_MAX_DISTANCE) ){
								int best_distance = abs(vertical_lines.at(0).at(X)-((zebra_high_part.at(X)+zebra_low_part.at(X))/2));
								int d,n=0;
								// find the middle zebra strip
								for(size_t index = 1; index < vertical_lines.size(); index++){
									d = abs(vertical_lines.at(index).at(X)-mid_point);
									if( d < best_distance){
										index_best_distance = index;
										best_distance = d;
									}
									if( abs(vertical_lines.at(index).at(TETA)) < HORIZONTAL_ANGLE ){
										n++;
										if(n>2){
											detected_zebra = false;
											cout << "Pocaralho não menganas" <<endl;
											break;

										}
									}
								}
							}else{
								detected_zebra = false;
							}
							// delete objects of crossroad
							if(detected_zebra){
								// delete horizontal lines
								objects.erase(objects.begin() + i);
								objects.erase(objects.begin() + j);
								//delete all vertical lines
								for(int k = indexes.size()-1; k >= 0; k--){
									size_t l=k;
									if(l!=index_best_distance){
										objects.erase(objects.begin() + indexes.at(l));
									}
								}
								break;
							}
						}
					}
				}
			}
			if(detected_zebra){
				cout << "**************************************PASSADEIRA**************************************" <<endl;
				//cout << "zebra: (" <<zebra_low_part.at(X) << ", " << zebra_low_part.at(Y) << ")  (" <<zebra_high_part.at(X) << ", " << zebra_high_part.at(Y) <<endl; 
				drawLane(zebra_high_part, drawing, MAGENTA);
				drawLane(zebra_low_part, drawing, MAGENTA);
				for(size_t index = 0; index < vertical_lines.size(); index++){
					if(index!=index_best_distance){
						drawLane(vertical_lines.at(index), drawing, CIAN);
						//cout << "Vertical at (" << vertical_lines.at(index).at(X) << ", " << vertical_lines.at(index).at(Y) << ")" << endl;
						//cout << "Vertical line angle of: " << vertical_lines.at(index).at(TETA) << endl;
					}
				}
				break;
			}
		}
	}
}
void check_display_update(){
	pthread_mutex_lock(&access_displays);

	if(display_detected!=5){
		serialPort.sendArray("r15\n");
		usleep(1000000);
		completed_lap=true;
	}

	pthread_mutex_unlock(&access_displays);
}
//TODO
void find_first_object(bool position){
	bool first_line=false;
	int min_dist, dist;

	if(position){
		while(!first_line){
			message.str("");
			message << "l50" << endl;
			serialPort.sendArray(message.str());
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
// Routine to change to "change_lane"
void change_lane(bool direction){

	present_lane = LEFT;
}
//TODO
// Routine that calculates servo rotation from distance of the line
void simple_distance_lines(vector<double> lanes, bool lane){
	// calculate distance to the midline depending on the lane you are in
	// according to that, calculate the angle of turn, and store it
	distanceMiddle = calculateDistanceToMidline(lanes.at(X), lanes.at(Y), lane);
	if(distanceMiddle > 0){
		dir='l';
		teta=(int) ( (float)(distanceMiddle)/TURN_COEFICIENT);
		lastNturns.push_back(LEFT);
	}else{
		dir='r';
		teta= (int) ( (float)(-distanceMiddle)/TURN_COEFICIENT);
		lastNturns.push_back(RIGHT);
	}
	//buffer only keeps 5, must delete the first
	lastNturns.erase(lastNturns.begin());
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
	// makes distance on xx 1/2 of yy to make better turns
	least_distance = sqrt( pow(l_x-line2follow.at(X),2) + Y_RATIO*pow(l_y-line2follow.at(Y),2) );

	for( size_t i = 1; i< objects.size(); i++ ){
		o_x = objects.at(i).at(X);
		o_y = objects.at(i).at(Y);
		if( (o_y>Y_MIN_TO_DETECT_LANE) && (o_y<Y_MAX_TO_DETECT_LANE)){

			calculated_distance = sqrt( pow(l_x-o_x, 2) + Y_RATIO*pow(l_y-o_y,2) );
			if( calculated_distance < least_distance){
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

	return least_distance;
}
void send_command_arduino(){


	serialPort.sendArray(turn_message.str());
	//serialPort.sendArray(speed_message.str());

}
char calculate_end_of_turn_side(){
	int n_rights=0;
	for(size_t i = 0; i<lastNturns.size(); i++){
		if(lastNturns.at(i)==RIGHT){
			n_rights++;
		}
	}
	if(n_rights > (lastNturns.size()/2) )
		return 'r';
	else
		return 'l';
}
void detect_end_of_turn(){

//if end of turn detected or in end of turn manouvre
#ifdef END_TURN
	int initial_y = line2follow.at(Y);
	if( (!detected_zebra) && ((distance_from_last_lane > DISTANCE_OF_END_TURN) || (end_of_turn)) ){
		//Just do it for the first time
		if( !end_of_turn){
			//if( abs(teta) < HORIZONTAL_LINE_THRESHOLD ){

			end_turn_dir=calculate_end_of_turn_side();
			end_of_turn = true;
			speed_message.str("");
			speed_message << "f" << 5 << endl;
			//serialPort.sendArray(speed_message.str());

			//TURN LEFT
			if(end_turn_dir=='l'){
				for( size_t i = 1; i< objects.size(); i++ ){
					if( abs(objects.at(i).at(Y)-initial_y) < HORIZONTAL ){
						if(objects.at(i).at(X)<line2follow.at(X)){
							line2follow = objects.at(i);
							last_line2follow = objects.at(i);
						}
					}
				}
				turn_message << end_turn_dir << TETA_END_TURN_LEFT  << "\n";
			}else{
				for( size_t i = 1; i< objects.size(); i++ ){
					if( abs(objects.at(i).at(Y)-initial_y) < HORIZONTAL ){
						if(objects.at(i).at(X)>line2follow.at(X)){
							line2follow = objects.at(i);
							last_line2follow = objects.at(i);
						}
					}
				}
				turn_message << end_turn_dir << TETA_END_TURN_RIGHT << "\n";
			}



			cout << endl << endl << "**************************************DETECTED SPIKE**************************************" << endl <<endl <<endl;
			cout << "Going: " << end_turn_dir << endl;
			//
		}else{
			if(end_turn_dir == 'l'){
				if( (distanceMiddle > 0) &&  (teta > (2 * HORIZONTAL)) ){
					cout << "**************************************Ended!**************************************" << endl;
					end_of_turn = false;
					speed_message.str("");;
					speed_message << "f" << SPEED << endl;
					serialPort.sendArray(speed_message.str());
				}
			}
			if(end_turn_dir == 'r'){
				if( (distanceMiddle < 0) && (teta > (2 * HORIZONTAL))){
					cout << "**************************************Ended!**************************************" << endl;
					end_of_turn = false;
					speed_message.str("");;
					speed_message << "f" << SPEED << endl;
					serialPort.sendArray(speed_message.str());
				}
			}

		}
	}else{
#endif
		//apply the simple distance algorithm
		turn_message.str("");
		turn_message << dir << teta << "\n";
#ifdef END_TURN
	}
#endif
}
// Routine that decides wich routines to call based on events
void move_in_lane(bool lane){
	if(objects.size()>=1){
		distance_from_last_lane=get_line2follow();
		simple_distance_lines(line2follow, lane);
		detect_end_of_turn();

		cout << "Distance last: " << distance_from_last_lane << " | Distance middle: " << distanceMiddle << " | Teta: " << teta << " | Dir: " << dir << endl;
	}else{
		cout << "No objects found" << endl;
	}
}
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
	float teta_rad;
	double area;
	Mat img;

;


	//cvtColor(frame, frame, CV_BGR2GRAY);
	GaussianBlur(frame, img, Size(0, 0), 3);
	addWeighted(frame, 1.5, img, -0.5, 0, img);
#ifdef SHOW_IMAGE
	imshow("frame",frame)
	imshow("img",img);
#endif
	threshold( img, frame, BINAY_THRESHOLD,255,THRESH_BINARY);
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
			teta_rad=atan2f(linReg[1],linReg[0]);
			objects.back().at(TETA)=(teta_rad/PI*180); //+ ( teta_rad > 0 ? 0 : 360);
			objects.back().at(AREA)=area;

			//drawFitLine(linReg, drawing, BLUE);
			drawContours( drawing, contours, i, WHITE );

		}
	}
	// Sorts objects from parameter AREA
	stable_sort( objects.begin(), objects.end(), compare_t( objects, X ) );

	//cout << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl;
	//for(  i = 0; i< objects.size(); i++ ){
	//cout << "#" << i << " ( " << objects.at(i).at(X) << " , " << objects.at(i).at(Y) << " ) teta:" <<
	//objects.at(i).at(TETA) << "; area: " << objects.at(i).at(AREA) << endl;
	//}

}
void controller(bool direction){

	finding_objects(image); //finds all relevant objects
	check_crossroad(); // finds crossroad
	move_in_lane(direction); // applies the movement prediction algorithm
	send_command_arduino(); // sends data to arduino

#ifdef SHOW_DRAWING
	imshow("Drawing", drawing);
#endif
	waitKey(1);

}
//TODO
void *trackHandler(void*){
	Timer timer;
	std::stringstream s;
	int signal;

	message.str("");
	message << "n" << endl;
	serialPort.sendArray(message.str());
	sleep(1);
	lastNturns.assign(N_TO_KEEP, 0); // adds N ints with value 0
	// As the thing starts it will
	while(true){
		//signal=wait_signal();
		/*TODO*/
		signal=GREEN_FRONT;
		completed_lap=false;
		if(signal==GREEN_FRONT){

			find_first_object(RIGHT);

			present_lane=OUTSIDE;
			speed_message.str("");
			speed_message << "f" << SPEED << endl;
			serialPort.sendArray(speed_message.str());
			while(!completed_lap){
				//timer.reset();
				image=UEye.getFrame();
				//imshow("IMAGE",image);
				//cout << endl << "-------------------------" << endl << endl << "Capture frame time: " << timer.elapsed() << endl;

				//timer.reset();
				controller(RIGHT);
				//cout << endl << "-------------------------" << endl << endl << "Controller procedure time: " << timer.elapsed() << endl;
				//				cout << "Time elapsed: " << timer.elapsed() << endl;
				//if(wait_ESC()){
				//serialPort.sendArray("n");
				//destroyAllWindows();
				//pthread_exit(NULL);
				//}
				//TODO stop at ZEBRAAAAAAAAA
				//check_display_update();
			}
		}else if(signal==YELLOW_LEFT){

			find_first_object(LEFT);
			message.str("");
			message << "f" << SPEED << endl;
			serialPort.sendArray(message.str());

			present_lane=OUTSIDE;
			while(!completed_lap){

				//				timer.reset();
				image=UEye.getFrame();
				flip(image,image, 0);
				//				cout << endl << "-------------------------" << endl << endl << "Capture frame time: " << timer.elapsed() << endl;

				controller(LEFT);

				//				cout << "Time elapsed: " << timer.elapsed() << endl;
				if(wait_ESC()){
					serialPort.sendArray("n");
					destroyAllWindows();
					pthread_exit(NULL);
				}
				//TODO stop at ZEBRAAAAAAAAA
				//check_display_update();
				if(present_lane==OUTSIDE){
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
