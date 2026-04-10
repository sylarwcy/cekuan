//
// Created by wcy on 25-8-1.
//

#ifndef HALCONOCRINFER_H
#define HALCONOCRINFER_H

#include "halconcpp/HalconCpp.h"
#include "halconcpp/HDevThread.h"

using namespace HalconCpp;

// Chapter: OCR / Deep OCR
// Short Description: Get the detection word boxes contained in the specified ResultKey.
void get_deep_ocr_detection_word_boxes (HTuple hv_DeepOcrResult, HTuple hv_ResultKey,
    HTuple *hv_WordBoxRow, HTuple *hv_WordBoxCol, HTuple *hv_WordBoxPhi, HTuple *hv_WordBoxLength1,
    HTuple *hv_WordBoxLength2);
// Short Description: Retrieve a deep learning device to work with.
void get_inference_dl_device (HTuple hv_UseFastAI2Devices, HTuple *hv_DLDevice);



#endif //HALCONOCRINFER_H
