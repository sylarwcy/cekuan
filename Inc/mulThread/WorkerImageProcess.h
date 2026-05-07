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

    void init(const WorkStation_DATA &paramData);

    void setAlgorithm(DualLineScanWidthImgPro *algo) { m_algo = algo; }

public slots:
    // 接收双线阵相机的同步图像
    void imgProcessMeasure(const DualCameraChunk &chunk);

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
};
