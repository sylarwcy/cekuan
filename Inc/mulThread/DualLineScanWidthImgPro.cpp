#include "DualLineScanWidthImgPro.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

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
        double hM = heightM_t[0].D();
        double wS = widthS_t[0].D();

        double am = a_m[0].D(), bm = b_m[0].D(), cm = c_m[0].D(), dm = d_m[0].D();
        double as = a_s[0].D(), bs = b_s[0].D(), cs = c_s[0].D(), ds = d_s[0].D();

        // =========================================================
        // 2. 图像物理行无损双向对齐 (解决 4px 俯仰角错位)
        // =========================================================
        int DiffY = -8;
        int absDiffY = std::abs(DiffY);
        int frameHeight = (int)hM;

        HalconCpp::HObject imgLeftAligned, imgRightAligned;

        if (DiffY > 0) {
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
            imgLeftAligned = imgLeft;
            imgRightAligned = imgRight;
        }

        // =========================================================
        // 3. 图像固定拼接 (生成完整的拼接大图底板)
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

        // =======================================================================
        // 4 & 5. 🚀 核心集成：引入全局区域智能分割 + 您的极致形态学与灰度复合安全锁
        // =======================================================================
        bool isPlateValid = false;
        double u_master_best = -1.0;
        double u_slave_best = -1.0;
        double u_master_min = -1.0;
        double u_slave_max = -1.0;

        try {
            HalconCpp::HObject Region, ConnectedRegions, SelectedRegions, RegionFillUp;
            HalconCpp::HObject PlateContour, SmoothedContour;
            HalconCpp::HTuple UsedThreshold, NumRegions, NumSelected;
            HalconCpp::HTuple Row1, Column1, Row2, Column2, Convexity, MeanGray, Deviation;

            // 1. 自动最大类间方差黑块阈值分割
            HalconCpp::BinaryThreshold(result.dispImage, &Region, "max_separability", "dark", &UsedThreshold);
            // 2. 连通域分离分析
            HalconCpp::Connection(Region, &ConnectedRegions);
            HalconCpp::CountObj(ConnectedRegions, &NumRegions);

            if (NumRegions.I() > 0) {
                // 3. 精准锁定面积最大（权重最高）的实体区域
                HalconCpp::SelectShapeStd(ConnectedRegions, &SelectedRegions, "max_area", 70.0);
                HalconCpp::CountObj(SelectedRegions, &NumSelected);

                if (NumSelected.I() > 0) {
                    // 4. 填充区域内孤立孔洞
                    HalconCpp::FillUp(SelectedRegions, &RegionFillUp);
                    // 5. 获取最小外接正矩形边界
                    HalconCpp::SmallestRectangle1(RegionFillUp, &Row1, &Column1, &Row2, &Column2);

                    // 计算拼接图上的像素长度（跨度）
                    double lengthPx = Column2[0].D() - Column1[0].D();
                    // 计算凸度特征
                    HalconCpp::Convexity(RegionFillUp, &Convexity);
                    // 计算区域均值灰度
                    HalconCpp::Intensity(RegionFillUp, result.dispImage, &MeanGray, &Deviation);

                    // =================================================================
                    // 🌟【第一道关卡：尖部打捞弱安全门禁】
                    // 只有当区域平均灰度极度漆黑（MeanGray < 80）且长度大于 60 像素时，
                    // 才允许将其认定为真实钢板的异形尖部！从而一键把空载时的皮带缝隙和噪声过滤为 0 点。
                    // =================================================================
                    if (MeanGray.Length() > 0 && MeanGray[0].D() < 80.0 && lengthPx > 60.0) {

                        // 6. 弱校验通过！提取闭合外层轮廓边界并执行 7 阶高斯平滑滤波
                        HalconCpp::GenContourRegionXld(RegionFillUp, &PlateContour, "border");
                        HalconCpp::SmoothContoursXld(PlateContour, &SmoothedContour, 7);

                        // 提取平滑轮廓上全量连续离散点的亚像素级位置集合
                        HalconCpp::HTuple contourRows, contourCols;
                        HalconCpp::GetContourXld(SmoothedContour, &contourRows, &contourCols);

                        if (contourRows.Length() > 0) {
                            // 创建高效的行扫描离散池（Time complexity: O(N)）
                            std::vector<double> minX(frameHeight, -1.0);
                            std::vector<double> maxX(frameHeight, -1.0);

                            for (int i = 0; i < contourRows.Length(); ++i) {
                                int r = std::round(contourRows[i].D());
                                double c = contourCols[i].D();
                                if (r >= 0 && r < frameHeight) {
                                    if (minX[r] < 0 || c < minX[r]) minX[r] = c;
                                    if (maxX[r] < 0 || c > maxX[r]) maxX[r] = c;
                                }
                            }

                            QVector<double> vecWidths;
                            QVector<double> vecMasterU;
                            QVector<double> vecSlaveU;

                            // 逐行解析坐标并代入 1D LUT 多项式方程换算毫米宽度
                            for (int y = 0; y < frameHeight; y++) {
                                if (minX[y] >= 0.0 && maxX[y] >= 0.0 && minX[y] < maxX[y]) {
                                    // 坐标反向恢复：换算回主副相机的原始 U 坐标
                                    double um = minX[y] + mStartX;
                                    double us = (maxX[y] - mWidth) + sStartX;

                                    if (um >= 0 && um < wM && us >= 0 && us < wS) {
                                        double XL = am*pow(um,3) + bm*pow(um,2) + cm*um + dm;
                                        double XR = as*pow(us,3) + bs*pow(us,2) + cs*us + ds;

                                        vecWidths.append(XR - XL);
                                        vecMasterU.append(um);
                                        vecSlaveU.append(us);

                                        result.rowWidths.append(XR - XL);

                                        double finalW = XR - XL;
                                        // 当算出的宽度极度不合理时，打印出来看是哪一行、左边坐标多少、右边坐标多少
                                        if (finalW < 100.0) {
                                            qDebug() << "[异常宽度警告] 行:" << y << " 左:" << XL << " 右:" << XR << " 宽:" << finalW;
                                            try {
                                                QString path = QString("C:/debug_plate_err_frame.jpg");
                                                HalconCpp::WriteImage(result.dispImage, "jpeg", 0, path.toLocal8Bit().constData());
                                            } catch (...) {}
                                        }
                                        vecWidths.append(finalW);
                                    }
                                }
                            }

                            // 顺逆合并单线多边形闭合链条算法
                            for (int y = 0; y < frameHeight; y++) {
                                if (minX[y] >= 0.0 && maxX[y] >= 0.0 && minX[y] < maxX[y]) {
                                    result.contourRows.append(y);
                                    result.contourColsLeft.append(minX[y]);
                                }
                            }
                            for (int y = frameHeight - 1; y >= 0; y--) {
                                if (minX[y] >= 0.0 && maxX[y] >= 0.0 && minX[y] < maxX[y]) {
                                    result.contourRows.append(y);
                                    result.contourColsLeft.append(maxX[y]);
                                }
                            }
                            if (!result.contourRows.isEmpty()) {
                                result.contourRows.append(result.contourRows.first());
                                result.contourColsLeft.append(result.contourColsLeft.first());
                            }

                            // =================================================================
                            // 🌟【第二道关卡：合法大身强核验锁】：只有凸度 > 0.9 且 跨度 > 500 时才放行大身状态机
                            // =================================================================
                            if (Convexity.Length() > 0 && Convexity[0].D() > 0.9 && lengthPx > 500.0) {
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

                                    u_master_min = sortedMasterU.first();
                                    u_slave_max = sortedSlaveU.last();
                                }
                            }
                        }
                    }
                }
            }
        } catch (HalconCpp::HException &e) {
            qWarning() << "[全局智能提取算法崩溃阻断异常]:" << e.ErrorMessage().Text();
            isPlateValid = false;
        }

        // =========================================================
        // 6. 换算图形画线及切边相对坐标
        // =========================================================
        if (result.isValid) {
            result.renderLeftX = static_cast<int>(u_master_best) - mStartX;
            result.renderRightX = mWidth + (static_cast<int>(u_slave_best) - sStartX);
            result.renderY = frameHeight / 2;

            result.renderBoundLeftX = static_cast<int>(u_master_min) - mStartX;
            result.renderBoundRightX = mWidth + (static_cast<int>(u_slave_max) - sStartX);
        } else {
            result.renderLeftX = -1;
            result.renderBoundLeftX = -1;
            result.renderBoundRightX = -1;
        }

    } catch (HalconCpp::HException &e) {
        qWarning() << "[算法报警] 处理失败:" << e.ErrorMessage().Text();
        result.isValid = false;
        HalconCpp::GenEmptyObj(&result.dispImage);
    }

    return result;
}
