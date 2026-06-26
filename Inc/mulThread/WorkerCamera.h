#pragma once
#include <QObject>
#include <QMutex>
#include <QTimer>
#include <map>
#include "MvCameraControl.h"
#include "workStationDataStructure.h"

struct CamContext {
    class WorkerCamera *pThis;
    int camIndex; // 0为左(Master)，1为右(Slave)
};

class WorkerCamera : public QObject {
    Q_OBJECT

public:
    explicit WorkerCamera(QObject *parent = nullptr);
    ~WorkerCamera();
    bool initCameras(const QString& leftSN, const QString& rightSN,  const int imgWidth, const int imgHeight);
    void startGrabbing();
    void stopGrabbing();
    bool HtupleIsEmpty(HalconCpp::HTuple &value);

public slots:
    void onUpdateSpeedFromPLC(double speed_m_s);

    // 🌟 新增：响应 UI 传来的热更新曝光时间
    void onUpdateExposureTime(int expTime_us);

    void onCameraDisconnected(int camIndex);
    void onTryReconnect();

signals:
    void sigImageReadyToAlg(const DualCameraChunk &chunk);
    void sigDisplayRawImage(const DualCameraChunk &chunk);
    void signalCameraLog(QString msg);
    void signalExceptionFired(int camIndex);
    void sigImageReadyTOUI(int width, int height);

public:
    static void __stdcall ImageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser);
    static void __stdcall ExceptionCallBack(unsigned int nMsgType, void *pUser);
    void processFrame(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, int camIndex);
    void configureMasterSlave();

    void *m_hDevLeft;
    void *m_hDevRight;

    QList<void*> m_camList;
    int m_thread_cycle_ms=200;

    CamContext m_ctxLeft;
    CamContext m_ctxRight;

    QString m_leftSN;
    QString m_rightSN;
    int m_imgWidth=4096;
    int m_imgHeight=300;

    // 🌟 新增：独立存放曝光时间，用作上限保护计算的基准
    int m_exposureTime_us = 5000;
    float m_lineRate = 190.0f;

    QMutex m_mutex;
    std::map<uint64_t, DualCameraChunk> m_bufferMap;

    bool m_bCameraLeftOnline;
    bool m_bCameraRightOnline;
    QTimer *m_pReconnectTimer;

    double m_mmPerPixelX = 0.09473;
    double m_currentSpeed_mm_s = 100.0;
};