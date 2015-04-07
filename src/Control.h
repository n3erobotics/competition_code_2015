#ifndef CONTROL_H_
#define CONTROL_H_

#include "UEyeOpenCV.hpp"

//Lane change

//Camera Controllers
#define BINAY_THRESHOLD 130

//Track image controls
#define MID_LINE_DISTANCE 110
#define TURN_COEFICIENT 2.75
#define N_TO_KEEP 7
#define TETA_END_TURN_LEFT 8
#define TETA_END_TURN_RIGHT 13
#define HORIZONTAL 8
#define HORIZONTAL_LINE_THRESHOLD 25
#define VERTICAL_LINE_THRESHOLD 35

//Other
#define SPEED 100
#define Y_MAX_TO_DETECT_LANE 160
#define Y_MIN_TO_DETECT_LANE 0
#define Y_RATIO 0.9
#define DISTANCE_OF_END_TURN 100
#define AREA_TO_DETECT 40

//Zebra
#define HORIZONTAL_ANGLE 7.5
#define ZEBRA_Y_MAX_DISTANCE 80
#define ZEBRA_X_MAX_DISTANCE 45
#define MIN_ZEBRA_HEIGHT 125
#define MAX_ZEBRA_HEIGHT 250

//Devices
#define ARDUINO_CONTROL "//dev//ttyACM0"
#define DEVICE_TOP 7

///capture width and height
const int height = AOIHEIGHT;
const int width = AOIWIDTH;

#endif /* CONTROL_H_ */
