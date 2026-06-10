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

    // void SetHandle(HalconCpp::HTuple &ori, HalconCpp::HTuple &pro);

    void startGrabbing();

    void stopGrabbing();

    bool HtupleIsEmpty(HalconCpp::HTuple &value);

public slots:
    void onUpdateSpeedFromPLC(double speed_m_s);

    // [新增] 处理掉线与重连的槽函数
    void onCameraDisconnected(int camIndex);

    void onTryReconnect();

signals:
    void sigImageReadyToAlg(const DualCameraChunk &chunk);
    void sigDisplayRawImage(const DualCameraChunk &chunk);

    void signalCameraLog(QString msg);

    void signalExceptionFired(int camIndex);// [新增] 底层异常抛出信号

    // [新增] 通知 UI 相机已经准备好，可以按这个尺寸初始化控件了
    void sigImageReadyTOUI(int width, int height);

    // 发送双相机同步采到的图像
    // void sigSyncedImagesReady(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight);

public:
    static void __stdcall ImageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser);

    // [新增] 海康设备异常回调函数
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
    int m_imgHeight=100;
    float m_lineRate = 380.0f;

    // HalconCpp::HTuple m_winHandle_ori, m_winHandle_pro;

    QMutex m_mutex;
    std::map<uint64_t, DualCameraChunk> m_bufferMap;

    // [新增] 状态与重连管理
    bool m_bCameraLeftOnline;
    bool m_bCameraRightOnline;
    QTimer *m_pReconnectTimer;

    // 依据算法字典刚性对齐，确定为 0.09473 mm
    double m_mmPerPixelX = 0.09473;
};
