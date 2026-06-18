// ==================== [ DualLineScanWidthImgPro.cpp ] ====================
#include "DualLineScanWidthImgPro.h"
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QFile>
#include <QCoreApplication>

DualLineScanWidthImgPro::DualLineScanWidthImgPro() {}
DualLineScanWidthImgPro::~DualLineScanWidthImgPro() {}

void DualLineScanWidthImgPro::resetBuffers() {
    m_hMasterBuffer.Clear();
    m_hSlaveBuffer.Clear();
    m_lastFrameID = -1;
}

bool DualLineScanWidthImgPro::initAlgorithm(const QString &masterDictPath, const QString &slaveDictPath, double encoderResolution) {
    m_encoder_mm_per_row = encoderResolution;
    bool masterOk = false; bool slaveOk = false;
    HalconCpp::HTuple dictMaster; HalconCpp::HTuple dictSlave;

    try {
        if (QFile::exists(masterDictPath)) HalconCpp::ReadDict(masterDictPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictMaster);
        else if (QFile::exists(QCoreApplication::applicationDirPath() + "/Camera_Master_1DLUT.hdict")) HalconCpp::ReadDict((QCoreApplication::applicationDirPath() + "/Camera_Master_1DLUT.hdict").toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictMaster);
        else HalconCpp::ReadDict("./Camera_Master_1DLUT.hdict", HalconCpp::HTuple(), HalconCpp::HTuple(), &dictMaster);

        HalconCpp::GetDictTuple(dictMaster, "Coef_a", &a_m); HalconCpp::GetDictTuple(dictMaster, "Coef_b", &b_m);
        HalconCpp::GetDictTuple(dictMaster, "Coef_c", &c_m); HalconCpp::GetDictTuple(dictMaster, "Coef_d", &d_m);
        masterOk = true;
    } catch (...) {
        a_m = 0.0; b_m = 0.0; c_m = encoderResolution; d_m = 0.0;
    }

    try {
        QString appDir = QCoreApplication::applicationDirPath();
        if (QFile::exists(slaveDictPath)) HalconCpp::ReadDict(slaveDictPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictSlave);
        else if (QFile::exists(appDir + "/Camera_Slave_1DLUT_Global.hdict")) HalconCpp::ReadDict((appDir + "/Camera_Slave_1DLUT_Global.hdict").toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictSlave);
        else HalconCpp::ReadDict("./Camera_Slave_1DLUT.hdict", HalconCpp::HTuple(), HalconCpp::HTuple(), &dictSlave);

        HalconCpp::GetDictTuple(dictSlave, "Coef_a", &a_s); HalconCpp::GetDictTuple(dictSlave, "Coef_b", &b_s);
        HalconCpp::GetDictTuple(dictSlave, "Coef_c", &c_s); HalconCpp::GetDictTuple(dictSlave, "Coef_d", &d_s);

        try {
            HalconCpp::HTuple tBaseline;
            HalconCpp::GetDictTuple(dictSlave, "BaselineOffset", &tBaseline);
            m_cameraBaselineOffsetMM = tBaseline[0].D();
        } catch (...) {
            m_cameraBaselineOffsetMM = 245.0;
        }
        slaveOk = true;
    } catch (...) {
        a_s = 0.0; b_s = 0.0; c_s = encoderResolution; d_s = 245.0;
    }

    m_isInitialized = true;
    return (masterOk && slaveOk);
}

void DualLineScanWidthImgPro::alignImages(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight, int DiffY, int frameHeight, double wM, double wS, HalconCpp::HObject& imgLeftAligned, HalconCpp::HObject& imgRightAligned) {
    int absDiffY = std::abs(DiffY);
    imgLeftAligned = imgLeft;
    imgRightAligned = imgRight;

    if (imgLeft.IsInitialized() && imgRight.IsInitialized()) {
        try {
            if (DiffY > 0) {
                HalconCpp::HObject currentTop, rowPad, paddedTop, verticalArray;
                HalconCpp::CropPart(imgLeft, &currentTop, 0, 0, wM, frameHeight - absDiffY);
                HalconCpp::CropPart(imgLeft, &rowPad, 0, 0, wM, 1);
                HalconCpp::ZoomImageSize(rowPad, &paddedTop, wM, absDiffY, "constant");
                HalconCpp::ConcatObj(paddedTop, currentTop, &verticalArray);
                HalconCpp::TileImagesOffset(verticalArray, &imgLeftAligned, HalconCpp::HTuple(0).Append(absDiffY), HalconCpp::HTuple(0).Append(0), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), wM, frameHeight);
            } else if (DiffY < 0) {
                HalconCpp::HObject currentTop, rowPad, paddedTop, verticalArray;
                HalconCpp::CropPart(imgRight, &currentTop, 0, 0, wS, frameHeight - absDiffY);
                HalconCpp::CropPart(imgRight, &rowPad, 0, 0, wS, 1);
                HalconCpp::ZoomImageSize(rowPad, &paddedTop, wS, absDiffY, "constant");
                HalconCpp::ConcatObj(paddedTop, currentTop, &verticalArray);
                HalconCpp::TileImagesOffset(verticalArray, &imgRightAligned, HalconCpp::HTuple(0).Append(absDiffY), HalconCpp::HTuple(0).Append(0), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), wS, frameHeight);
            }
        } catch (...) {
            imgLeftAligned = imgLeft;
            imgRightAligned = imgRight;
        }
    }
}

// =======================================================================
// 🌟 降维打击：基于边缘梯度极性的一维测量（1D Caliper）
// =======================================================================
void DualLineScanWidthImgPro::extractEdges(const HalconCpp::HObject& img, int frameHeight, QVector<double>& outMinX, QVector<double>& outMaxX) {
    if (!img.IsInitialized()) return;
    try {
        HalconCpp::HTuple w_img, h_img;
        HalconCpp::GetImageSize(img, &w_img, &h_img);
        int width = w_img[0].I();
        int height = h_img[0].I();

        HalconCpp::HTuple measureHandle;

        // 🌟 修复编译错误：Halcon 的带旋转角的矩形永远以 Rectangle2 命名！
        // 创建一个长度等于图像宽、高度仅为 1 像素的卡尺，中心初始在第 0 行
        HalconCpp::GenMeasureRectangle2(0.5, width / 2.0, 0, width / 2.0, 0.5, width, height, "nearest_neighbor", &measureHandle);

        for (int r = 0; r < height; ++r) {
            // 将卡尺平移到当前行扫描
            HalconCpp::TranslateMeasure(measureHandle, r + 0.5, width / 2.0);

            HalconCpp::HTuple rowEdge, columnEdge, amplitude, distance;

            // Sigma=2.0 (平滑毛刺), Threshold=30 (忽略微弱渐变), "all" (抓取所有超过阈值的跳变边缘)
            HalconCpp::MeasurePos(img, measureHandle, 2.0, 30.0, "all", "all", &rowEdge, &columnEdge, &amplitude, &distance);

            double firstPos = -1.0;
            double lastNeg = -1.0;

            // 遍历该行找到的所有亚像素边缘
            for (int i = 0; i < columnEdge.Length(); ++i) {
                if (amplitude[i].D() > 0) {
                    // 🌟 正梯度 (暗 -> 亮)：说明这是钢板的左边缘！
                    if (firstPos < 0) firstPos = columnEdge[i].D(); // 我们只信任最左侧出现的第一个钢板正边缘
                } else {
                    // 🌟 负梯度 (亮 -> 暗)：说明这是钢板的右边缘！
                    lastNeg = columnEdge[i].D(); // 不断更新，最后留下的一定是最右侧的负边缘
                }
            }

            if (firstPos >= 0) {
                if (outMinX[r] < 0 || firstPos < outMinX[r]) outMinX[r] = firstPos;
            }
            if (lastNeg >= 0) {
                if (outMaxX[r] < 0 || lastNeg > outMaxX[r]) outMaxX[r] = lastNeg;
            }
        }

        // 必须释放卡尺句柄防内存泄漏
        HalconCpp::CloseMeasure(measureHandle);
    } catch (...) {}
}

void DualLineScanWidthImgPro::processMode1(const HalconCpp::HObject& imgLeftAligned, int frameHeight, int mStartX, int mWidth, WidthResult& result, QVector<double>& vecWidths) {
    HalconCpp::CropPart(imgLeftAligned, &result.dispImage, 0, mStartX, mWidth, frameHeight);
    QVector<double> minX(frameHeight, -1.0), maxX(frameHeight, -1.0);
    extractEdges(result.dispImage, frameHeight, minX, maxX);

    double frameMinCol = 9999.0; double frameMaxCol = -1.0;
    for(int y = 0; y < frameHeight; y++) {
        if(minX[y] >= 0 && maxX[y] > minX[y]) {
            double pixel_span = maxX[y] - minX[y];
            result.rowWidths.append(pixel_span);
            result.contourRows.append(y);
            result.contourColsLeft.append(minX[y]);
            vecWidths.append(pixel_span);
            if(minX[y] < frameMinCol) frameMinCol = minX[y];
            if(maxX[y] > frameMaxCol) frameMaxCol = maxX[y];
        }
    }
    for (int y = frameHeight - 1; y >= 0; y--) {
        if(minX[y] >= 0 && maxX[y] > minX[y]) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxX[y]);
        }
    }
    if (!vecWidths.isEmpty()) {
        result.renderLeftX = static_cast<int>(frameMinCol);
        result.renderRightX = static_cast<int>(frameMaxCol);
        result.renderY = frameHeight / 2;
    }
}

void DualLineScanWidthImgPro::processMode2(const HalconCpp::HObject& imgRightAligned, int frameHeight, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths) {
    HalconCpp::CropPart(imgRightAligned, &result.dispImage, 0, sSafeStartX, sSafeWidth, frameHeight);
    QVector<double> minX(frameHeight, -1.0), maxX(frameHeight, -1.0);
    extractEdges(result.dispImage, frameHeight, minX, maxX);

    double frameMinCol = 9999.0; double frameMaxCol = -1.0;
    for(int y = 0; y < frameHeight; y++) {
        if(minX[y] >= 0 && maxX[y] > minX[y]) {
            double pixel_span = maxX[y] - minX[y];
            result.rowWidths.append(pixel_span);
            result.contourRows.append(y);
            result.contourColsLeft.append(minX[y]);
            vecWidths.append(pixel_span);
            if(minX[y] < frameMinCol) frameMinCol = minX[y];
            if(maxX[y] > frameMaxCol) frameMaxCol = maxX[y];
        }
    }
    for (int y = frameHeight - 1; y >= 0; y--) {
        if(minX[y] >= 0 && maxX[y] > minX[y]) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxX[y]);
        }
    }
    if (!vecWidths.isEmpty()) {
        result.renderLeftX = static_cast<int>(frameMinCol);
        result.renderRightX = static_cast<int>(frameMaxCol);
        result.renderY = frameHeight / 2;
    }
}

void DualLineScanWidthImgPro::processMode3(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths) {
    HalconCpp::HObject IMC, ISC, Disp;
    HalconCpp::CropPart(imgLeftAligned, &IMC, 0, mStartX, mWidth, frameHeight);
    HalconCpp::CropPart(imgRightAligned, &ISC, 0, sSafeStartX, sSafeWidth, frameHeight);
    HalconCpp::ConcatObj(IMC, ISC, &Disp);
    HalconCpp::TileImagesOffset(Disp, &result.dispImage, HalconCpp::HTuple(0).Append(0), HalconCpp::HTuple(0).Append(mWidth), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), mWidth + sSafeWidth, frameHeight);

    QVector<double> minL(frameHeight, -1.0), maxL(frameHeight, -1.0);
    QVector<double> minR(frameHeight, -1.0), maxR(frameHeight, -1.0);
    extractEdges(IMC, frameHeight, minL, maxL);
    extractEdges(ISC, frameHeight, minR, maxR);

    double frameMinCol = 9999.0; double frameMaxCol = -1.0;
    for(int y = 0; y < frameHeight; y++) {
        if (maxL[y] >= 0 && maxR[y] >= 0) {
            double um = maxL[y] + mStartX;
            double us = maxR[y] + sSafeStartX;
            double x_m = a_m[0].D() * pow(um, 3) + b_m[0].D() * pow(um, 2) + c_m[0].D() * um + d_m[0].D();
            double x_s = a_s[0].D() * pow(us, 3) + b_s[0].D() * pow(us, 2) + c_s[0].D() * us + d_s[0].D();
            double offset = x_m - x_s;

            result.rowWidths.append(offset);
            vecWidths.append(offset);
            if (maxL[y] < frameMinCol) frameMinCol = maxL[y];
            if (maxR[y] + mWidth > frameMaxCol) frameMaxCol = maxR[y] + mWidth;
        }
    }

    for (int y = 0; y < frameHeight; y++) {
        if (maxL[y] >= 0 && maxR[y] >= 0) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxL[y]);
        }
    }
    for (int y = frameHeight - 1; y >= 0; y--) {
        if (maxL[y] >= 0 && maxR[y] >= 0) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxL[y]);
        }
    }
    if (!vecWidths.isEmpty()) {
        result.renderLeftX = static_cast<int>(frameMinCol);
        result.renderRightX = static_cast<int>(frameMaxCol);
        result.renderY = frameHeight / 2;
    }
}

void DualLineScanWidthImgPro::processMode0(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, int sSafeEndX, double wM, double wS, WidthResult& result, QVector<double>& vecWidths) {
    double X_Stitch_mm = a_m[0].D() * pow(mStartX+mWidth, 3) + b_m[0].D() * pow(mStartX+mWidth, 2) + c_m[0].D() * (mStartX+mWidth) + d_m[0].D();
    int SCutX = sSafeStartX; double min_diff = 9999999.0;
    for (int u = sSafeStartX; u < sSafeEndX; u++) {
        double x_val = a_s[0].D() * pow(u, 3) + b_s[0].D() * pow(u, 2) + c_s[0].D() * u + d_s[0].D() + m_cameraBaselineOffsetMM;
        if (std::abs(x_val - X_Stitch_mm) < min_diff) { min_diff = std::abs(x_val - X_Stitch_mm); SCutX = u; }
    }
    int sStartX = SCutX;
    int sCutWidth = sSafeEndX - sStartX;
    if (sCutWidth < 0) sCutWidth = 0;

    HalconCpp::HObject ISC, IMC;
    HalconCpp::CropPart(imgRightAligned, &ISC, 0, sStartX, sCutWidth, frameHeight);
    HalconCpp::CropPart(imgLeftAligned, &IMC, 0, mStartX, mWidth, frameHeight);
    HalconCpp::ConcatObj(IMC, ISC, &result.dispImage);
    HalconCpp::TileImagesOffset(result.dispImage, &result.dispImage, HalconCpp::HTuple(0).Append(0), HalconCpp::HTuple(0).Append(mWidth), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), HalconCpp::HTuple(-1).Append(-1), mWidth + sCutWidth, frameHeight);

    QVector<double> minL(frameHeight, -1.0), maxL(frameHeight, -1.0);
    QVector<double> minR(frameHeight, -1.0), maxR(frameHeight, -1.0);
    extractEdges(IMC, frameHeight, minL, maxL);
    extractEdges(ISC, frameHeight, minR, maxR);

    double frameMinCol = 9999.0; double frameMaxCol = -1.0;

    for(int y = 0; y < frameHeight; y++) {
        if(minL[y] >= 0 && maxR[y] >= 0) {
            double um = minL[y] + mStartX;
            double us = maxR[y] + sStartX;
            double XL = a_m[0].D() * pow(um, 3) + b_m[0].D() * pow(um, 2) + c_m[0].D() * um + d_m[0].D();
            double XR = a_s[0].D() * pow(us, 3) + b_s[0].D() * pow(us, 2) + c_s[0].D() * us + d_s[0].D() + m_cameraBaselineOffsetMM;
            double finalW = XR - XL;

            result.rowWidths.append(finalW);
            result.contourRows.append(y);
            result.contourColsLeft.append(minL[y]);
            vecWidths.append(finalW);

            if(minL[y] < frameMinCol) frameMinCol = minL[y];
            if(maxR[y] + mWidth > frameMaxCol) frameMaxCol = maxR[y] + mWidth;

        } else if (minL[y] >= 0 && maxL[y] >= 0 && minR[y] < 0) {
            double umL = minL[y] + mStartX;
            double umR = maxL[y] + mStartX;
            double XL = a_m[0].D() * pow(umL, 3) + b_m[0].D() * pow(umL, 2) + c_m[0].D() * umL + d_m[0].D();
            double XR = a_m[0].D() * pow(umR, 3) + b_m[0].D() * pow(umR, 2) + c_m[0].D() * umR + d_m[0].D();
            double finalW = XR - XL;

            result.rowWidths.append(finalW);
            result.contourRows.append(y);
            result.contourColsLeft.append(minL[y]);
            vecWidths.append(finalW);

            if(minL[y] < frameMinCol) frameMinCol = minL[y];
            if(maxL[y] > frameMaxCol) frameMaxCol = maxL[y];

        } else if (minR[y] >= 0 && maxR[y] >= 0 && minL[y] < 0) {
            double usL = minR[y] + sStartX;
            double usR = maxR[y] + sStartX;
            double XL = a_s[0].D() * pow(usL, 3) + b_s[0].D() * pow(usL, 2) + c_s[0].D() * usL + d_s[0].D() + m_cameraBaselineOffsetMM;
            double XR = a_s[0].D() * pow(usR, 3) + b_s[0].D() * pow(usR, 2) + c_s[0].D() * usR + d_s[0].D() + m_cameraBaselineOffsetMM;
            double finalW = XR - XL;

            result.rowWidths.append(finalW);
            result.contourRows.append(y);
            result.contourColsLeft.append(minR[y] + mWidth);
            vecWidths.append(finalW);

            if(minR[y] + mWidth < frameMinCol) frameMinCol = minR[y] + mWidth;
            if(maxR[y] + mWidth > frameMaxCol) frameMaxCol = maxR[y] + mWidth;
        }
    }

    for (int y = frameHeight - 1; y >= 0; y--) {
        if(minL[y] >= 0 && maxR[y] >= 0) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxR[y] + mWidth);
        } else if (minL[y] >= 0 && maxL[y] >= 0 && minR[y] < 0) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxL[y]);
        } else if (minR[y] >= 0 && maxR[y] >= 0 && minL[y] < 0) {
            result.contourRows.append(y);
            result.contourColsLeft.append(maxR[y] + mWidth);
        }
    }

    if (!vecWidths.isEmpty()) {
        result.renderLeftX = static_cast<int>(frameMinCol);
        result.renderRightX = static_cast<int>(frameMaxCol);
        result.renderY = frameHeight / 2;
    }
}

// =======================================================================
// 主核心管道 (极其清爽解耦)
// =======================================================================
WidthResult DualLineScanWidthImgPro::processFrame(const HalconCpp::HObject &imgLeft, const HalconCpp::HObject &imgRight, long long frameID) {
    WidthResult result;
    result.isValid = false;

    if (!m_isInitialized || (!imgLeft.IsInitialized() && !imgRight.IsInitialized())) return result;

    try {
        HalconCpp::HTuple widthM_t, heightM_t, widthS_t, heightS_t;
        double wM = 4096.0, hM = 200.0, wS = 4096.0;

        if (imgLeft.IsInitialized()) {
            HalconCpp::GetImageSize(imgLeft, &widthM_t, &heightM_t);
            wM = widthM_t[0].D(); hM = heightM_t[0].D();
        }
        if (imgRight.IsInitialized()) {
            HalconCpp::GetImageSize(imgRight, &widthS_t, &heightS_t);
            wS = widthS_t[0].D();
        }
        int frameHeight = (int) hM;

        if (frameID != -1) {
            if (m_lastFrameID != -1 && frameID != m_lastFrameID + 1) {
                m_hMasterBuffer.Clear(); m_hSlaveBuffer.Clear();
            }
            m_lastFrameID = frameID;
        }

        HalconCpp::HObject imgLeftAligned = imgLeft, imgRightAligned = imgRight;
        alignImages(imgLeft, imgRight, -8, frameHeight, wM, wS, imgLeftAligned, imgRightAligned);

        // =========================================================
        // 🌟 物理死区拦截：强制对齐用户指定的现场边界参数！
        // =========================================================
        int mStartX = 270;
        int mEndX = wM;
        if (mEndX > (int) wM) mEndX = (int) wM;
        int mWidth = mEndX - mStartX;

        int sSafeStartX = 0;
        int sSafeEndX = std::min(3520, (int)wS);
        int sSafeWidth = sSafeEndX - sSafeStartX;

        QVector<double> vecWidths;

        if (m_calibMode == 1) {
            processMode1(imgLeftAligned, frameHeight, mStartX, mWidth, result, vecWidths);
        } else if (m_calibMode == 2) {
            processMode2(imgRightAligned, frameHeight, sSafeStartX, sSafeWidth, result, vecWidths);
        } else if (m_calibMode == 3) {
            processMode3(imgLeftAligned, imgRightAligned, frameHeight, mStartX, mWidth, sSafeStartX, sSafeWidth, result, vecWidths);
        } else {
            processMode0(imgLeftAligned, imgRightAligned, frameHeight, mStartX, mWidth, sSafeStartX, sSafeWidth, sSafeEndX, wM, wS, result, vecWidths);
        }

        if (!result.contourRows.isEmpty()) {
            result.contourRows.append(result.contourRows.first());
            result.contourColsLeft.append(result.contourColsLeft.first());
        }

        if (vecWidths.size() > 5) {
            result.isValid = true;
            std::sort(vecWidths.begin(), vecWidths.end());
            result.widthValue = vecWidths[vecWidths.size() / 2];
        } else {
            result.isValid = false;
        }

    } catch (HalconCpp::HException &e) {
        qWarning() << "[算法异常]:" << e.ErrorMessage().Text();
        result.isValid = false;
    }

    return result;
}

void DualLineScanWidthImgPro::updatePixelResolution(bool isMaster, double newC) {
    if (isMaster) c_m = newC; else c_s = newC;
}

void DualLineScanWidthImgPro::updateBaselineOffset(double newOffset) {
    m_cameraBaselineOffsetMM = newOffset;
}

void DualLineScanWidthImgPro::saveCurrentDictsToDisk() {
    try {
        QString appDir = QCoreApplication::applicationDirPath();
        HalconCpp::HTuple dictM; HalconCpp::CreateDict(&dictM);
        HalconCpp::SetDictTuple(dictM, "Coef_a", a_m); HalconCpp::SetDictTuple(dictM, "Coef_b", b_m);
        HalconCpp::SetDictTuple(dictM, "Coef_c", c_m); HalconCpp::SetDictTuple(dictM, "Coef_d", d_m);
        HalconCpp::WriteDict(dictM, (appDir + "/Camera_Master_1DLUT.hdict").toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple());

        HalconCpp::HTuple dictS; HalconCpp::CreateDict(&dictS);
        HalconCpp::SetDictTuple(dictS, "Coef_a", a_s); HalconCpp::SetDictTuple(dictS, "Coef_b", b_s);
        HalconCpp::SetDictTuple(dictS, "Coef_c", c_s); HalconCpp::SetDictTuple(dictS, "Coef_d", d_s);
        HalconCpp::SetDictTuple(dictS, "BaselineOffset", HalconCpp::HTuple(m_cameraBaselineOffsetMM));
        HalconCpp::WriteDict(dictS, (appDir + "/Camera_Slave_1DLUT.hdict").toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple());
    } catch (...) {}
}