#pragma once
#include <QObject>
#include <QMutex>
#include <QTimer>
#include <map>
#include "MvCameraControl.h"
#include "workStationDataStructure.h"

struct CamContext {
    class WorkerCamera* pThis;
    int camIndex; // 0为左(Master)，1为右(Slave)
};

class WorkerCamera : public QObject
{
    Q_OBJECT
public:
    explicit WorkerCamera(QObject *parent = nullptr);
    ~WorkerCamera();

    bool initCameras(const QString& leftSN, const QString& rightSN, const int height);
    void startGrabbing();
    void stopGrabbing();

public slots:
    void onUpdateSpeedFromPLC(double speed_m_s);

    // [新增] 处理掉线与重连的槽函数
    void onCameraDisconnected(int camIndex);
    void onTryReconnect();

    signals:
        void signalDualChunkReady(const DualCameraChunk& chunk);
    void signalCameraLog(QString msg);

    // [新增] 底层异常抛出信号
    void signalExceptionFired(int camIndex);

public:
    static void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

    // [新增] 海康设备异常回调函数
    static void __stdcall ExceptionCallBack(unsigned int nMsgType, void* pUser);

    void processFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, int camIndex);

private:
    void configureMasterSlave();

    void* m_hDevLeft;
    void* m_hDevRight;

    CamContext m_ctxLeft;
    CamContext m_ctxRight;

    QString m_leftSN;
    QString m_rightSN;
    int m_cam_img_height;

    QMutex m_mutex;
    std::map<uint64_t, DualCameraChunk> m_bufferMap;

    // [新增] 状态与重连管理
    bool m_bCameraLeftOnline;
    bool m_bCameraRightOnline;
    QTimer* m_pReconnectTimer;
};