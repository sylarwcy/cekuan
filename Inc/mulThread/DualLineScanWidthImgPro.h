// ==================== [ DualLineScanWidthImgPro.h ] ====================
#pragma once
#include "ImageProcessor.h"
#include <QMap>
#include "workStationDataStructure.h"

class DualLineScanWidthImgPro {
public:
    DualLineScanWidthImgPro();
    virtual ~DualLineScanWidthImgPro();

    void setEdgeExtractionMode(bool use1D) { m_use1DMeasureMode = use1D; }

    bool initAlgorithm(const QString& masterDictPath, const QString& slaveDictPath, double encoderResolution);
    WidthResult processFrame(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight, long long frameID = -1);

    // 🌟 新增：离线特供通道，专门受理现成的 dispImage (拼接图) 进行极限测试！
    WidthResult processOfflineDispImage(const HalconCpp::HObject& dispImage, long long frameID = -1);

    void updatePixelResolution(bool isMaster, double newC);
    void updateBaselineOffset(double newOffset);
    void saveCurrentDictsToDisk();
    void resetBuffers();
    void setCalibMode(int mode) { m_calibMode = mode; }

private:
    void alignImages(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight, int DiffY, int frameHeight, double wM, double wS, HalconCpp::HObject& imgLeftAligned, HalconCpp::HObject& imgRightAligned);
    void extractEdges(const HalconCpp::HObject& img, int frameHeight, QVector<double>& outMinX, QVector<double>& outMaxX);

    void processMode1(const HalconCpp::HObject& imgLeftAligned, int frameHeight, int mStartX, int mWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode2(const HalconCpp::HObject& imgRightAligned, int frameHeight, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode3(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode0(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, int sSafeEndX, double wM, double wS, WidthResult& result, QVector<double>& vecWidths);

private:
    bool m_isInitialized{false};
    double m_encoder_mm_per_row{0.09473};

    bool m_use1DMeasureMode{true};

    HalconCpp::HObject m_hMasterBuffer;
    HalconCpp::HObject m_hSlaveBuffer;
    long long m_lastFrameID{-1};

    int m_lastCropS_X{1000};
    int m_lastCropE_X{2000};

    HalconCpp::HTuple a_m, b_m, c_m, d_m;
    HalconCpp::HTuple a_s, b_s, c_s, d_s;

    double m_cameraBaselineOffsetMM{245.0};

    int m_calibMode{0};
};