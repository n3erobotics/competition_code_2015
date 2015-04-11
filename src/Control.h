#ifndef CONTROL_H_
#define CONTROL_H_

#include "UEyeOpenCV.hpp"

//Lane change
#define CONTROL_WITH_DS3
#define DS3_SPEED 280

//Camera Controllers
#define BINAY_THRESHOLD 60

//Track image controls
#define DISTANCE_OF_END_TURN 100
#define MID_LINE_DISTANCE 200
#define TURN_COEFICIENT 2.25
#define N_TO_KEEP 3
#define HORIZONTAL_LINE_THRESHOLD 25
#define VERTICAL_LINE_THRESHOLD 30

//Spike
#define TETA_END_TURN_LEFT 33
#define TETA_END_TURN_RIGHT 42
#define HORIZONTAL 15
#define X_MAX_TO_DETECT 100
#define X_TO_STOP_TURN 220
#define Y_TO_STOP_TURN 60

//Other
#define SPEED 280
#define Y_MAX_TO_DETECT_LANE 145
#define Y_MIN_TO_DETECT_LANE 0
#define Y_RATIO 1
#define AREA_TO_DETECT 35

//Zebra
#define HORIZONTAL_ANGLE 7.5
#define ZEBRA_Y_MAX_DISTANCE 95
#define ZEBRA_Y_MIN_DISTANCE 35
#define ZEBRA_X_MAX_DISTANCE 90
#define ZEBRA_VERTICAL_TO_HORIZONTAL_DISTANCE 7
#define MIN_ZEBRA_HEIGHT 125
#define MAX_ZEBRA_HEIGHT 250

//Devices
#define ARDUINO_CONTROL "//dev//ttyACM1"
#define DEVICE_TOP 7

///capture width and height
const int height = AOIHEIGHT;
const int width = AOIWIDTH;

#endif /* CONTROL_H_ */
