//
// Created by sylar on 25-10-14.
//
#pragma once

#include "halconcpp/HalconCpp.h"

struct WorkStation_DATA
{
    QString m_location;       //工位位置
    int m_serialNumber;       //工位序号
    int m_loop_refresh_ms;    //通信循环刷新时间
    int run_mode;             //运行模式，存图-0，识别-1
    QString ocrBufferPath;    //ocr数据存图路径
    QString detBufferPath;    //det数据存图路径
    QString remotePathPrefix; //二级存图的路径（共享文件夹路径）

    // 0.图像处理参数
    QString det_model_path;      //目标检测模型路径
    QString det_param_path;      //目标检测数据集参数
    QString ocr_det_1_path;      //ocr一级检测模型
    QString ocr_det_2_path;      //ocr二级检测模型
    QString ocr_recog_2_path;    //ocr二级识别模型
    // QString device;           //计算设备 cpu or gpu
    // 仿射变换参数
    QString affine_x_pre;      // "1348,-96,1405,1874"
    QString affine_y_pre;      // "458,376,4111,2936"
    QString affine_x_post;     // "1000,0,0,1000"
    QString affine_y_post;     // "0,0,2000,2000"
    // 区域参数
    QString roi_region;  //"540,100,900,1850"
    //目标跟踪参数
    QString max_age;
    QString min_hits;
    QString iouThreshold;

    // 1.相机参数
    QString master_ip, master_sn;           // 主相机IP
    QString slave_ip, slave_sn;            // 从相机IP
    int camImgWidth;             // 图像宽
    int camImgHeight;            // 图像高
    QString test_img_path;       // 测试图片路径

    // 2.数据库参数
    QString dbType;              // 数据库类型
    QString dbConnName;          // 连接名称
    QString dbName;              // 数据库名
    QString dbHostName;          // 主机地址
    int dbHostPort;              // 通信端口
    QString dbUserName;          // 用户名称
    QString dbUserPwd;           // 用户密码
    QString dbTableName;         // 数据库表名
    QString dbKeyName;           // 数据库表的字段名

    // 3.MQTT参数
    int mqttUse;                 // 是否启用MQTT（0-禁用，1-启用）
    int mqttTriggerTime;         // MQTT触发时间（单位：ms）
    QString mqttHostname;        // MQTT服务器主机名
    int mqttPort;                // MQTT服务器端口
    QString mqttSaveImagePath;   // MQTT图像保存路径
    int mqttImageCycleNum;       // MQTT图像循环数量
    QString mqttPublicMsg;       // MQTT消息主题-【错误识别的消息】
};

// 1. 底层采集发给算法的“双目同步图像块”
struct DualCameraChunk {
    uint64_t frameID;           // 统一的帧号 (基于硬件 BlockID 或 MVS 的 nFrameNum)
    int height;                 // 图像行数

    bool hasLeft;                 // 左图是否到达
    bool hasRight;                // 右图是否到达

    HalconCpp::HObject imgLeft{};   // 左相机图像 (Master)
    HalconCpp::HObject imgRight{};  // 右相机图像 (Slave)

    // 构造函数：初始化标志位
    DualCameraChunk() : frameID(0), height(0), hasLeft(false), hasRight(false) {}
};

// 2. 算法发给 UI 和 数据库的“测量结果”
struct WidthResult {
    uint64_t frameID;
    bool isValid;
    double widthValue;
    double yawAngle;

    // UI 画十字线专用的中位数坐标 (代表平均宽度)
    int renderLeftX;
    int renderRightX;
    int renderY;

    // 💡【新增】：当前帧最宽处绝对极限包络坐标 (专供切边使用，防圆弧截断)
    int renderBoundLeftX;
    int renderBoundRightX;

    QVector<double> rowWidths;
    HalconCpp::HObject dispImage;

    // 💡【更新】：记得在构造函数里把新增的变量也初始化一下 (-1 代表无效)
    WidthResult() : frameID(0), isValid(false), widthValue(0.0), yawAngle(0.0),
                    renderLeftX(-1), renderRightX(-1), renderY(-1),
                    renderBoundLeftX(-1), renderBoundRightX(-1), rowWidths() {
        HalconCpp::GenEmptyObj(&dispImage);
    }
};

Q_DECLARE_METATYPE(DualCameraChunk)
Q_DECLARE_METATYPE(WidthResult)

