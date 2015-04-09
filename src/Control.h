#ifndef CONTROL_H_
#define CONTROL_H_

#include "UEyeOpenCV.hpp"

//Lane change

//Camera Controllers
#define BINAY_THRESHOLD 130

//Track image controls
#define DISTANCE_OF_END_TURN 100
#define MID_LINE_DISTANCE 110
#define TURN_COEFICIENT 2.75
#define N_TO_KEEP 7
#define TETA_END_TURN_LEFT 14
#define TETA_END_TURN_RIGHT 35
#define HORIZONTAL 2.5
#define HORIZONTAL_LINE_THRESHOLD 25
#define VERTICAL_LINE_THRESHOLD 30

//Other
#define SPEED 10
#define Y_MAX_TO_DETECT_LANE 155
#define Y_MIN_TO_DETECT_LANE 0
#define Y_RATIO 1
#define AREA_TO_DETECT 25

//Zebra
#define HORIZONTAL_ANGLE 7.5
#define ZEBRA_Y_MAX_DISTANCE 95
#define ZEBRA_Y_MIN_DISTANCE 35
#define ZEBRA_X_MAX_DISTANCE 90
#define ZEBRA_VERTICAL_TO_HORIZONTAL_DISTANCE 7
#define MIN_ZEBRA_HEIGHT 125
#define MAX_ZEBRA_HEIGHT 250

//Devices
#define ARDUINO_CONTROL "//dev//ttyACM0"
#define DEVICE_TOP 7

///capture width and height
const int height = AOIHEIGHT;
const int width = AOIWIDTH;

#endif /* CONTROL_H_ */
