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

void check_crossroad();
int wait_signal();
void simple_distance_lines(vector<double> lanes, bool lane);
void move_in_lane(bool lane);
void finding_objects(Mat cut_bin);
void *trackHandler(void *v_serialPort);
void virar_esquerda();
void virar_direita();

#endif /* TRACKHANDLER_H_ */
