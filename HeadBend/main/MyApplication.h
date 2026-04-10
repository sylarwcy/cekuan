#ifndef _MYAPPLICATION_
#define _MYAPPLICATION_

// 引入Qt应用程序基类
#include <QApplication>
// 引入智能指针库
#include <memory>

// 引入数据库连接线程类
#include "dbconnthread.h"
// 引入Oracle数据处理类
#include "orcldata.h"

// 引入全局数据定义
#include "GlobalData.h"
// 引入GigE相机处理类
#include "myCameraGigE.h"
// 引入共享内存处理类
#include "SharedMemoryManager.h"

// 引入控制器类
#include "Workstation.h"
/////////////////////////////////////////////////
// 日志相关头文件
#include "qlog_inc.h"

// Halcon机器视觉库头文件
#include "halcon_inc.h"
/////////////////////////////////////////////////

// 自定义应用程序类，继承自QApplication
// 用于管理整个应用程序的全局资源和状态
class MyApplication : public QApplication
{
public:
    // 构造函数：接收命令行参数
    MyApplication(int argc, char * argv[]);
    // 析构函数
    ~MyApplication();

    void initLogger();

public:
    // 图像数据结构体，存储图像相关数据
    struct IMG_DATA m_img_data;

    // 节点数据指针，存储节点相关配置和状态
    NodeData *pNodeData;

    // 前置相机指针
    MyCameraGigE *p_camera_front;
    // 后置相机指针
    MyCameraGigE *p_camera_back;

    // 前置相机原始图像显示窗口句柄
    HTuple winHandle_front_ori;
    // 前置相机处理后图像显示窗口句柄
    HTuple winHandle_front_pro;

    // 后置相机原始图像显示窗口句柄
    HTuple winHandle_back_ori;
    // 后置相机处理后图像显示窗口句柄
    HTuple winHandle_back_pro;

    // // MQTT到L2的数据结构体
    // struct MQTT_L2_DATA m_mqtt_l2_data;
    // // L2到MQTT的数据结构体
    // struct L2_MQTT_DATA m_l2_mqtt_data;

    // 触发停止标志位：true表示停止触发，false表示正常触发
    bool isStopTrigger;

    // 图像测试模式标志位：true表示启用图像测试
    bool isTestImage;
    // 视频测试模式标志位：true表示启用视频测试
    bool isTestVideo;

    // 前置相机标定参数
    HTuple f_hv_x0,f_hv_y0,f_hv_x1,f_hv_y1;
    // 后置相机标定参数
    HTuple b_hv_x0,b_hv_y0,b_hv_x1,b_hv_y1;

    // 前置相机视频测试句柄
    HTuple  hv_AcqHandle_front;
    // 前置相机视频测试的起始帧和结束帧
    int frame_start_front,frame_end_front;

    // 后置相机视频测试句柄
    HTuple  hv_AcqHandle_back;
    // 后置相机视频测试的起始帧和结束帧
    int frame_start_back,frame_end_back;

    // 本地数据库连接线程指针
    DbConnThread *dbConn_local;

    //----------------------------------------------------------------------------
    // 共享内存管理器
    SharedMemoryManager *m_mem_manager;
    // 工作点位线程指针，用于管理业务逻辑控制
    QList<Workstation*> m_workstationList;
    //相机拿数据的锁
    CRITICAL_SECTION m_hSaveImageMux;
};

#endif