#include "DualLineScanWidthImgPro.h"
#include <QDebug>
#include <cmath>

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
        // 1. 彻底抛弃 Tuple 运算，全部转为 C++ 原生 double
        // =========================================================
        HalconCpp::HTuple widthM_t, heightM_t, widthS_t, heightS_t;
        HalconCpp::GetImageSize(imgLeft, &widthM_t, &heightM_t);
        HalconCpp::GetImageSize(imgRight, &widthS_t, &heightS_t);

        double wM = widthM_t[0].D();
        double hM = heightM_t[0].D();
        double wS = widthS_t[0].D();
        double hS = heightS_t[0].D();

        // 提取字典系数 (转为纯 C++ double)
        double am = a_m[0].D(), bm = b_m[0].D(), cm = c_m[0].D(), dm = d_m[0].D();
        double as = a_s[0].D(), bs = b_s[0].D(), cs = c_s[0].D(), ds = d_s[0].D();

        double cy[3] = { hM * 0.15, hM * 0.50, hM * 0.85 };
        double measureHeight_Half = 50.0;

        double master_startX = 100.0;
        if (master_startX >= wM) master_startX = 0.0;
        double cx_m = master_startX + (wM - master_startX) / 2.0;
        double half_w_m = (wM - master_startX) / 2.0;

        double slave_endX = 3200.0;
        if (slave_endX > wS) slave_endX = wS;
        double cx_s = slave_endX / 2.0;
        double half_w_s = slave_endX / 2.0;

        double Best_Amp_Sum = 0.0;
        double Best_Raw_Width = 0.0;
        int Valid_Zone_Count = 0;
        double X_Left_Top = -999.0, X_Left_Bottom = -999.0;

        double u_master_best = -1.0, u_slave_best = -1.0;
        int best_y = -1;

        // =========================================================
        // 2. 卡尺寻边
        // =========================================================
        for (int i = 0; i < 3; i++) {
            HalconCpp::HTuple HM, HS;
            HalconCpp::HTuple RowM, ColM, AmpM, DistM;
            HalconCpp::HTuple RowS, ColS, AmpS, DistS;

            HalconCpp::GenMeasureRectangle2(cy[i], cx_m, 0, half_w_m, measureHeight_Half, wM, hM, "nearest_neighbor", &HM);
            HalconCpp::MeasurePos(imgLeft, HM, 2.0, 15, "negative", "first", &RowM, &ColM, &AmpM, &DistM);
            HalconCpp::CloseMeasure(HM);

            HalconCpp::GenMeasureRectangle2(cy[i], cx_s, 0, half_w_s, measureHeight_Half, wS, hS, "nearest_neighbor", &HS);
            HalconCpp::MeasurePos(imgRight, HS, 2.0, 15, "positive", "first", &RowS, &ColS, &AmpS, &DistS);
            HalconCpp::CloseMeasure(HS);

            if (ColM.Length() > 0 && ColS.Length() > 0) {
                double um = ColM[0].D();
                double us = ColS[0].D();

                // 纯 C++ 运算，永不崩溃
                double XL_Zone = am*pow(um,3) + bm*pow(um,2) + cm*um + dm;
                double XR_Zone = as*pow(us,3) + bs*pow(us,2) + cs*us + ds;

                Valid_Zone_Count++;
                if (i == 0) X_Left_Top = XL_Zone;
                else if (i == 2) X_Left_Bottom = XL_Zone;

                double currentAmpSum = std::abs(AmpM[0].D()) + std::abs(AmpS[0].D());
                if (currentAmpSum > Best_Amp_Sum) {
                    Best_Amp_Sum = currentAmpSum;
                    Best_Raw_Width = XR_Zone - XL_Zone;
                    u_master_best = um;
                    u_slave_best = us;
                    best_y = (int)cy[i];
                }
            }
        }

        // 姿态补偿
        if (Valid_Zone_Count > 0) {
            result.isValid = true;
            if (X_Left_Top != -999.0 && X_Left_Bottom != -999.0) {
                double DeltaX = X_Left_Bottom - X_Left_Top;
                double DeltaY = (cy[2] - cy[0]) * m_encoder_mm_per_row;
                double YawAngle_rad = std::atan2(std::abs(DeltaX), DeltaY);
                result.yawAngle = YawAngle_rad * 180.0 / 3.141592653589793;
                result.widthValue = Best_Raw_Width * std::cos(YawAngle_rad);
            } else {
                result.widthValue = Best_Raw_Width;
                result.yawAngle = 0.0;
            }
        }

        // =========================================================
        // 3. 图像拼接 (用纯 C++ for 循环替代 Halcon 矩阵运算)
        // =========================================================
        int DiffY = 12;
        HalconCpp::HObject IMC, ISC;
        HalconCpp::CropPart(imgLeft, &IMC, 0, 0, wM, hM - DiffY);
        HalconCpp::CropPart(imgRight, &ISC, DiffY, 0, wS, hM - DiffY);

        double w_m1 = wM - 1.0;
        double X_Stitch_mm = am*pow(w_m1,3) + bm*pow(w_m1,2) + cm*w_m1 + dm;

        int SCutX = 0;
        double min_diff = 9999999.0;
        int ws_int = (int)wS;

        // 纯 C++ 暴力寻找最佳拼接口，速度极快且防弹
        for(int u = 0; u < ws_int; u++) {
            double x_val = as*pow(u,3) + bs*pow(u,2) + cs*u + ds;
            double diff = std::abs(x_val - X_Stitch_mm);
            if(diff < min_diff) {
                min_diff = diff;
                SCutX = u;
            }
        }

        int SlaveW_Val = ws_int - SCutX;
        HalconCpp::HObject ISN;
        HalconCpp::CropPart(ISC, &ISN, 0, SCutX, SlaveW_Val, hM - DiffY);

        HalconCpp::HObject DisplayObjs, StitchedImage;
        HalconCpp::ConcatObj(IMC, ISN, &DisplayObjs);

        HalconCpp::HTuple rowOff, colOff, mOne;
        rowOff.Append(0); rowOff.Append(0);
        colOff.Append(0); colOff.Append((int)wM);
        mOne.Append(-1);  mOne.Append(-1);

        HalconCpp::TileImagesOffset(DisplayObjs, &StitchedImage, rowOff, colOff,
                                    mOne, mOne, mOne, mOne, (int)wM + SlaveW_Val, (int)hM - DiffY);

        // =========================================================
        // 4. 精准视窗裁剪与打包数据
        // =========================================================
        int finalS_X, finalE_X;

        if (result.isValid) {
            // 有钢板：根据边缘动态计算窗口，并更新记忆
            int lineL_X = static_cast<int>(u_master_best);
            int lineR_X = (int)wM + static_cast<int>(u_slave_best) - SCutX;

            finalS_X = std::max(0, lineL_X - 150); // 留出150px边距
            finalE_X = std::min((int)wM + SlaveW_Val, lineR_X + 150);

            // 更新“粘性窗口”
            m_lastCropS_X = finalS_X;
            m_lastCropE_X = finalE_X;

            // 设置 UI 渲染所需的相对坐标
            result.renderLeftX = lineL_X - finalS_X;
            result.renderRightX = lineR_X - finalS_X;
            result.renderY = (int)(hM * 0.5); // 默认在中间画个标记
        } else {
            // 没钢板：使用最后一次记忆的窗口位置对当前传送带画面进行裁切
            finalS_X = m_lastCropS_X;
            finalE_X = m_lastCropE_X;

            // 确保窗口没越界（防止图像尺寸突变）
            if (finalE_X > ((int)wM + SlaveW_Val)) finalE_X = (int)wM + SlaveW_Val;

            result.renderLeftX = -1; // 标记为无效，UI 不画线
        }

        // 无论有无钢板，都输出同样大小的裁切图，实现背景实时刷新
        HalconCpp::CropPart(StitchedImage, &result.dispImage, 0, finalS_X, finalE_X - finalS_X, hM - DiffY);

    } catch (HalconCpp::HException &e) {
        qWarning() << "[算法报警] 测宽计算异常. 错误码:" << e.ErrorCode() << " 详情:" << e.ErrorMessage().Text();
        result.isValid = false;
        HalconCpp::GenEmptyObj(&result.dispImage);
    }

    return result;
}