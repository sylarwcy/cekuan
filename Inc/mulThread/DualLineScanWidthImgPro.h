// ==================== [ DualLineScanWidthImgPro.h ] ====================
#pragma once
#include "ImageProcessor.h"
#include <QMap>
#include "workStationDataStructure.h"

/**
 * @brief 双线阵相机钢板测宽图像处理核心大脑
 * @details 负责双目图像的物理对齐、死区屏蔽、边缘特征游程编码提取、多项式反投影计算及标定状态机调度。
 */
class DualLineScanWidthImgPro {
public:
    DualLineScanWidthImgPro();
    virtual ~DualLineScanWidthImgPro();

    /**
     * @brief 算法引擎初始化（热启动）
     * @param masterDictPath 主相机(左)的1D-LUT标定字典绝对路径
     * @param slaveDictPath  副相机(右)的1D-LUT标定字典绝对路径
     * @param encoderResolution 编码器默认的单行毫米当量(兜底用)
     * @return true: 加载字典成功; false: 加载失败，启用默认防爆参数
     */
    bool initAlgorithm(const QString& masterDictPath, const QString& slaveDictPath, double encoderResolution);

    /**
     * @brief 图像帧处理主入口 (状态机枢纽)
     * @param imgLeft 主相机(左)原始输入图像
     * @param imgRight 副相机(右)原始输入图像
     * @param frameID 当前帧的自增流水号（用于监测千兆网口丢帧，触发时序断层熔断）
     * @return WidthResult 包含了物理宽度、UI绘图相对坐标、全景轮廓点云的综合数据包
     */
    WidthResult processFrame(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight, long long frameID = -1);

    /**
     * @brief 热更新单台相机的像素尺寸当量 (用于标定步骤1和2完毕后的动态注入)
     * @param isMaster true为更新主相机，false为更新副相机
     * @param newC 拟合出的新一阶多项式系数 (毫米/像素)
     */
    void updatePixelResolution(bool isMaster, double newC);

    /**
     * @brief 热更新双目相机的机械重叠轴距常数 (用于标定步骤3完毕后的动态注入)
     * @param newOffset 主副相机在物理世界中的X轴安装平移偏差 (mm)
     */
    void updateBaselineOffset(double newOffset);

    /**
     * @brief 持久化存盘：将内存中最新的相机系数矩阵和轴距覆盖写入本地 .hdict 文件
     */
    void saveCurrentDictsToDisk();

    /**
     * @brief 时序缓冲池复位器
     * @details 在“新钢板车头入画”或“发生网络丢帧”时调用，倒掉跨帧拼接的残留图像，防止车头白条和残影污染
     */
    void resetBuffers();

    /**
     * @brief 标定状态机网关
     * @param mode 0: 正常双目生产测宽
     * 1: 主相机单目像素当量标定
     * 2: 副相机单目像素当量标定
     * 3: 双目重叠区域轴距物理差值标定
     */
    void setCalibMode(int mode) { m_calibMode = mode; }

private:
    // =======================================================================
    // 内部核心解耦算子组件
    // =======================================================================

    /**
     * @brief [算子] 图像纵向无损插值对齐
     * @details 利用 ZoomImageSize 单行拉伸填补机制，无物理断层地对齐主副相机的俯仰角微小落差 (DiffY)
     */
    void alignImages(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight, int DiffY, int frameHeight, double wM, double wS, HalconCpp::HObject& imgLeftAligned, HalconCpp::HObject& imgRightAligned);

    /**
     * @brief [算子] 游程编码(RunLength)纯净边缘提取器
     * @details 废弃形态学平滑，直接扫描二值化后的白色像素，精准抓取单行最左(outMinX)和最右(outMaxX)绝对坐标，精度达1像素。
     */
    void extractEdges(const HalconCpp::HObject& img, int frameHeight, QVector<double>& outMinX, QVector<double>& outMaxX);

    // =======================================================================
    // 四大状态机模式流水线
    // =======================================================================
    void processMode1(const HalconCpp::HObject& imgLeftAligned, int frameHeight, int mStartX, int mWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode2(const HalconCpp::HObject& imgRightAligned, int frameHeight, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode3(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, WidthResult& result, QVector<double>& vecWidths);
    void processMode0(const HalconCpp::HObject& imgLeftAligned, const HalconCpp::HObject& imgRightAligned, int frameHeight, int mStartX, int mWidth, int sSafeStartX, int sSafeWidth, int sSafeEndX, double wM, double wS, WidthResult& result, QVector<double>& vecWidths);

private:
    bool m_isInitialized{false};          // 算法引擎启动就绪标志
    double m_encoder_mm_per_row{0.09473}; // 硬件编码器线速度当量基准

    HalconCpp::HObject m_hMasterBuffer;   // 主相机时序帧补齐缓冲池
    HalconCpp::HObject m_hSlaveBuffer;    // 副相机时序帧补齐缓冲池
    long long m_lastFrameID{-1};          // 上一帧处理流水号 (用于监测丢帧)

    int m_lastCropS_X{1000};              // (已废弃/备用)旧版截取记录
    int m_lastCropE_X{2000};              // (已废弃/备用)旧版截取记录

    // =======================================================================
    // 光学系统多项式反投影系数矩阵 (世界坐标系转换库)
    // 公式: X_mm = a*u^3 + b*u^2 + c*u + d
    // =======================================================================
    HalconCpp::HTuple a_m, b_m, c_m, d_m; // 主相机(左)多项式系数
    HalconCpp::HTuple a_s, b_s, c_s, d_s; // 副相机(右)多项式系数

    double m_cameraBaselineOffsetMM{245.0}; // 主副相机在物理世界X轴的相对平移安装差 (重叠轴距)

    int m_calibMode{0}; // 引擎当前运行模式
};