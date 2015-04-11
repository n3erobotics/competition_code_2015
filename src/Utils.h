/*
 * Utils.h
 *
 *  Created on: Mar 16, 2014
 *      Author: ruben
 */
 
#ifndef UTILS_H_
#define UTILS_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>
#include <cv.h>

#define PI 3.14159265

//Positioning
#define RIGHT 0
#define LEFT 1

//Vector predefined positions
#define X 0
#define Y 1
#define TETA 2
#define AREA 3
#define COUNTER 4

//Math with points
#define A 1.085
#define B -1
#define OFFSET_RIGHT 360
#define OFFSET_LEFT -417


//Colors
#define RED Scalar(0,0,255)
#define GREEN Scalar(0,255,0)
#define BLUE Scalar(255,0,0)
#define WHITE Scalar( 255, 255,255 )
#define YELLOW Scalar(0,255,255)
#define MAGENTA Scalar(255,0,255)
#define CIAN Scalar(255,255,0)

using namespace cv;

//waitKEYS
#define ESC 27
#define ENTER 13

// Timer class that, as the name specifies,
// counts the time in seconds between
// reset() and elapsed() function calls
class Timer
{
public:
    Timer() { clock_gettime(CLOCK_REALTIME, &beg_); }

    double elapsed() {
        clock_gettime(CLOCK_REALTIME, &end_);
        return end_.tv_sec - beg_.tv_sec +
            (end_.tv_nsec - beg_.tv_nsec) / 1000000000.;
    }

    void reset() { clock_gettime(CLOCK_REALTIME, &beg_); }

private:
    timespec beg_, end_;
};

// Structure used in sorting algorithms
// Usage: stable_sort( vector.begin(), vector.end(), compare_t( vector, vec_index ) );
struct compare_t
{
	// Our functor takes a reference to the table and a column to use when comparing rows
	vector <vector <double> > & matrix;
	unsigned                    column;

	compare_t(vector <vector <double> > & matrix, unsigned column):
		matrix( matrix ),
		column( column )
	{ }

	// This is the comparison function: taking two rows and comparing them
	// by the (previously) specified column
	bool operator () ( const vector <double> & lhrow, const vector <double> & rhrow ) const
	{
		return lhrow.at( column ) < rhrow.at( column );
	}
};


Mat get_frame(VideoCapture webcam);
bool wait_ESC();
void waiting(int key);
void SIG_handler(int signo);
void drawLane(vector<double> Lane, Mat &frame, const Scalar& color);
void drawFitLine(vector<float> linReg, Mat &frame, const Scalar& color);
int calculateDistanceToMidline(int x, int y, bool dir);

#endif /* UTILS_H_ */
