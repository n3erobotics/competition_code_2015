/*
 * Control.h
 *
 *  Created on: May 9, 2014
 *      Author: ruben
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include "UEyeOpenCV.hpp"

//Lane change

//Camera Controllers
#define BINAY_THRESHOLD 40
#define BRIGTHNESS 0.4
#define SATURATION 0.5
#define CONTRAST 0.6
#define HUE 0.2

//Image Turning Controllers
#define MID_LINE_DISTANCE 110
#define TURN_COEFICIENT 2.75
#define N_TO_KEEP 7
#define TETA_END_TURN_LEFT 8
#define TETA_END_TURN_RIGHT 13
#define CHANGE_2RIGHT_LANE_ANGLE 25
#define CHANGE_2LEFT_LANE_ANGLE 25
#define HORIZONTAL 8

//Other
#define SPEED 70
#define Y_MAX_TO_DETECT_LANE 150
#define Y_MIN_TO_DETECT_LANE 0
#define Y_RATIO 2
#define DISTANCE_OF_END_TURN 100
#define AREA_TO_DETECT 50

//Zebra
#define HORIZONTAL_LINE_THRESHOLD 20
#define VERTICAL_LINE_THRESHOLD 40
#define MIN_ZEBRA_HEIGHT 125
#define MAX_ZEBRA_HEIGHT 250

//Devices
#define ARDUINO_CONTROL "//dev//ttyACM0"
#define DEVICE_TOP 7

///capture width and height
const int height = AOIHEIGHT;
const int width = AOIWIDTH;

#endif /* CONTROL_H_ */
