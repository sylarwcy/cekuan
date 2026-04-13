#pragma once
#include "ImageProcessor.h"
#include <QMap>
#include "workStationDataStructure.h"

class DualLineScanWidthImgPro {
public:
    DualLineScanWidthImgPro();
    virtual ~DualLineScanWidthImgPro();

    // 核心处理函数：传入左右相机图像，返回测量结果
    WidthResult process(const HObject& imgLeft, const HObject& imgRight);

    // 参数设置：标定参数与相机物理安装参数
    // pixelResLeft/Right: 像素当量 (mm/pixel)
    // cameraBaseDistance: 两台相机视野零点之间的物理基线距离 (mm)
    void setCalibrationParams(double pixelResLeft, double pixelResRight, double cameraBaseDistance);

    // 算法参数设置（可在UI动态调节）
    void setAlgorithmParams(int edgeThreshold, double edgeAlpha);

private:
    // 提取单侧图像的边缘亚像素位置
    // direction: "positive"由暗到亮(左边缘), "negative"由亮到暗(右边缘)
    bool extractEdge(const HObject& img, const HTuple& transition, double& edgeColPos);

private:
    // --- 标定参数 ---
    double m_pixelResLeft;
    double m_pixelResRight;
    double m_cameraBaseDistance;

    // --- 算法参数 ---
    HTuple m_edgeAlpha;      // Canny滤波器的平滑系数
    HTuple m_edgeThreshold;  // 边缘梯度阈值

    // 缓存结果用于可视化等
    WidthResult m_lastResult;
};
