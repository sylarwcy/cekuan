#pragma once
#include "ImageProcessor.h"
#include <QMap>
#include <QVector>
#include "workStationDataStructure.h"

class DualLineScanWidthImgPro {
public:
    DualLineScanWidthImgPro();
    virtual ~DualLineScanWidthImgPro();

    // 1. 初始化算法（加载LUT字典，仅在程序启动时调用一次）
    bool initAlgorithm(const QString& masterDictPath, const QString& slaveDictPath, double encoderResolution);

    // 2. 核心处理函数（单帧测宽 + 姿态补偿）
    WidthResult processFrame(const HObject& imgLeft, const HObject& imgRight);

private:
    bool m_isInitialized{false};
    double m_encoder_mm_per_row{0.09473}; // 线阵走带当量
    HalconCpp::HObject m_hMasterBuffer;
    HalconCpp::HObject m_hSlaveBuffer;

    // 【新增】：记录最后的裁切窗口位置，用于在无钢板时刷新背景
    int m_lastCropS_X{1000};
    int m_lastCropE_X{2000};

    // 缓存主副相机的 1D LUT 多项式系数
    HalconCpp::HTuple a_m, b_m, c_m, d_m;
    HalconCpp::HTuple a_s, b_s, c_s, d_s;
};
