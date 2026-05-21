#include "DualLineScanWidthImgPro.h"
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QDir>             // 新增：用于操作文件夹
#include <QDateTime>        // 新增：用于获取时间戳
#include <QCoreApplication> // 新增：用于获取程序运行路径

DualLineScanWidthImgPro::DualLineScanWidthImgPro() {}

DualLineScanWidthImgPro::~DualLineScanWidthImgPro() {}

bool DualLineScanWidthImgPro::initAlgorithm(const QString& masterDictPath, const QString& slaveDictPath, double encoderResolution) {
    try {
        m_encoder_mm_per_row = encoderResolution;

        HalconCpp::HTuple dictMaster, dictSlave;
        HalconCpp::ReadDict(masterDictPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictMaster);
        HalconCpp::GetDictTuple(dictMaster, "Coef_a", &a_m);
        HalconCpp::GetDictTuple(dictMaster, "Coef_b", &b_m);
        HalconCpp::GetDictTuple(dictMaster, "Coef_c", &c_m);
        HalconCpp::GetDictTuple(dictMaster, "Coef_d", &d_m);

        HalconCpp::ReadDict(slaveDictPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple(), &dictSlave);
        HalconCpp::GetDictTuple(dictSlave, "Coef_a", &a_s);
        HalconCpp::GetDictTuple(dictSlave, "Coef_b", &b_s);
        HalconCpp::GetDictTuple(dictSlave, "Coef_c", &c_s);
        HalconCpp::GetDictTuple(dictSlave, "Coef_d", &d_s);

        m_isInitialized = true;
        return true;
    } catch (HalconCpp::HException &e) {
        qCritical() << "[算法错误] 字典加载失败:" << e.ErrorMessage().Text();
        m_isInitialized = false;
        return false;
    }
}

WidthResult DualLineScanWidthImgPro::processFrame(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight) {
    WidthResult result;
    result.isValid = false;

    if (!m_isInitialized || !imgLeft.IsInitialized() || !imgRight.IsInitialized()) {
        return result;
    }

    try {
        // =========================================================
        // 1. 获取基础参数与多项式系数
        // =========================================================
        HalconCpp::HTuple widthM_t, heightM_t, widthS_t, heightS_t;
        HalconCpp::GetImageSize(imgLeft, &widthM_t, &heightM_t);
        HalconCpp::GetImageSize(imgRight, &widthS_t, &heightS_t);

        double wM = widthM_t[0].D();
        double hM = heightM_t[0].D(); // 此时 hM = 380
        double wS = widthS_t[0].D();

        double am = a_m[0].D(), bm = b_m[0].D(), cm = c_m[0].D(), dm = d_m[0].D();
        double as = a_s[0].D(), bs = b_s[0].D(), cs = c_s[0].D(), ds = d_s[0].D();

        // =========================================================
        // 2. 图像物理行无损对齐 (智能双向支持)
        // =========================================================
        int DiffY = -7;
        int absDiffY = std::abs(DiffY);
        int frameHeight = (int)hM;

        HalconCpp::HObject imgLeftAligned, imgRightAligned;

        if (DiffY > 0) {
            // DiffY > 0: 主相机提前，对主相机进行行延迟缓冲
            if (!m_hMasterBuffer.IsInitialized()) {
                HalconCpp::CropPart(imgLeft, &m_hMasterBuffer, 0, 0, wM, absDiffY);
            }
            HalconCpp::HObject currentTop, nextBuffer;
            HalconCpp::CropPart(imgLeft, &currentTop, 0, 0, wM, frameHeight - absDiffY);
            HalconCpp::CropPart(imgLeft, &nextBuffer, frameHeight - absDiffY, 0, wM, absDiffY);

            HalconCpp::HObject verticalArray;
            HalconCpp::ConcatObj(m_hMasterBuffer, currentTop, &verticalArray);

            HalconCpp::HTuple rowOffV, colOffV, mOneV;
            rowOffV.Append(0); rowOffV.Append(absDiffY);
            colOffV.Append(0); colOffV.Append(0);
            mOneV.Append(-1);  mOneV.Append(-1);
            HalconCpp::TileImagesOffset(verticalArray, &imgLeftAligned, rowOffV, colOffV,
                                        mOneV, mOneV, mOneV, mOneV, wM, frameHeight);

            m_hMasterBuffer = nextBuffer;
            imgRightAligned = imgRight;

        } else if (DiffY < 0) {
            // DiffY < 0: 副相机提前，对副相机进行行延迟缓冲
            if (!m_hSlaveBuffer.IsInitialized()) {
                HalconCpp::CropPart(imgRight, &m_hSlaveBuffer, 0, 0, wS, absDiffY);
            }
            HalconCpp::HObject currentTop, nextBuffer;
            HalconCpp::CropPart(imgRight, &currentTop, 0, 0, wS, frameHeight - absDiffY);
            HalconCpp::CropPart(imgRight, &nextBuffer, frameHeight - absDiffY, 0, wS, absDiffY);

            HalconCpp::HObject verticalArray;
            HalconCpp::ConcatObj(m_hSlaveBuffer, currentTop, &verticalArray);

            HalconCpp::HTuple rowOffV, colOffV, mOneV;
            rowOffV.Append(0); rowOffV.Append(absDiffY);
            colOffV.Append(0); colOffV.Append(0);
            mOneV.Append(-1);  mOneV.Append(-1);
            HalconCpp::TileImagesOffset(verticalArray, &imgRightAligned, rowOffV, colOffV,
                                        mOneV, mOneV, mOneV, mOneV, wS, frameHeight);

            m_hSlaveBuffer = nextBuffer;
            imgLeftAligned = imgLeft;

        } else {
            // DiffY == 0: 无需补偿
            imgLeftAligned = imgLeft;
            imgRightAligned = imgRight;
        }

        // =========================================================
        // 3. 高密度轮廓提取
        // =========================================================
        double master_startX = 700.0;
        double cx_m = master_startX + (wM - master_startX) / 2.0;
        double half_w_m = (wM - master_startX) / 2.0;

        double cx_s = 3200.0 / 2.0;
        double half_w_s = 3200.0 / 2.0;

        QVector<double> vecWidths;
        QVector<double> vecMasterU;
        QVector<double> vecSlaveU;

        for (int y = 0; y < frameHeight; y++) {
            HalconCpp::HTuple HM, HS, RowM, ColM, AmpM, DistM, RowS, ColS, AmpS, DistS;

            HalconCpp::GenMeasureRectangle2(y, cx_m, 0, half_w_m, 1.0, wM, frameHeight, "nearest_neighbor", &HM);
            HalconCpp::MeasurePos(imgLeftAligned, HM, 2.0, 30, "negative", "first", &RowM, &ColM, &AmpM, &DistM);
            HalconCpp::CloseMeasure(HM);

            HalconCpp::GenMeasureRectangle2(y, cx_s, 0, half_w_s, 1.0, wS, frameHeight, "nearest_neighbor", &HS);
            HalconCpp::MeasurePos(imgRightAligned, HS, 2.0, 30, "positive", "first", &RowS, &ColS, &AmpS, &DistS);
            HalconCpp::CloseMeasure(HS);

            if (ColM.Length() > 0 && ColS.Length() > 0) {
                double um = ColM[0].D();
                double us = ColS[0].D();

                double XL = am*pow(um,3) + bm*pow(um,2) + cm*um + dm;
                double XR = as*pow(us,3) + bs*pow(us,2) + cs*us + ds;

                vecWidths.append(XR - XL);
                vecMasterU.append(um);
                vecSlaveU.append(us);
            }
        }

        // =========================================================
        // 4. 点云数据清洗与结算
        // =========================================================
        double u_master_best = -1.0;
        double u_slave_best = -1.0;

        if (vecWidths.size() > 20) {
            result.isValid = true;
            result.yawAngle = 0.0;

            QVector<double> sortedWidths = vecWidths;
            std::sort(sortedWidths.begin(), sortedWidths.end());
            result.widthValue = sortedWidths[sortedWidths.size() / 2];

            QVector<double> sortedMasterU = vecMasterU;
            QVector<double> sortedSlaveU = vecSlaveU;
            std::sort(sortedMasterU.begin(), sortedMasterU.end());
            std::sort(sortedSlaveU.begin(), sortedSlaveU.end());
            u_master_best = sortedMasterU[sortedMasterU.size() / 2];
            u_slave_best = sortedSlaveU[sortedSlaveU.size() / 2];
        }

        // =========================================================
        // 5. 图像固定拼接 (仅供 UI 渲染)
        // =========================================================
        int mStartX = 700;
        int mEndX = 4096;
        if (mEndX > (int)wM) mEndX = (int)wM;
        int mWidth = mEndX - mStartX;

        double X_Stitch_mm = am*pow(mEndX,3) + bm*pow(mEndX,2) + cm*mEndX + dm;
        int SCutX = 0;
        double min_diff = 9999999.0;
        for(int u = 0; u < (int)wS; u++) {
            double x_val = as*pow(u,3) + bs*pow(u,2) + cs*u + ds;
            double diff = std::abs(x_val - X_Stitch_mm);
            if(diff < min_diff) { min_diff = diff; SCutX = u; }
        }

        int sStartX = SCutX;
        int sEndX = 2900;
        if (sEndX > (int)wS) sEndX = (int)wS;
        int sWidth = sEndX - sStartX;
        if (sWidth < 0) sWidth = 0;

        HalconCpp::HObject ISC_Aligned, IMC_Aligned, DisplayObjs;
        HalconCpp::CropPart(imgRightAligned, &ISC_Aligned, 0, sStartX, sWidth, frameHeight);
        HalconCpp::CropPart(imgLeftAligned, &IMC_Aligned, 0, mStartX, mWidth, frameHeight);

        HalconCpp::ConcatObj(IMC_Aligned, ISC_Aligned, &DisplayObjs);

        HalconCpp::HTuple rowOff, colOff, mOne;
        rowOff.Append(0); rowOff.Append(0);
        colOff.Append(0); colOff.Append(mWidth);
        mOne.Append(-1);  mOne.Append(-1);

        HalconCpp::TileImagesOffset(DisplayObjs, &result.dispImage, rowOff, colOff,
                                    mOne, mOne, mOne, mOne, mWidth + sWidth, frameHeight);

        // =========================================================
        // 6. 计算 UI 画线坐标
        // =========================================================
        if (result.isValid) {
            result.renderLeftX = static_cast<int>(u_master_best) - mStartX;
            result.renderRightX = mWidth + (static_cast<int>(u_slave_best) - sStartX);
            result.renderY = frameHeight / 2;
        } else {
            result.renderLeftX = -1;
        }

        // =========================================================
        // 7. 调试存图 (⚠️ 核心修改：增加了 result.isValid 的判断，有板子才存)
        // =========================================================
        bool enableDebugSave = true; // 上产线前改为 false

        // 【修改点】：同时判断开关打开且当前帧测到了有效钢板才执行保存
        if (enableDebugSave && result.isValid) {
            try {
                QString dirPath = QCoreApplication::applicationDirPath() + "/DebugImages/";
                QDir dir(dirPath);
                if (!dir.exists()) dir.mkpath(".");

                QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");

                if (imgLeftAligned.IsInitialized()) {
                    HalconCpp::WriteImage(imgLeftAligned, "tiff", 0, (dirPath + "1_Aligned_Left_" + timeStr + ".tif").toLocal8Bit().constData());
                }
                if (imgRightAligned.IsInitialized()) {
                    HalconCpp::WriteImage(imgRightAligned, "tiff", 0, (dirPath + "2_Aligned_Right_" + timeStr + ".tif").toLocal8Bit().constData());
                }
                if (result.dispImage.IsInitialized()) {
                    HalconCpp::WriteImage(result.dispImage, "tiff", 0, (dirPath + "3_Stitched_" + timeStr + ".tif").toLocal8Bit().constData());
                }
            } catch (HalconCpp::HException &e) {
                qWarning() << "[调试存图异常] " << e.ErrorMessage().Text();
            }
        }

    } catch (HalconCpp::HException &e) {
        qWarning() << "[算法报警] 处理失败:" << e.ErrorMessage().Text();
        result.isValid = false;
        HalconCpp::GenEmptyObj(&result.dispImage);
    }

    return result;
}