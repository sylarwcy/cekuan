#include "DualLineScanWidthImgPro.h"
#include <QDebug>

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