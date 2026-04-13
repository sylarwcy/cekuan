#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QThread>
#include "WorkerSample.h"
#include "workertrigger.h"
#include "WorkerDatabase.h"
#include "WorkerMQTT.h"
#include "workStationDataStructure.h"
#include "WorkerCamera.h"
#include "WorkerImageProcess.h"
#include "WorkerPLC.h"

class Workstation : public QObject
{
  Q_OBJECT

public:
    explicit Workstation(QObject *parent = nullptr);
    ~Workstation();

    void StartTrigger();
    void SettingQThread();
    void Init(QString iniSessionName);
    int ReadSetting();  // 读取配置文件（从setting.ini加载参数）

    QString m_iniSessionName; //配置文件session名
    QString m_location;       //工位位置
    int m_serialNumber;       //工位序号
    WorkStation_DATA m_workstation_param; //工位参数

private:
    // ===== 业务工作类 =====
    WorkerCamera* m_pWorkerCamera;
    WorkerImageProcess* m_pWorkerImageProcess;
    WorkerPLC* m_pWorkerPlc;

    // ===== 物理线程 =====
    QThread* m_pThreadCamera;
    QThread* m_pThreadImageProcess;
    QThread* m_plcThread;

    // QThread m_thread_sample;
    // WorkerSample *p_worker_sample;

    QThread m_thread_trigger;
    WorkerTrigger *p_worker_trigger;

    // QThread m_thread_database;
    // WorkerDatabase *p_worker_database;

    // QThread m_thread_mqtt;
    // WorkerMQTT *p_worker_mqtt;


public slots:
    // 接收底层相机的日志
    void onLogReceived(QString msg);

    // 接收算法线程算完的结果
    void onWidthDataReady(const WidthResult& result);

    void onDisplayImage(const HalconCpp::HObject& dispImg);

    // 接收算法处理异常（如找不到边缘、Halcon算子报错等）
    void onProcessError(const QString& errorMsg);

signals:
    //界面刷新开始
    void start_loop_trigger();
    // 将底层发来的日志，继续向上转发给界面 (frmmain)
    void signalLogToUI(QString msg);

    // 将算法算出的宽度结果，向上转发给界面画曲线
    void signalMeasureResultToUI(const WidthResult& result);

    void sigSendDataToPLC(double width, double offset, int statusFlag);
};
#endif // CONTROLLER_H
