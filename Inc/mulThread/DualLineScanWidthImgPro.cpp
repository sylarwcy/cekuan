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

        double cy[3] = { hM * 0.15, hM * 0.50, hM * 0.85 };
        double measureHeight_Half = 50.0;

        // =========================================================
        // 2. 卡尺寻边计算 (物理坐标系内完成)
        // =========================================================
        double master_startX = 100.0;
        double cx_m = master_startX + (wM - master_startX) / 2.0;
        double half_w_m = (wM - master_startX) / 2.0;
        double cx_s = 3200.0 / 2.0;
        double half_w_s = 3200.0 / 2.0;

        double Best_Raw_Width = 0.0;
        double Best_Amp_Sum = 0.0;
        int Valid_Zone_Count = 0;
        double X_Left_Top = -999.0, X_Left_Bottom = -999.0;
        double u_master_best = -1.0, u_slave_best = -1.0;

        for (int i = 0; i < 3; i++) {
            HalconCpp::HTuple HM, HS, RowM, ColM, AmpM, DistM, RowS, ColS, AmpS, DistS;

            HalconCpp::GenMeasureRectangle2(cy[i], cx_m, 0, half_w_m, measureHeight_Half, wM, hM, "nearest_neighbor", &HM);
            HalconCpp::MeasurePos(imgLeft, HM, 2.0, 30, "negative", "first", &RowM, &ColM, &AmpM, &DistM);
            HalconCpp::CloseMeasure(HM);

            HalconCpp::GenMeasureRectangle2(cy[i], cx_s, 0, half_w_s, measureHeight_Half, wS, hM, "nearest_neighbor", &HS);
            HalconCpp::MeasurePos(imgRight, HS, 2.0, 30, "positive", "first", &RowS, &ColS, &AmpS, &DistS);
            HalconCpp::CloseMeasure(HS);

            if (ColM.Length() > 0 && ColS.Length() > 0) {
                double um = ColM[0].D();
                double us = ColS[0].D();
                double XL = am*pow(um,3) + bm*pow(um,2) + cm*um + dm;
                double XR = as*pow(us,3) + bs*pow(us,2) + cs*us + ds;

                Valid_Zone_Count++;
                if (i == 0) X_Left_Top = XL;
                else if (i == 2) X_Left_Bottom = XL;

                double ampSum = std::abs(AmpM[0].D()) + std::abs(AmpS[0].D());
                if (ampSum > Best_Amp_Sum) {
                    Best_Amp_Sum = ampSum;
                    Best_Raw_Width = XR - XL;
                    u_master_best = um;
                    u_slave_best = us;
                }
            }
        }

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
        // 3. 图像固定拼接 (硬截取 + 自动算缝)
        // =========================================================
        int DiffY = 4; // 主相机提前的行数 (Y方向像素错位)
        int finalHeight = (int)hM; // 恢复全图高度，一像素都不丢！

        int mStartX = 700;
        int mEndX = 4096;
        if (mEndX > (int)wM) mEndX = (int)wM;
        int mWidth = mEndX - mStartX;

        // 自动计算副相机横向切点 SCutX
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

        // --- 核心：主相机无损行缓存逻辑 ---
        HalconCpp::HObject ISC_Aligned, IMC_Current;

        // 1. 【副相机】：因为副相机滞后，它就是基准时间点，直接取全高度！
        HalconCpp::CropPart(imgRight, &ISC_Aligned, 0, sStartX, sWidth, finalHeight);

        // 2. 【主相机】：先截取当前帧完整 X 范围 (此时还没切高度)
        HalconCpp::CropPart(imgLeft, &IMC_Current, 0, mStartX, mWidth, finalHeight);

        // 防呆：如果是程序刚启动的第一帧，兜里没东西，就拿当前帧的顶部垫一下
        if (!m_hMasterBuffer.IsInitialized()) {
            HalconCpp::CropPart(IMC_Current, &m_hMasterBuffer, 0, 0, mWidth, DiffY);
        }

        // 3. 切割主相机当前帧：
        // 头部拿去跟上一帧口袋里的尾巴拼 (高度: finalHeight - DiffY)
        // 尾部放进口袋，留给副相机的下一帧去用 (高度: DiffY)
        HalconCpp::HObject masterCurrentTop, masterNextBuffer;
        HalconCpp::CropPart(IMC_Current, &masterCurrentTop, 0, 0, mWidth, finalHeight - DiffY);
        HalconCpp::CropPart(IMC_Current, &masterNextBuffer, finalHeight - DiffY, 0, mWidth, DiffY);

        // 4. 纵向组装主相机对齐图：
        // 上一帧的尾部缓存 (DiffY) + 当前帧头部 (finalHeight - DiffY) = 完美还原的一帧
        HalconCpp::HObject verticalArray, IMC_Aligned;
        HalconCpp::ConcatObj(m_hMasterBuffer, masterCurrentTop, &verticalArray);

        // 【关键修复】：不能用 TileImages，必须用 TileImagesOffset 精确控制坐标！
        HalconCpp::HTuple rowOffV, colOffV, mOneV;
        rowOffV.Append(0); rowOffV.Append(DiffY); // 口袋里的图放 Y=0，当前大图放 Y=DiffY
        colOffV.Append(0); colOffV.Append(0);     // X 坐标都在最左侧 0 对齐
        mOneV.Append(-1);  mOneV.Append(-1);

        // 将两部分严丝合缝地纵向拼成高度为 finalHeight 的一整张图
        HalconCpp::TileImagesOffset(verticalArray, &IMC_Aligned, rowOffV, colOffV,
                                    mOneV, mOneV, mOneV, mOneV, mWidth, finalHeight);

        // 5. 更新口袋：把主相机当前帧超前拍到的尾部装进去
        m_hMasterBuffer = masterNextBuffer;

        // 6. 横向组装主副相机：使用 TileImagesOffset 严丝合缝贴合
        HalconCpp::HObject DisplayObjs;
        HalconCpp::ConcatObj(IMC_Aligned, ISC_Aligned, &DisplayObjs);

        HalconCpp::HTuple rowOff, colOff, mOne;
        rowOff.Append(0); rowOff.Append(0);
        colOff.Append(0); colOff.Append(mWidth);
        mOne.Append(-1);  mOne.Append(-1);

        HalconCpp::TileImagesOffset(DisplayObjs, &result.dispImage, rowOff, colOff,
                                    mOne, mOne, mOne, mOne, mWidth + sWidth, finalHeight);

        // =========================================================
        // 4. 计算 UI 画线坐标 (坐标相对于全尺寸拼接大图)
        // =========================================================
        if (result.isValid) {
            result.renderLeftX = static_cast<int>(u_master_best) - mStartX;
            result.renderRightX = mWidth + (static_cast<int>(u_slave_best) - sStartX);
            result.renderY = finalHeight / 2; // 回到全尺寸的中心线
        } else {
            result.renderLeftX = -1;
        }
    } catch (HalconCpp::HException &e) {
        qWarning() << "[算法报警] 处理失败:" << e.ErrorMessage().Text();
        result.isValid = false;
        HalconCpp::GenEmptyObj(&result.dispImage);
    }

    return result;
}