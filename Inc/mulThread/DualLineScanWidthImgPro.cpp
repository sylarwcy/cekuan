#include "DualLineScanWidthImgPro.h"
#include <QDebug>
// 假设此函数在你的图像处理类中运行
// void DualLineScanWidthImgPro::processAndStitch(HObject ho_ImageLeft, HObject ho_ImageRight) {
//     HObject ho_ImageFull, ho_ImagesConcat;

//     try {
//         // 1. 合并对象
//         GenEmptyObj(&ho_ImagesConcat);
//         ConcatObj(ho_ImageLeft, ho_ImageRight, &ho_ImagesConcat);

//         // 2. 计算像素级偏移
//         // Y偏移 = 物理位移 / 像素当量
//         // X偏移 = 物理位移 / 像素当量
//         HTuple hv_RowOffsets, hv_ColOffsets, hv_MinusOne;
//         hv_RowOffsets.Append(0).Append(AppConfig::StitchOffsetY / AppConfig::StitchScale);
//         hv_ColOffsets.Append(0).Append(AppConfig::StitchOffsetX / AppConfig::StitchScale);
//         hv_MinusOne.Append(-1).Append(-1);

//         // 3. 执行拼接
//         TileImagesOffset(ho_ImagesConcat, &ho_ImageFull,
//                          hv_RowOffsets, hv_ColOffsets,
//                          hv_MinusOne, hv_MinusOne, hv_MinusOne, hv_MinusOne,
//                          AppConfig::StitchTotalWidth, AppConfig::StitchTotalHeight);

//         // 4. 使用你要求的链路进行显示
//         if (!HtupleIsEmpty(winHandle_pro)) {
//             // 激活窗口
//             HDevWindowStack::SetActive(winHandle_pro);

//             if (HDevWindowStack::IsOpen()) {
//                 // 关闭即时刷新，防止大图闪烁
//                 SetSystem("flush_graphic", "false");

//                 // 获取拼接图尺寸并设置显示视口（自适应缩放的关键）
//                 HTuple hv_FullW, hv_FullH;
//                 GetImageSize(ho_ImageFull, &hv_FullW, &hv_FullH);
//                 SetPart(winHandle_pro, 0, 0, hv_FullH - 1, hv_FullW - 1);

//                 // 显示拼接完成的大图
//                 DispObj(ho_ImageFull, winHandle_pro);

//                 // 恢复刷新
//                 SetSystem("flush_graphic", "true");
//             }
//         }

//         // 5. 清理内存
//         ho_ImagesConcat.Clear();
//         // 如果 ho_ImageFull 不需要传给下一级，也记得 Clear

//     } catch (HException &ex) {
//         // 异常处理逻辑
//     }
// }
DualLineScanWidthImgPro::DualLineScanWidthImgPro()
    : m_pixelResLeft(0.1),
      m_pixelResRight(0.1),
      m_cameraBaseDistance(1500.0) // 假设两相机视野内侧相距1500mm
{
    // 默认算法参数
    m_edgeAlpha = 2.0;
    m_edgeThreshold = 30;
}

DualLineScanWidthImgPro::~DualLineScanWidthImgPro() {
    // 释放可能持有的持久化 Halcon 句柄（如有）
}

void DualLineScanWidthImgPro::setCalibrationParams(double pixelResLeft, double pixelResRight, double cameraBaseDistance) {
    m_pixelResLeft = pixelResLeft;
    m_pixelResRight = pixelResRight;
    m_cameraBaseDistance = cameraBaseDistance;
}

void DualLineScanWidthImgPro::setAlgorithmParams(int edgeThreshold, double edgeAlpha) {
    m_edgeThreshold = edgeThreshold;
    m_edgeAlpha = edgeAlpha;
}

WidthResult DualLineScanWidthImgPro::process(const HObject& imgLeft, const HObject& imgRight) {
    WidthResult result;
    result.isOk = false;
    result.widthValue = 0.0;
    result.centerOffset = 0.0;

    // 1. 图像有效性校验
    if (!imgLeft.IsInitialized() || !imgRight.IsInitialized()) {
        result.errorMsg = "Input images are empty or not initialized.";
        return result;
    }

    try {
        double leftEdgePos = 0.0;
        double rightEdgePos = 0.0;

        // 2. 提取左右边缘
        // 假设背景暗，钢板亮：左相机视场中钢板在右侧（极性为从暗到亮 positive）
        bool okLeft = extractEdge(imgLeft, "positive", leftEdgePos);
        // 右相机视场中钢板在左侧（极性为从亮到暗 negative）
        bool okRight = extractEdge(imgRight, "negative", rightEdgePos);

        if (!okLeft || !okRight) {
            result.errorMsg = "Failed to extract continuous edges on one or both sides.";
            return result;
        }

        result.leftEdgeCol = leftEdgePos;
        result.rightEdgeCol = rightEdgePos;

        // 3. 计算物理宽度
        // 测宽公式取决于标定坐标系的建立方式。
        // 这里假设一种最常见的双盲区安装法：
        // 宽度 = 基线距离 + 左相机边缘相对零点的物理偏移 + 右相机边缘相对零点的物理偏移
        double physicalLeftOffset = leftEdgePos * m_pixelResLeft;
        double physicalRightOffset = rightEdgePos * m_pixelResRight;

        result.widthValue = m_cameraBaseDistance + physicalLeftOffset + physicalRightOffset;

        // 计算跑偏量（以产线中心为基准）
        result.centerOffset = (physicalLeftOffset - physicalRightOffset) / 2.0;

        result.isOk = true;
        result.errorMsg = "Success";



    } catch (HException& e) {
        // 捕获 Halcon 内部算子异常
        result.isOk = false;
        result.errorMsg = QString("Halcon Exception: %1").arg(e.ErrorMessage().Text());
        qWarning() << result.errorMsg;
    } catch (...) {
        result.isOk = false;
        result.errorMsg = "Unknown standard exception in algorithm.";
        qWarning() << result.errorMsg;
    }

    m_lastResult = result;
    return result;
}

bool DualLineScanWidthImgPro::extractEdge(const HObject& img, const HTuple& transition, double& edgeColPos) {
    HObject edges, selectedEdges, sortedEdges;
    HTuple rowBegin, colBegin, rowEnd, colEnd, nr, nc, dist;
    HTuple numEdges;

    try {
        // 1. 亚像素边缘提取 (使用Canny算子)
        // 参数可根据钢板红热(自发光)或冷轧(打背光/结构光)的信噪比调整
        EdgesSubPix(img, &edges, "canny", m_edgeAlpha, m_edgeThreshold, m_edgeThreshold + 20);

        // 2. 筛选符合极性特征且长度足够长的边缘
        // 对于线阵相机拼成的图，真正的边缘通常是一条贯穿图像上下的长直线
        SelectShapeXld(edges, &selectedEdges, "contlength", "and", 100.0, 999999.0);

        CountObj(selectedEdges, &numEdges);
        if (numEdges.I() == 0) {
            return false; // 未找到有效边缘
        }

        // 3. 拟合直线以获取高精度坐标
        // 选出最长的一根作为主边缘（如果有多条，可按长度排序取第一条）
        SortContoursXld(selectedEdges, &sortedEdges, "upper_left", "true", "row");
        HObject longestEdge;
        SelectObj(sortedEdges, &longestEdge, 1);

        // 使用 Tukey 权重进行鲁棒直线拟合，自动剔除飞溅的水汽或氧化铁皮造成的噪点
        FitLineContourXld(longestEdge, "tukey", -1, 0, 5, 2,
                          &rowBegin, &colBegin, &rowEnd, &colEnd, &nr, &nc, &dist);

        // 4. 返回边缘的平均列坐标 (X坐标)
        // 线阵测宽通常关注水平(Column)方向的位置
        edgeColPos = (colBegin.D() + colEnd.D()) / 2.0;

        return true;

    } catch (HException& e) {
        qWarning() << "extractEdge Exception:" << e.ErrorMessage().Text();
        return false;
    }
}
// ======================================================================
// 提取左相机的边缘 (左边画面：钢板在右，背景在左)
// ======================================================================
double DualLineScanWidthImgPro::findLeftEdgePixel(HalconCpp::HObject imgLeft)
{
    double edgeCol = 0.0;
    try {
        // 1. 获取图像尺寸
        HalconCpp::HTuple hv_Width, hv_Height;
        HalconCpp::GetImageSize(imgLeft, &hv_Width, &hv_Height);

        // 2. 生成一个 1D 测量句柄 (Measure Handle)
        HalconCpp::HTuple hv_Row = hv_Height / 2;       // 测量线的 Y 坐标 (画面正中间)
        HalconCpp::HTuple hv_Col = hv_Width / 2;        // 测量线的 X 中心
        HalconCpp::HTuple hv_Phi = 0.0;                 // 角度 0 度 (水平向右)
        HalconCpp::HTuple hv_Length1 = hv_Width / 2;    // 测量框的半长 (覆盖整个图宽)
        HalconCpp::HTuple hv_Length2 = 20;              // 测量框的半高 (20像素用来平均降噪)
        HalconCpp::HTuple hv_Interpolation = "nearest_neighbor";
        HalconCpp::HTuple hv_MeasureHandle;

        HalconCpp::GenMeasureRectangle2(hv_Row, hv_Col, hv_Phi, hv_Length1, hv_Length2,
                                        hv_Width, hv_Height, hv_Interpolation, &hv_MeasureHandle);

        // 3. 执行亚像素找边
        HalconCpp::HTuple hv_RowEdge, hv_ColEdge, hv_Amplitude, hv_Distance;
        // "negative" 表示由亮变暗 (左边亮光背景，向右碰到黑色的钢板)
        HalconCpp::MeasurePos(imgLeft, hv_MeasureHandle, 1.0, 30, "negative", "first",
                              &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);

        // 4. 释放句柄 (极度重要，防止内存泄漏)
        HalconCpp::CloseMeasure(hv_MeasureHandle);

        // 5. 提取坐标
        if (hv_ColEdge.Length() > 0) {
            edgeCol = hv_ColEdge[0].D();
        } else {
            edgeCol = -1.0;
        }
    } catch (HalconCpp::HException &except) {
        edgeCol = -1.0;
    }
    return edgeCol;
}

// ======================================================================
// 提取右相机的边缘 (右边画面：钢板在左，背景在右)
// ======================================================================
double DualLineScanWidthImgPro::findRightEdgePixel(HalconCpp::HObject imgRight)
{
    double edgeCol = 0.0;
    try {
        HalconCpp::HTuple hv_Width, hv_Height;
        HalconCpp::GetImageSize(imgRight, &hv_Width, &hv_Height);

        HalconCpp::HTuple hv_Row = hv_Height / 2;
        HalconCpp::HTuple hv_Col = hv_Width / 2;
        HalconCpp::HTuple hv_Phi = 0.0;
        HalconCpp::HTuple hv_Length1 = hv_Width / 2;
        HalconCpp::HTuple hv_Length2 = 20;
        HalconCpp::HTuple hv_Interpolation = "nearest_neighbor";
        HalconCpp::HTuple hv_MeasureHandle;

        HalconCpp::GenMeasureRectangle2(hv_Row, hv_Col, hv_Phi, hv_Length1, hv_Length2,
                                        hv_Width, hv_Height, hv_Interpolation, &hv_MeasureHandle);

        HalconCpp::HTuple hv_RowEdge, hv_ColEdge, hv_Amplitude, hv_Distance;
        // "positive" 表示由暗变亮 (左边是黑色的钢板，向右离开钢板变成亮光背景)
        HalconCpp::MeasurePos(imgRight, hv_MeasureHandle, 1.0, 30, "positive", "last",
                              &hv_RowEdge, &hv_ColEdge, &hv_Amplitude, &hv_Distance);

        HalconCpp::CloseMeasure(hv_MeasureHandle);

        if (hv_ColEdge.Length() > 0) {
            edgeCol = hv_ColEdge[hv_ColEdge.Length() - 1].D();
        } else {
            edgeCol = -1.0;
        }
    } catch (HalconCpp::HException &except) {
        edgeCol = -1.0;
    }
    return edgeCol;
}