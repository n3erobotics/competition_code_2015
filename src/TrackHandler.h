/*
 * TrackHandler.h
 *
 *  Created on: May 9, 2014
 *      Author: ruben
 */

#ifndef TRACKHANDLER_H_
#define TRACKHANDLER_H_


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>
#include <cv.h>
#include <sstream>
#include <cmath>
#include <signal.h>

using namespace cv;
using namespace std;

int wait_signal();
bool object_on_right(vector<double> line_object);
bool object_on_left(vector<double> line_object);
bool only_objects_on_right();
bool only_objects_on_left();
void simple_distance_lines(vector<double> lanes, bool lane);
void move_in_lane(bool lane);
void finding_objects(Mat cut_bin);
void *trackHandler(void *v_serialPort);

#endif /* TRACKHANDLER_H_ */
