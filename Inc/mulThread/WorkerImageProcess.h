// ==================== [ WorkerImageProcess.h ] ====================
#pragma once
#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QStringList>
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
    void imgProcessMeasure(const DualCameraChunk &chunk);
    void slot_reloadCalibration();

    void slot_setCalibMode(int mode);
    void slot_hotUpdateMasterPixel(double newC);
    void slot_hotUpdateSlavePixel(double newC);
    void slot_hotUpdateBaseline(double newOffset);

    // 🌟 接收前端传来的实时动态钢板厚度
    void slot_updateCurrentThickness(double t);
    // 🌟 接收标定界面传来的 K 系数
    void slot_updateThicknessK(double k, double baseT);

    void startOfflineTest();

private slots:
    void onOfflineTimerTimeout();

signals:
    void sigMeasureReady(const WidthResult& result);
    void displayImageReady(const HalconCpp::HObject &dispImg);
    void sigProcessError(QString);
    void sigPlateFinished(double avgWidth, double totalLength, double maxWidth, double minWidth);
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
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0;

    // 🌟 厚度补偿物理引擎组件
    double m_currentThickness{2.0};
    double m_thicknessK{0.0};
    double m_baseThickness{2.0};

    // 离线播放组件
    bool m_bIsOfflineMode{false};
    int m_offlineIntervalMs{1500};
    QTimer* m_pOfflineTimer{nullptr};
    QStringList m_offlineFiles;
    QString m_offlineDir;
    int m_offlineIndex{0};
};