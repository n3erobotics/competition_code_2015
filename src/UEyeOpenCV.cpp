//******************************************************************************
//
//                 Low Cost Vision
//
//******************************************************************************
// Project:        ueyeOpencv
// File:           UEyeOpenCV.cpp
// Description:    Wrapper class of UEye camera to support OpenCV Mat using the UEye SDK
// Author:         Wouter Langerak
// Notes:          For more functionalities use the SDK of UEye, the purpose of this project is to make it compatible with OpenCV Mat.
//
// License: newBSD 
//  
// Copyright �� 2012, HU University of Applied Sciences Utrecht.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// - Neither the name of the HU University of Applied Sciences Utrecht nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE HU UNIVERSITY OF APPLIED SCIENCES UTRECHT
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//******************************************************************************

#include "UEyeOpenCV.hpp"
#include <iostream>
#include <ueye.h>

using namespace std;

UeyeOpencvCam::UeyeOpencvCam() {
        using std::cout;
        using std::endl;
        mattie = cv::Mat(AOIHEIGHT, AOIWIDTH, CV_8UC1);
        hCam = 0;
        char* ppcImgMem;
        int pid;
        INT nAOISupported = 0;
        double on = 1;
        double empty, min, max, interval;
        int retInt = IS_SUCCESS;
		double exposureTime;
        IS_RECT rectAOI;
        UINT nRange[3];
        
        ZeroMemory(nRange, sizeof(nRange));
        
        rectAOI.s32X = AOIX;
        rectAOI.s32Y = AOIY;
        rectAOI.s32Width = AOIWIDTH;
        rectAOI.s32Height = AOIHEIGHT;
        
        //INIT CAMERA
        retInt = is_InitCamera(&hCam, 0);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't initialize camera, make sure it is connected!" << endl;
        }
        
        //retInt = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
        retInt = is_SetColorMode(hCam, IS_CM_MONO8);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set colormode" << endl;
        }
        retInt = is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set AOI" << endl;
        }
        INT gamma = GAMMA;
        retInt = is_Gamma(hCam, IS_GAMMA_CMD_SET, (void *)&gamma, sizeof(gamma));
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set gamma" << endl;
        }
        INT temperature = RGB_COLOR_MODEL_ADOBE_RGB_D65;
        retInt = is_ColorTemperature(hCam, COLOR_TEMPERATURE_CMD_SET_RGB_COLOR_MODEL, (void *)&temperature, sizeof(temperature));
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set temperature" << endl;
        }
        retInt = is_SetHardwareGain(hCam, 100, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set gainboost" << endl;
        }
        retInt = is_SetGainBoost(hCam, IS_SET_GAINBOOST_ON);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set gainboost" << endl;
        }
        INT blacklevel = IS_AUTO_BLACKLEVEL_ON;
        retInt = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_SET_MODE, (void*)&blacklevel, sizeof(blacklevel));
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set displaymode" << endl;
        }
        retInt = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set displaymode" << endl;
        }
        retInt = is_ImageFormat(hCam, IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, (void*) &nAOISupported, sizeof(nAOISupported));
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set image format" << endl;
        }
        retInt = is_AllocImageMem(hCam, AOIWIDTH, AOIHEIGHT, 8, &ppcImgMem, &pid);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't allocate memory for image" << endl;
        }
        retInt = is_SetImageMem(hCam, ppcImgMem, pid);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set image memory" << endl;
        }
        //set auto settings
        /*retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &on, &empty);
		if (retInt != IS_SUCCESS) {
                cout << "Couldn't set automatic white balance" << endl;
        }
        retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't set automatic gain" << endl;
        }*/
        
        //PIXEL CLOCK
        UINT pixelClock = PIXELCLOCK;
        is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nRange, sizeof(nRange));
		cout << "PIXELCLOCK Min: " << nRange[0]  << " Max: " << nRange[1] << endl;
		retInt = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelClock, sizeof(pixelClock));
		if (retInt != IS_SUCCESS) {
			cout << "Couldn't set pixelclock range!" << endl;
		}
		is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&pixelClock, sizeof(pixelClock));
		cout << "Using: " << pixelClock << endl;

		//FRAMERATE
		is_GetFrameTimeRange(hCam, &min, &max, &interval);
		cout << "FRAMERATE Min: " << 1/min << " Max: " << 1/max << endl;
		double NEWFPS;
		retInt = is_SetFrameRate(hCam, FRAMERATE, &NEWFPS);
		if (retInt != IS_SUCCESS) {
				cout << "Couldn't set framerate!" << endl;
		}
		cout << "Using: " << NEWFPS << endl;

		//EXPOSURE
		exposureTime = EXPOSURE;
		is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN, (void*) &min, sizeof(min));
		cout << "EXPOSURE Min: " << min << " Max: " << max << endl;
		retInt = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*) &exposureTime, sizeof(exposureTime));
		if (retInt != IS_SUCCESS) {
			cout << "Couldn't set exposure" << endl;
		}
		is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*) &exposureTime, sizeof(exposureTime));
		cout << "Using: " << exposureTime << endl;
        
        retInt = is_CaptureVideo(hCam, IS_WAIT);
        if (retInt != IS_SUCCESS) {
                cout << "Couldn't start video capture" << endl;
        }
}

UeyeOpencvCam::~UeyeOpencvCam() {
        int retInt = is_ExitCamera(hCam);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
}

cv::Mat UeyeOpencvCam::getFrame() {
        getFrame_mem(mattie);
        return mattie;
}

void UeyeOpencvCam::getFrame_mem(cv::Mat& mat) {
        VOID* pMem;
        
        int retInt = is_GetImageMem(hCam, &pMem);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
//      if (mat.cols == width && mat.rows == height && mat.depth() == 3) {
                memcpy(mat.ptr(), pMem, AOIWIDTH * AOIHEIGHT );
//      } else {
//              throw UeyeOpenCVException(hCam, -1337);
//      }
}

HIDS UeyeOpencvCam::getHIDS() {
        return hCam;
}

void UeyeOpencvCam::setAutoWhiteBalance(bool set) {
        double empty;
        double on = set ? 1 : 0;
        int retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &on, &empty);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
}

void UeyeOpencvCam::setAutoGain(bool set) {
        double empty;
        double on = set ? 1 : 0;
        int retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
}
void UeyeOpencvCam::getInfo() {
        SENSORINFO Info;
        int retInt = IS_SUCCESS;
        
        retInt = is_GetSensorInfo(hCam, &Info);
        if (retInt != IS_SUCCESS) {
			cout << "rebenta por todo o lado" << endl;
                throw UeyeOpenCVException(hCam, retInt);
        }
        cout << "Sensor name: " << Info.SensorID << endl <<
			"Camera Module: " << Info.strSensorName << endl <<
			"Color Mode: " << (int)Info.nColorMode << endl <<
			"Width: " << Info.nMaxWidth << endl <<
			"Height: " << Info.nMaxHeight << endl <<
			"Gain: " << Info.bMasterGain << endl <<
			"Gain Red: " << Info.bRGain << endl <<
			"Gain Green: " << Info.bGGain << endl <<
			"Gain Blue: " << Info.bBGain << endl <<
			"Global Shutter: " << Info.bGlobShutter << endl <<
			"Pixel Size: " << ((float_t)Info.wPixelSize)/100.0 << "um" << endl <<
			"Color of 1st Pixel: " << (int)Info.nUpperLeftBayerPixel << endl;
}

void UeyeOpencvCam::close(){
		is_ExitCamera(hCam);
}	
