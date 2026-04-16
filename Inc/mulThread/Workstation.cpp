// Controller.cpp
#include "Workstation.h"
#include <QThread>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <utility>

#include "MyApplication.h"
#include "frmmain.h"
#include "ini/settings.h"

Workstation::Workstation(QObject *parent) : QObject(parent),
    m_pWorkerCamera(nullptr),
    m_pWorkerImageProcess(nullptr),
    m_pThreadCamera(nullptr),
    m_pThreadImageProcess(nullptr)
{
}

Workstation::~Workstation() {
    // ====== 安全退出机制 (工业级防死锁) ======

    if (m_pWorkerCamera) {
        m_pWorkerCamera->stopGrabbing(); // 先让硬件停下来
    }

    if (m_pThreadCamera) {
        m_pThreadCamera->quit();
        m_pThreadCamera->wait(2000); // 最多等 2 秒
    }

    if (m_pThreadImageProcess) {
        m_pThreadImageProcess->quit();
        m_pThreadImageProcess->wait(2000);
    }

    // 打扫战场
    if (m_pWorkerCamera)       { delete m_pWorkerCamera; }
    if (m_pWorkerImageProcess) { delete m_pWorkerImageProcess; }
    if (m_pThreadCamera)       { delete m_pThreadCamera; }
    if (m_pThreadImageProcess)    { delete m_pThreadImageProcess; }

    // //触发进程停止
    // pApp->isStopTrigger = true;
    // m_thread_trigger.quit();
    // m_thread_trigger.wait();
    //
    // m_thread_sample.quit();
    // m_thread_sample.wait();
    //
    // m_thread_database.quit();
    // m_thread_database.wait();
    //
    // Sleep(100);
}

void Workstation::Init(QString iniSessionName) {
    m_iniSessionName = std::move(iniSessionName);
    ReadSetting();
    SettingQThread();
}

void Workstation::SettingQThread() {
    //Sample线程
    // p_worker_sample = new WorkerSample();
    // p_worker_sample->moveToThread(&m_thread_sample);
    // connect(&m_thread_sample, &QThread::finished, p_worker_sample, &QObject::deleteLater);

    //trigger界面刷新线程，用触发器老自动停止
    p_worker_trigger = new WorkerTrigger();
    p_worker_trigger->init(m_workstation_param);
    p_worker_trigger->moveToThread(&m_thread_trigger);
    connect(&m_thread_trigger, &QThread::finished, p_worker_trigger, &QObject::deleteLater);

    //database线程
    // p_worker_database = new WorkerDatabase();
    // p_worker_database->init(m_workstation_param);
    // p_worker_database->moveToThread(&m_thread_database);
    //绑定信号：sig_connect_db() ⬅➡ p_worker_database.connectDB()
    // connect(&m_thread_database, &QThread::started, p_worker_database, &WorkerDatabase::connectDB);
    // connect(&m_thread_database, &QThread::finished, p_worker_database, &QObject::deleteLater);

    //mqtt线程
    // p_worker_mqtt = new WorkerMQTT();
    // p_worker_mqtt->init(m_workstation_param);
    // p_worker_mqtt->moveToThread(&m_thread_mqtt);
    // connect(&m_thread_mqtt, &QThread::started, p_worker_mqtt, &WorkerMQTT::startConnectMqtt);
    // connect(&m_thread_mqtt, &QThread::finished, p_worker_mqtt, &QObject::deleteLater);

    // ==============================================================
    // 1. 注册自定义结构体 (极其重要，否则跨线程传参会直接导致程序崩溃)
    // ==============================================================
    qRegisterMetaType<DualCameraChunk>("DualCameraChunk");
    qRegisterMetaType<WidthResult>("MeasureResult");

    // ==============================================================
    // 2. 实例化业务逻辑类
    // ==============================================================
    m_pWorkerCamera = new WorkerCamera();
    m_pWorkerImageProcess = new WorkerImageProcess();
    m_pWorkerPlc = new WorkerPLC();

    // ==============================================================
    // 3. 实例化 Qt 物理线程
    // ==============================================================
    m_pThreadCamera = new QThread(this);
    m_pThreadImageProcess = new QThread(this);
    m_plcThread = new QThread(this);

    // ==============================================================
    // 4. 将业务类移入物理线程 (这就是 Qt 最优雅的 moveToThread 模型)
    // ==============================================================
    m_pWorkerCamera->moveToThread(m_pThreadCamera);
    m_pWorkerImageProcess->moveToThread(m_pThreadImageProcess);
    m_pWorkerPlc->moveToThread(m_plcThread);

    // ==============================================================
    // 5. 绑定跨线程通信的“神经总线” (强制使用 QueuedConnection 异步队列)
    // ==============================================================

    // [神经A]: 海康底层拼好图 -> 扔给 Halcon 算法大脑
    connect(m_pWorkerCamera, &WorkerCamera::sigImageReadyToAlg,
            m_pWorkerImageProcess, &WorkerImageProcess::imgProcessMeasure,
            Qt::QueuedConnection);

    connect(m_pWorkerCamera, &WorkerCamera::sigDisplayRawImage,
            this, &Workstation::onDisplayImage,
            Qt::QueuedConnection);

    // [神经B]: Halcon 算法算完宽度 -> 扔给主界面 UI 进行曲线渲染和存库
    connect(m_pWorkerImageProcess, &WorkerImageProcess::sigMeasureReady,
            this, &Workstation::onProcessResult,
            Qt::QueuedConnection);

    // [神经C]: 收集底层日志 -> 扔给主界面显示
    connect(m_pWorkerCamera, &WorkerCamera::signalCameraLog,
            this, &Workstation::onLogReceived,
            Qt::QueuedConnection);

    connect(m_pThreadCamera, &QThread::finished, m_pWorkerCamera, &QObject::deleteLater);
    connect(m_pThreadImageProcess, &QThread::finished, m_pWorkerImageProcess, &QObject::deleteLater);
    connect(m_plcThread, &QThread::finished, m_pWorkerPlc, &QObject::deleteLater);
    // ==============================================================
    // 6. 启动物理线程 (开启它们内部的事件循环 EventLoop)
    // ==============================================================
    m_pThreadCamera->start();
    m_pThreadImageProcess->start();

    if (m_pWorkerCamera->initCameras(m_workstation_param.master_sn,m_workstation_param.slave_sn,m_workstation_param.camImgWidth,m_workstation_param.camImgHeight))
        m_pWorkerCamera->startGrabbing();
    else {
        onLogReceived("初始化严重故障：请检查相机网络和配置文件中的 SN 号！");
    }

    //工位类的触发信号绑定trigger的工作函数
    connect(this, &Workstation::start_loop_trigger, p_worker_trigger, &WorkerTrigger::on_doSomething);

    //启动线程
    // m_thread_sample.start();
    m_thread_trigger.start();
    // m_thread_database.start();
    // m_thread_mqtt.start();
}

void Workstation::StartTrigger() {
    emit start_loop_trigger();
}

// 读取配置文件（从setting.ini加载参数到setting结构体）
int Workstation::ReadSetting() {
    // 创建INI配置文件处理对象（指定配置文件为setting.ini）
    IniSettings setting_ini("setting.ini");

    // 0.工位信息
    m_workstation_param.m_serialNumber = setting_ini.getValue(m_iniSessionName, "serialNumber").toInt();
    m_serialNumber = m_workstation_param.m_serialNumber;
    m_workstation_param.m_location = setting_ini.getValue(m_iniSessionName, "location");
    m_location = m_workstation_param.m_location;
    m_workstation_param.m_loop_refresh_ms = setting_ini.getValue(m_iniSessionName, "loop_refresh_ms").toInt();

    // 1.读取相机配置
    m_workstation_param.master_ip = setting_ini.getValue(m_iniSessionName, "Master_ip_addr"); // 相机ip
    m_workstation_param.master_sn = setting_ini.getValue(m_iniSessionName, "Master_sn"); // 相机sn
    m_workstation_param.slave_ip = setting_ini.getValue(m_iniSessionName, "front_ip_addr"); // 相机ip
    m_workstation_param.slave_sn = setting_ini.getValue(m_iniSessionName, "Slave_sn"); // 相机sn
    m_workstation_param.camImgWidth = setting_ini.getValue(m_iniSessionName, "front_camera_res_width").toInt(); // 图像高
    m_workstation_param.camImgHeight = setting_ini.getValue(m_iniSessionName, "front_camera_res_height").toInt(); // 图像高
    m_workstation_param.test_img_path = setting_ini.getValue(m_iniSessionName, "test_img_filename"); // 图像高

    // 2.图像处理配置
    // m_workstation_param.device = setting_ini.getValue(m_iniSessionName, "device");
    m_workstation_param.run_mode = setting_ini.getValue(m_iniSessionName, "mode").toInt();
    m_workstation_param.ocrBufferPath = setting_ini.getValue(m_iniSessionName, "ocrBufferPath");
    m_workstation_param.detBufferPath = setting_ini.getValue(m_iniSessionName, "detBufferPath");
    m_workstation_param.remotePathPrefix = setting_ini.getValue(m_iniSessionName, "remotePath");

    m_workstation_param.roi_region = setting_ini.getValue(m_iniSessionName, "roi_region");
    m_workstation_param.det_model_path = setting_ini.getValue(m_iniSessionName, "det_model");
    m_workstation_param.det_param_path = setting_ini.getValue(m_iniSessionName, "det_param");
    m_workstation_param.ocr_det_1_path = setting_ini.getValue(m_iniSessionName, "ocr_det_1");
    m_workstation_param.ocr_det_2_path = setting_ini.getValue(m_iniSessionName, "ocr_det_2");
    m_workstation_param.ocr_recog_2_path = setting_ini.getValue(m_iniSessionName, "ocr_recog_2");
    m_workstation_param.affine_x_pre = setting_ini.getValue(m_iniSessionName, "affine_x_pre");
    m_workstation_param.affine_y_pre = setting_ini.getValue(m_iniSessionName, "affine_y_pre");
    m_workstation_param.affine_x_post = setting_ini.getValue(m_iniSessionName, "affine_x_post");
    m_workstation_param.affine_y_post = setting_ini.getValue(m_iniSessionName, "affine_y_post");
    m_workstation_param.max_age = setting_ini.getValue(m_iniSessionName, "max_age");
    m_workstation_param.min_hits = setting_ini.getValue(m_iniSessionName, "min_hits");
    m_workstation_param.iouThreshold = setting_ini.getValue(m_iniSessionName, "iouThreshold");

    // 3.读取数据库配置
    // m_workstation_param.dbType = setting_ini.getValue(m_iniSessionName, "DbType"); // 数据库类型
    // m_workstation_param.dbConnName = setting_ini.getValue(m_iniSessionName, "dbConnName"); // 连接名称
    // m_workstation_param.dbName = setting_ini.getValue(m_iniSessionName, "DbName"); // 数据库名
    // m_workstation_param.dbHostName = setting_ini.getValue(m_iniSessionName, "HostName"); // 主机地址
    // m_workstation_param.dbHostPort = setting_ini.getValue(m_iniSessionName, "HostPort").toInt(); // 通信端口
    // m_workstation_param.dbUserName = setting_ini.getValue(m_iniSessionName, "UserName"); // 用户名称
    // m_workstation_param.dbUserPwd = setting_ini.getValue(m_iniSessionName, "UserPwd"); // 用户密码
    // m_workstation_param.dbTableName = setting_ini.getValue(m_iniSessionName, "TableName"); // 数据库表名
    // m_workstation_param.dbKeyName = setting_ini.getValue(m_iniSessionName, "KeyName"); // 数据库表的字段名

    // 4.读取MQTT配置
    // m_workstation_param.mqttUse = setting_ini.getValue(m_iniSessionName, "mqtt_use").toInt(); // 是否启用MQTT
    // m_workstation_param.mqttTriggerTime = setting_ini.getValue(m_iniSessionName, "mqtt_trigger_time").toInt(); // 触发时间
    // m_workstation_param.mqttHostname = setting_ini.getValue(m_iniSessionName, "mqtt_hostname"); // 服务器主机名
    // m_workstation_param.mqttPort = setting_ini.getValue(m_iniSessionName, "mqtt_port").toInt(); // 服务器端口
    // m_workstation_param.mqttSaveImagePath = setting_ini.getValue(m_iniSessionName, "mqtt_save_image_path"); // 图像保存路径
    // m_workstation_param.mqttImageCycleNum = setting_ini.getValue(m_iniSessionName, "mqtt_image_cycle_num").toInt();
    // 图像循环数量
    // m_workstation_param.mqttPublicMsg = setting_ini.getValue(m_iniSessionName, "mqtt_public_msg"); // 收识别错误结果的主题

    return 0; // 读取成功返回0
}

void Workstation::onLogReceived(QString msg)
{
    // 这里可以统一加上时间戳，或者直接发射给 UI
    qDebug() << "[中枢日志]" << msg;
    emit signalLogToUI(msg);
}

// -------------------------------------------------------------------------
// 槽函数实现：测宽结果接收
// -------------------------------------------------------------------------
void Workstation::onProcessResult(const WidthResult& result)
{
    // 1. 格式化数据 (保留两位小数)
    QString strWidth = QString::number(result.widthValue, 'f', 2);
    QString strOffset = QString::number(result.centerOffset, 'f', 2);

    // 2. 更新 UI (注意：由于 Workstation 在主线程，直接操作 UI 是安全的)
    // ui->lcdNumber_Width->display(strWidth);
    // ui->lcdNumber_Offset->display(strOffset);

    // 4. 数据记录与通讯
    // 将数据压入图表缓存用于绘制曲线
    emit sigSendDataToUI(result);

    // 发送给数据库线程保存 (不要在这里直接写 SQL 语句，避免阻塞主线程)
    // emit sigSaveToDatabase(QDateTime::currentDateTime(), widthValue, centerOffset);

    // 下发给 PLC 或 二级系统 (L2)
    // emit sigSendDataToPLC(widthValue, centerOffset, currentStatus);
}

void Workstation::onProcessError(const QString& errorMsg) {
    // 1. 打印后台日志
    qWarning() << "[中枢报警] 算法处理异常:" << errorMsg;

    // 2. 发送给界面显示
    onLogReceived("算法异常: " + errorMsg);

    // 3. (可选) 如果你已经接好了 PLC，这里可以给 PLC 发送故障状态码
    // emit sigSendDataToPLC(0.0, 0.0, 4); // 假设 4 代表算法/相机故障
}

// 主控接收到图像后，调用 UI 界面的 Halcon 窗口刷新
void Workstation::onDisplayImage(const DualCameraChunk &chunk) {
    // 这里可以加一个简单的 UI 刷新频率控制 (例如每秒最多刷新 10 次，防止人眼看不清且浪费性能)
    static QElapsedTimer timer;
    if (!timer.isValid()) timer.start();

    if (timer.elapsed() > 100) { // 限制 10Hz 刷新率
        emit sigForwardToView(chunk);
        timer.restart();
    }
}