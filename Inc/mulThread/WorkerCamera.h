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

public slots:
    void onUpdateSpeedFromPLC(double speed_m_s);

    // [新增] 处理掉线与重连的槽函数
    void onCameraDisconnected(int camIndex);

    void onTryReconnect();

signals:
    void signalDualChunkReady(const DualCameraChunk &chunk);

    void signalCameraLog(QString msg);

    void signalExceptionFired(int camIndex);// [新增] 底层异常抛出信号

    // [新增] 通知 UI 相机已经准备好，可以按这个尺寸初始化控件了
    void signalCameraReady(int width, int height);

public:
    static void __stdcall ImageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser);

    // [新增] 海康设备异常回调函数
    static void __stdcall ExceptionCallBack(unsigned int nMsgType, void *pUser);

    void processFrame(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, int camIndex);

private:
    void configureMasterSlave();

    void *m_hDevLeft;
    void *m_hDevRight;

    CamContext m_ctxLeft;
    CamContext m_ctxRight;

    QString m_leftSN;
    QString m_rightSN;
    int m_imgWidth;
    int m_imgHeight;
    float m_lineRate = 500.0f;

    QMutex m_mutex;
    std::map<uint64_t, DualCameraChunk> m_bufferMap;

    // [新增] 状态与重连管理
    bool m_bCameraLeftOnline;
    bool m_bCameraRightOnline;
    QTimer *m_pReconnectTimer;
};
