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
    void onSyncedImagesReady(const HalconCpp::HObject &imgLeft, const HalconCpp::HObject &imgRight);

signals:
    // 处理完成，向 UI 或数据库发送结果
    void resultReady(double width, bool isOk);

    // 向 UI 发送处理后的图像用于显示 (例如在 HWindowControl 中显示)
    void displayImageReady(const HalconCpp::HObject &dispImg);

private:
    DualLineScanWidthImgPro *m_algo;
};
