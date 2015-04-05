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
// Copyright © 2012, HU University of Applied Sciences Utrecht. 
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

UeyeOpencvCam::UeyeOpencvCam(int wdth, int heigh) {
        width = wdth;
        height = heigh;
        using std::cout;
        using std::endl;
        mattie = cv::Mat(AOIHEIGHT, AOIWIDTH, CV_8UC3);
        hCam = 0;
        char* ppcImgMem;
        int pid;
        INT nAOISupported = 0;
        double on = 1;
        double empty, min, max, interval;
        int retInt = IS_SUCCESS;
        IS_RECT rectAOI;
        UINT nRange[3];
        
        ZeroMemory(nRange, sizeof(nRange));
        
        rectAOI.s32X = AOIX;
        rectAOI.s32Y = AOIY;
        rectAOI.s32Width = AOIWIDTH;
        rectAOI.s32Height = AOIHEIGHT;
        
        /*INIT CAMERA*/
        retInt = is_InitCamera(&hCam, 0);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        /*PIXEL CLOCK*/
        retInt = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nRange, sizeof(nRange));
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }else{
			UINT pixelClock = PIXELCLOCK;
			cout << "PIXELCLOCK Min: " << nRange[0]  << " Max: " << nRange[1]  << " Inc: " << nRange[2] << endl;
			retInt = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelClock, sizeof(pixelClock));
			if (retInt != IS_SUCCESS) {
				throw UeyeOpenCVException(hCam, retInt);
			}else{
				
				is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&pixelClock, sizeof(pixelClock));
				cout << "Using: " << pixelClock << endl;
			}
		}
		/*FRAMERATE*/
		retInt = is_GetFrameTimeRange(hCam, &min, &max, &interval);
		if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }else{
			cout << "FRAMERATE Min: " << 1/min << " Max: " << 1/max << " Inc: " << 1/interval << endl;
		    retInt = is_SetFrameRate(hCam, FRAMERATE, NULL);
			if (retInt != IS_SUCCESS) {
					throw UeyeOpenCVException(hCam, retInt);
			}
			double FPS;
			is_GetFramesPerSecond(hCam, &FPS);
			cout << "Using: " << FPS << endl;
		}
		/*EXPOSURE*/
		
        retInt = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_ImageFormat(hCam, IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, (void*) &nAOISupported, sizeof(nAOISupported));
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_AllocImageMem(hCam, width, height, 24, &ppcImgMem, &pid);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_SetImageMem(hCam, ppcImgMem, pid);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        //set auto settings
        retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &on, &empty);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }
        retInt = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
        }        
        retInt = is_CaptureVideo(hCam, IS_DONT_WAIT);
        if (retInt != IS_SUCCESS) {
                throw UeyeOpenCVException(hCam, retInt);
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
                memcpy(mat.ptr(), pMem, AOIWIDTH * AOIHEIGHT * 3);
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
