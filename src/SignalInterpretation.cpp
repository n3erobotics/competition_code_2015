/*
 * SignalInterpretation.cpp
 *
 *  Created on: May 10, 2014
 *      Author: ruben
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>
#include <cv.h>
#include <sstream>
#include <cmath>
#include <signal.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "SignalInterpretation.h"
#include "Utils.h"
#include "Control.h"

using namespace cv;
using namespace std;


#define BOUNDING_AREA 5500		//Min area to consider for analysis
#define BINARY_THRESHOLD 70		//Binary Threshold for filter binary calculation and signals definition


//TODO::: PARAMETROS PARA MELHORAR DEFINICAO DOS SEMAPHOROS
#define IMAGE_SEMAPHORE_THRESHOLD 70		//mudar aqui para meter imagem de teste mais escura
#define IMAGE_SEMAPHORE_THRESHOLD_X 70
#define BETWEEN_SEMAPHORE_CHECK_SLEEP 2	//sleep 5 seconds
#define COMER_FRAMES 40
#define SHARPEN_LVL 0.8		// de 0 a 1.5 sendo 1.5 maximo (best at 0.7 i guess?)
//TODO HEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEERE BITCHHHHHHHHHHHHHHHHH




//100
#define SQUARE_RECTANGLE 0.40
#define RESIZE_XY_SIZE Size(600,600)
#define CANNY_THRESHOLD 70
//70
#define MIN_MATCH 4
#define IMAGE_BIN_COUNT 10000000

#define TINY_NAME_BYTES		10
#define SMALL_NAME_BYTES	50

using namespace cv;
using namespace std;

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

typedef struct _define_raw_test{
	Mat src_gray;

	Mat bin;
	Mat equalized_histogram;
	string filter_name;
	vector<Point2i> filter_mc_points;
}define_raw_test;
define_raw_test src_filtered[3];

typedef struct _compare_signal{
	Mat ref;
	string name;
	Mat src_gray;
	vector<KeyPoint> keypoints;
	Mat descriptors;
}define_compare_signal;
vector<define_compare_signal> compare_signals, side_compare_signals;

vector<Mat> possible_signals;
vector<Rect> possible_signals_rect;

//Camera Image
Mat src;
Mat src_bgr[3];
Mat src_gray;
vector<KeyPoint> keypoints;
Mat descriptors;
int thresh = CANNY_THRESHOLD;
Mat src_2;
Mat src_2_gray;
Mat aux_mat[3];
Mat src_puro;
Mat sharpen;

/// Function header
void find_possible_signals(int num, void *argv);
//void test_compare_for_signal(void*arg);

extern VideoCapture webcamT;
extern pthread_mutex_t access_displays, access_HUB;
extern int display_detected;
Mat frame;

void * signalInterpretation(void *)
{
	//TODO:: ALTERA AQUI
	VideoCapture webcamT;
	webcamT.open(DEVICE_TOP);
	webcamT.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	webcamT.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	//TODO:: ALTERA AQUI
	//Get Signals and Resize
	const char *lista_sinais = "Signs/Signals.txt";	//caminho que contem todos os sinais para contra-teste
	const char *sinais_laterais = "Signs/Signals2.txt";
	fstream fp;
	fp.open(lista_sinais,std::fstream::in);
	Mat aux, resize_aux;
	Mat Aux[3];
	string name, append = "Signs/";;
	Mat img_matches;
	Mat testest;
int count=0;
	while(!fp.eof()){
		define_compare_signal aux_compare_signal;
		fp >> name;
		name = (append + name);
		aux_compare_signal.name = name;
		aux = imread(aux_compare_signal.name.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
		//equalizeHist(aux, aux);
		blur(aux,aux, Size(3,3) );
		if(count==0){
			count++;
			threshold(aux,aux, IMAGE_SEMAPHORE_THRESHOLD_X, 255,THRESH_BINARY);
		}
		else{
			threshold(aux,aux, IMAGE_SEMAPHORE_THRESHOLD, 255,THRESH_BINARY);
		}
//		imshow("TESTT0EST",aux);waitKey(0);
		resize(aux, resize_aux,RESIZE_XY_SIZE);
		resize_aux.copyTo(aux_compare_signal.ref);
		compare_signals.push_back(aux_compare_signal);
	}
	fp.close();
	fp.open(sinais_laterais,std::fstream::in);
	Ptr<FeatureDetector> detector(new OrbFeatureDetector(400,1.15,31,0, 3, ORB::HARRIS_SCORE, 31));
	Ptr<DescriptorExtractor> extractor(new OrbDescriptorExtractor(400));
	BFMatcher matcher(NORM_HAMMING2);
	vector<DMatch> matches;
	while(!fp.eof()){
		define_compare_signal aux_side_compare_signals;
		fp >> name;
		name = (append + name);
		aux_side_compare_signals.name = name;
		aux = imread(aux_side_compare_signals.name.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
		equalizeHist(aux, aux);
		resize(aux, resize_aux,RESIZE_XY_SIZE);
		resize_aux.copyTo(aux_side_compare_signals.ref);
		side_compare_signals.push_back(aux_side_compare_signals);
	}
	fp.close();
	for(unsigned int num_signals = 0; num_signals<side_compare_signals.size();num_signals++){
		//GET COMPARE IMAGE KEYPOINTS AND DESCRIPTORS
		blur(side_compare_signals[num_signals].ref,side_compare_signals[num_signals].src_gray, Size(3,3) );
		detector->detect(side_compare_signals[num_signals].src_gray,side_compare_signals[num_signals].keypoints);
		extractor->compute(side_compare_signals[num_signals].src_gray,side_compare_signals[num_signals].keypoints,side_compare_signals[num_signals].descriptors);
		//waitKey(1);
	}
	bool disregard_semaphore = false;
//	side_compare_signals.clear(); //TODO TIRAR AQUIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
	while(1){
		//Sharpen image
		src = get_frame(webcamT);
		GaussianBlur(src,sharpen,cv::Size(0, 0), 3);
		////imshow("Blur",image);
		addWeighted(src, 1.5, sharpen, -(SHARPEN_LVL), 0, sharpen); //imagem peso imagem peso
		////imshow("sharpen",sharpen);waitKey(1);

		split(sharpen,src_bgr);
		src_filtered[0].src_gray=src_bgr[0];
		src_filtered[1].src_gray=src_bgr[1];
		src_filtered[2].src_gray=src_bgr[2];
		blur(src_filtered[0].src_gray,src_filtered[0].src_gray, Size(3,3) );
		src_filtered[0].filter_name = "CannyB";
		blur(src_filtered[1].src_gray,src_filtered[1].src_gray, Size(3,3) );
		src_filtered[1].filter_name = "CannyG";
		blur(src_filtered[2].src_gray,src_filtered[2].src_gray, Size(3,3) );
		src_filtered[2].filter_name = "CannyR";

		threshold(src_filtered[0].src_gray,src_filtered[0].bin, BINARY_THRESHOLD, 255,THRESH_BINARY);
		threshold(src_filtered[1].src_gray,src_filtered[1].bin, BINARY_THRESHOLD, 255,THRESH_BINARY);
		threshold(src_filtered[2].src_gray,src_filtered[2].bin, BINARY_THRESHOLD, 255,THRESH_BINARY);
		Aux[0] = src_filtered[0].bin - (src_filtered[0].bin & src_filtered[1].bin & src_filtered[2].bin);
		Aux[1] = src_filtered[1].bin - (src_filtered[0].bin & src_filtered[1].bin & src_filtered[2].bin);
		Aux[2] = src_filtered[2].bin - (src_filtered[0].bin & src_filtered[1].bin & src_filtered[2].bin);
		src_filtered[0].bin = Aux[0];
		src_filtered[1].bin = Aux[1];
		src_filtered[2].bin = Aux[2];

		//Aply for each -> find_possible_signals(num_args,Mat *src_grey,const char <window name>)
		find_possible_signals( 1, (void *) &src_filtered[0]);
		find_possible_signals( 1, (void *) &src_filtered[1]);
		find_possible_signals( 1, (void *) &src_filtered[2]);

		Mat test_src,aux_test,test_gray;
		string signal_name;
		while(!possible_signals.empty()){
			test_src = possible_signals.back();
			cvtColor(test_src,test_src,CV_RGB2GRAY);
			test_src.copyTo(test_gray);
			threshold(test_src,test_src, IMAGE_SEMAPHORE_THRESHOLD, 255,THRESH_BINARY);
			detector->detect(test_gray,keypoints);
			extractor->compute(test_gray,keypoints,descriptors);

			int i;
			float distThreshold = 38;
			int matchCount=0,best_match=-1, best_match_count=0;

			for(unsigned int num_signals = 0; (num_signals<compare_signals.size()) || disregard_semaphore;num_signals++){
			//imshow("COMPARE",compare_signals[num_signals].ref);
				Scalar soma_compare = sum(compare_signals[num_signals].ref);
			imshow("TEST",test_src);waitKey(0);
				Scalar soma_test = sum(test_src);
				bitwise_xor(test_src,compare_signals[num_signals].ref,aux_test);
//				//imshow("AND",aux_test);
				Scalar soma_resultado = sum(aux_test);
//				cout << "COMPARE: " << soma_compare << "  TEST: " << soma_test << "   RESULT: " << soma_resultado << endl;
//			waitKey(0);
				if((soma_test[0] <= soma_compare[0]+10000000) && (soma_test[0] >= soma_compare[0]-10000000) && (soma_resultado[0]<=IMAGE_BIN_COUNT)){
					//					cout << compare_signals[num_signals].name.c_str() << endl;
					signal_name =compare_signals[num_signals].name;
					////imshow("Problema",test_src);waitKey(1);
					//cout << "COMPARE: " << soma_compare << "  TEST: " << soma_test << "   RESULT: " << soma_resultado << endl;
					switch(num_signals){
					case 0:
					case 7:
//						if(soma_resultado[0]<10000000){
//							cout << "jump over X" << endl;
//							continue;}
						cout << "DETECTED CROSS"<<endl;
						pthread_mutex_lock(&access_displays);

						display_detected=RED_CROSS;

						pthread_mutex_unlock(&access_displays);

						break;
					case 1:
						disregard_semaphore=true;
						cout << "DETECTED YELLOW LEFT"<<endl;
						pthread_mutex_lock(&access_displays);

						display_detected=YELLOW_LEFT;

						pthread_mutex_unlock(&access_displays);

						break;
					case 2:
						disregard_semaphore=true;
						cout << "DETECTED YELLOW RIGHT"<<endl;
						pthread_mutex_lock(&access_displays);

						display_detected=YELLOW_RIGHT;

						pthread_mutex_unlock(&access_displays);

						break;
					case 3:
						disregard_semaphore=true;
						cout << "DETECTED GREEN ARROW"<< endl;
						pthread_mutex_lock(&access_displays);

						display_detected=GREEN_FRONT;

						pthread_mutex_unlock(&access_displays);

						break;
					}
					if(disregard_semaphore){
						//cout << "breaking out of Reference Mat vector loop" << endl;
						break;
					}
					//cout << "Reference Mat vector break FAILED" << endl;
				}
				if(disregard_semaphore){
					//cout << "breaking from main While loop" <<endl;
					break;
				}
				//cout << "main break failed" << endl;

				for(unsigned int num_signals = 0; num_signals<side_compare_signals.size();num_signals++){
					matchCount=0;
					matcher.match(side_compare_signals[num_signals].descriptors, descriptors, matches);
					for (unsigned int n=0; n<matches.size(); ++n) {
						if (matches.size() > 0){
							if (matches[n].distance > distThreshold){
								//matches[n].erase(matches.begin());
							}else{
								++matchCount;
							}
						}
						if(matchCount>best_match_count /*&& best_match_count >= 15*/){
							best_match = num_signals;
							best_match_count=matchCount;
							//							cout << "bestmatch" << best_match << "count" << best_match_count << endl;
						}
					}
					i = best_match;
					if((best_match==0 && best_match_count >22) || (best_match==1 && best_match_count >10) || (best_match==2 && best_match_count >8) || (best_match==3 && best_match_count >13) || (best_match==4 && best_match_count >16) || (best_match==5 && best_match_count >18) || (best_match==6 && best_match_count >10)){
						/////////////////////FUNÇÃO AQUI//////////////////////////////
						//cout << "match:" << best_match << "from:" << side_compare_signals[best_match].name << endl;
						cout << "DETECTED FLAAG"<< endl;
						pthread_mutex_lock(&access_displays);

						display_detected=RG_FLAG;

						pthread_mutex_unlock(&access_displays);


					}
				}
				//waitKey(1);
			}
			possible_signals.pop_back();
		}
		//TODO: PROBLEMAS COM CAMARA PODEM SER AQUI
		if(disregard_semaphore){
			disregard_semaphore=false;
			possible_signals.clear();
			cout << "going to sleep" <<endl;
			sleep(BETWEEN_SEMAPHORE_CHECK_SLEEP);
			for(int num_frames=0;num_frames<COMER_FRAMES;num_frames++)
				src = get_frame(webcamT);
			cout << "done sleeping" <<endl;
		}
		possible_signals_rect.clear();
		//		waitKey(0);
	}
	destroyAllWindows();

	pthread_exit(NULL);

}




/** @function find_possible_signals */
void find_possible_signals(int num, void *argv)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat bin;
	define_raw_test *data = (define_raw_test *) argv;
	RNG rng(12345);

	/// Detect edges using canny
	Canny( data->bin, canny_output, CANNY_THRESHOLD, CANNY_THRESHOLD*2, 3 );

	/// Find contours
	findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	/// Approximate contours to polygons + get bounding rects and circles
	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

	for( unsigned int i = 0; i < contours.size(); i++ )
	{ approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
	boundRect[i] = boundingRect( Mat(contours_poly[i]) );
	//minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
	}

	/// Draw polygonal contour + bonding rects + circles
	Mat drawingg;
	src.copyTo(drawingg);

	Mat aux_resize;
	bool correr = true;
	for(unsigned int i = 0; i< contours.size(); i++ ){
		if(boundRect[i].area()>BOUNDING_AREA && !((boundRect[i].height > (boundRect[i].width * (1+SQUARE_RECTANGLE))) || (boundRect[i].height < (boundRect[i].width * (1-SQUARE_RECTANGLE))))){
			for(unsigned int num_rect = 0; num_rect<possible_signals_rect.size(); num_rect++){
				if(possible_signals_rect[num_rect]==boundRect[i]){
					correr = false; break;}
			}
			//			drawContours( drawingg, contours_poly, i, Scalar(0,255,0), 1, 8, vector<Vec4i>(), 0, Point() );
			//			//imshow("Drawing",drawingg);waitKey(0);
			if(correr){
				Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
				Rect rectangulo(boundRect[i]);
//				drawContours( drawingg, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//				rectangle( drawingg, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
				resize(src(boundRect[i]),aux_resize,RESIZE_XY_SIZE);
				//resize(sharpen(boundRect[i]),aux_resize,RESIZE_XY_SIZE);
				possible_signals.push_back(aux_resize);
				possible_signals_rect.push_back(rectangulo);
			}
		}
	}
//	imshow("Drawing",drawingg);waitKey(1);
}
