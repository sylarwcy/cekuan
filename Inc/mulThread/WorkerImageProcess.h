#pragma once
#include <QObject>
#include <QMutex>
#include "ImgProGlobal.h"
#include "WorkerCamera.h"
#include "workStationDataStructure.h"

class WorkerImageProcess : public QObject {
    Q_OBJECT

public:
    explicit WorkerImageProcess(QObject *parent = nullptr);

    ~WorkerImageProcess();

    void init(const WorkStation_DATA &paramData, double mmPerPixel);
    // void init(const WorkStation_DATA &paramData);

    void setAlgorithm(DualLineScanWidthImgPro *algo) { m_algo = algo; }

public slots:
    // 接收双线阵相机的同步图像
    void imgProcessMeasure(const DualCameraChunk &chunk);
    void slot_reloadCalibration();

signals:
    // 处理完成，向 UI 或数据库发送结果
    void sigMeasureReady(const WidthResult& result);

    // 向 UI 发送处理后的图像用于显示 (例如在 HWindowControl 中显示)
    void displayImageReady(const HalconCpp::HObject &dispImg);

    void sigProcessError(QString);

    // // 已有的测量结果信号
    // void sigMeasureReady(WidthResult res);

    // // 【新增】将拼接好的大图发给主界面显示的信号
    // void sigDisplayImageReady(HalconCpp::HObject imgFull);
private:
    DualLineScanWidthImgPro *m_algo;
    bool m_isProcessing{false};
    WorkStation_DATA m_paramData; // 🌟 新增：备份初始化工位参数，用于重载

private:
    bool m_isPlateActive = false;
    double m_sumWidth = 0.0;
    int m_validFrameCount = 0;
    int m_totalRows = 0;
    double m_mm_per_row = 1.0;

    // --- 新增：用于记录最大和最小宽度 ---
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0; // 初始给一个极大的值，方便找最小值

signals:
    // 已有的单帧实时数据信号
    // void sigMeasureReady(WidthResult res);

    // --- 新增：整块钢板走完后，发送汇总数据的信号 ---
    // 按顺序：平均宽度，总长度，最大宽度，最小宽度
    void sigPlateFinished(double avgWidth, double totalLength, double maxWidth, double minWidth);
};
