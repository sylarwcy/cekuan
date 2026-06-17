// ==================== [ WorkerImageProcess.h ] ====================
#pragma once
#include <QObject>
#include <QMutex>
#include "ImgProGlobal.h"
#include "WorkerCamera.h"
#include "workStationDataStructure.h"
#include "DualLineScanWidthImgPro.h"

class WorkerImageProcess : public QObject {
    Q_OBJECT

public:
    explicit WorkerImageProcess(QObject *parent = nullptr);
    ~WorkerImageProcess();

    void init(const WorkStation_DATA &paramData, double mmPerPixel);
    void setAlgorithm(DualLineScanWidthImgPro *algo) { m_algo = algo; }

public slots:
    // 接收双线阵相机的同步图像
    void imgProcessMeasure(const DualCameraChunk &chunk);
    void slot_reloadCalibration();

    // 🌟 新增：由弹窗下发的状态机切换指令
    void slot_setCalibMode(int mode);
    void slot_hotUpdateMasterPixel(double newC);
    void slot_hotUpdateSlavePixel(double newC);
    void slot_hotUpdateBaseline(double newOffset);

    signals:
        // 处理完成，向 UI 或数据库发送结果
        void sigMeasureReady(const WidthResult& result);
    // 向 UI 发送处理后的图像用于显示
    void displayImageReady(const HalconCpp::HObject &dispImg);
    void sigProcessError(QString);
    // 整块钢板走完后，发送汇总数据的信号
    void sigPlateFinished(double avgWidth, double totalLength, double maxWidth, double minWidth);

    // 🌟 新增信号：在独立的图像子线程中将最终算好的双目重叠基线间距安全发射给 UI 弹窗
    void sig_baselineCalibrated(double value);

private:
    DualLineScanWidthImgPro *m_algo;
    bool m_isProcessing{false};
    WorkStation_DATA m_paramData;

private:
    bool m_isPlateActive = false;
    double m_sumWidth = 0.0;
    int m_validFrameCount = 0;
    int m_totalRows = 0;
    double m_mm_per_row = 1.0;

    // --- 新增：用于记录最大和最小宽度 ---
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0;
};