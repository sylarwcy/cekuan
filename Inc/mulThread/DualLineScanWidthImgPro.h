#pragma once
#include "ImageProcessor.h"
#include <QMap>
#include "workStationDataStructure.h"

class DualLineScanWidthImgPro {
public:
    DualLineScanWidthImgPro();
    virtual ~DualLineScanWidthImgPro();

    // 1. 初始化算法
    bool initAlgorithm(const QString& masterDictPath, const QString& slaveDictPath, double encoderResolution);

    // 2. 核心处理函数（380行高密度点云无损扫描版本）
    WidthResult processFrame(const HObject& imgLeft, const HObject& imgRight);

private:
    bool m_isInitialized{false};
    double m_encoder_mm_per_row{0.09473};
    HalconCpp::HObject m_hMasterBuffer;
    HalconCpp::HObject m_hSlaveBuffer; // 新增：副相机行延迟缓存口袋

    int m_lastCropS_X{1000};
    int m_lastCropE_X{2000};

    // 缓存主副相机的 1D LUT 多项式系数
    HalconCpp::HTuple a_m, b_m, c_m, d_m;
    HalconCpp::HTuple a_s, b_s, c_s, d_s;
};