//
// Created by wcy on 25-8-1.
//

#ifndef HALCONPREPROCESSING_H
#define HALCONPREPROCESSING_H

#include "halconcpp/HalconCpp.h"

using namespace HalconCpp;

class HalconPreprocessing {
    HTuple hv_HomMat2D;

public:
    void AffineInit(const HTuple &hv_x_pre, const HTuple &hv_y_pre,
                           const HTuple &hv_x_post, const HTuple &hv_y_post);

    void Affine(const HObject &img, HObject &ho_TransImage);

    static HTuple GetSuitableInferDevice();
    static int affineHeight,affineWidth;

    // HObject m_bgImg;
};


#endif //HALCONPREPROCESSING_H
