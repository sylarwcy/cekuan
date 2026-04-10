#include "MyApplication.h"
#include <stdio.h>
#include <QDebug>  // Qt调试工具
#include <QDir>

// 构造函数实现
MyApplication::MyApplication(int argc, char *argv[]): QApplication(argc, argv) {

    // 0.初始化日志
    initLogger();

    // 1.实例化全局共享数据的对象，并加载setting配置文件数据
    pNodeData = new NodeData();
    pNodeData->ReadSetting();//读配置文件

    // 2.初始化工位线程
    for (int i = 0; i < pNodeData->setting.work_station_num; ++i) {
        Workstation* station = new Workstation(NULL);
        station->Init(QString("workstation%1").arg(i+1));
        m_workstationList.append(station);
    }

    // 3.初始化共享内存管理类
    m_mem_manager = new SharedMemoryManager(m_workstationList);
    m_mem_manager->init(pNodeData->setting.from_plc_share_mem_name, pNodeData->setting.to_plc_share_mem_name);

    // //初始化MQTT相关数据结构体（从设备到MQTT方向），将内存清零
    // memset(&m_mqtt_l2_data, 0, sizeof(m_mqtt_l2_data));
    // // 初始化MQTT相关数据结构体（从MQTT到设备方向），将内存清零
    // memset(&m_l2_mqtt_data, 0, sizeof(m_l2_mqtt_data));
    // // 初始化图像数据结构体，将内存清零
    // memset(&m_img_data, 0, sizeof(m_img_data));

    isStopTrigger = false; // 初始化触发停止标志位为false（表示未停止触发）
    isTestImage = false; // 初始化测试图像标志位为false（表示未启用图像测试）
    isTestVideo = false; // 初始化测试视频标志位为false（表示未启用视频测试）
}

// 析构函数实现
MyApplication::~MyApplication() {
    // 释放 Workstation 对象（如果拥有所有权）
    qDeleteAll(m_workstationList);
    m_workstationList.clear();

    // 释放其他资源
    delete m_mem_manager;
    delete pNodeData;
}

void MyApplication::initLogger() {
    // 1. init the logging mechanism
    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    //设置log位置
    const QString sLogPath(QDir(QCoreApplication::applicationDirPath()+"/log").filePath("rot_img_log.txt"));

    // 2. add two destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
        sLogPath, EnableLogRotation, MaxSizeBytes(1024*1024*500), MaxOldLogCount(10)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    //DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&logFunction));

    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
    //logger.addDestination(functorDestination);

    // 3. start logging
    QLOG_INFO() << "OCR PlateNumber program started";
    QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
}