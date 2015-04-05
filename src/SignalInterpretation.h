/*
 * SingalInterpretation.h
 *
 *  Created on: May 10, 2014
 *      Author: ruben
 */

#ifndef SIGNALINTERPRETATION_H_
#define SIGNALINTERPRETATION_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>
#include <cv.h>
#include <sstream>
#include <cmath>
#include <signal.h>

#define RED_CROSS 0
#define YELLOW_RIGHT 1
#define YELLOW_LEFT 2
#define GREEN_FRONT 3
#define RG_FLAG 4

void *signalInterpretation(void *v_serialPort);

#endif /* SINGALINTERPRETATION_H_ */
