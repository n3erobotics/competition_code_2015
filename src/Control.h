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
#define BINAY_THRESHOLD 25
#define BRIGTHNESS 0.4
#define SATURATION 0.5
#define CONTRAST 0.6
#define HUE 0.2

//Image Turning Controllers
#define MID_LINE_DISTANCE 110
#define TURN_COEFICIENT 4
#define CHANGE_2RIGHT_LANE_ANGLE 25
#define CHANGE_2LEFT_LANE_ANGLE 25
#define MINIMUM_ANGLE_CURVE 20
#define MINIMUM_DETECTED_TIMES_CURVE 10
#define STABLE_CURVE_ANGLE 2



//Other
#define SPEED 70
#define Y_MAX_TO_DETECT_LANE 400
#define Y_MIN_TO_DETECT_LANE 0
#define AREA_TO_DETECT 100
#define DISCONTINUOUS_LINE_BREAK 450

//Zebra
#define HORIZONTAL_LINE_THRESHOLD 15
#define MIN_ZEBRA_HEIGHT 125
#define MAX_ZEBRA_HEIGHT 250

//Devices
#define ARDUINO_CONTROL "//dev//ttyACM0"
#define DEVICE_TOP 7

///capture width and height
const int height = 480;
const int width = 752;

#endif /* CONTROL_H_ */
