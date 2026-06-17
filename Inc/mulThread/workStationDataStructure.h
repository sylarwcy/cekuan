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

    int renderLeftX;
    int renderRightX;
    int renderY;
    int renderBoundLeftX;
    int renderBoundRightX;

    QVector<double> rowWidths;

    // 💡 新增：用于将单帧的微观轮廓坐标传给监视窗口渲染
    QVector<double> contourRows;
    QVector<double> contourColsLeft;
    QVector<double> contourColsRight;

    // 🌟 新增：自动标定专用——主副相机各自未拼接前的原始纯净图像边界像素列号(u坐标)
    double calibMasterLeftU{-1.0};
    double calibMasterRightU{-1.0};
    double calibSlaveLeftU{-1.0};
    double calibSlaveRightU{-1.0};

    HalconCpp::HObject dispImage;
    HalconCpp::HObject calibRawMasterChunk;
    HalconCpp::HObject calibRawSlaveChunk;

    WidthResult() : frameID(0), isValid(false), widthValue(0.0), yawAngle(0.0),
                    renderLeftX(-1), renderRightX(-1), renderY(-1),
                    renderBoundLeftX(-1), renderBoundRightX(-1) {
        HalconCpp::GenEmptyObj(&dispImage);
        HalconCpp::GenEmptyObj(&calibRawMasterChunk);
        HalconCpp::GenEmptyObj(&calibRawSlaveChunk);
    }
};

// 打包用于异步发送给文件服务器和 Web 端的所有核心数据包
struct PlateMqttReportData {
    QString plateID;
    double length;
    double thickness;
    double targetWidth;
    double avgWidth;
    double maxWidth;
    double minWidth;
    HalconCpp::HObject ho_fusedImage;   // 原始完整拼接大图
    HalconCpp::HObject ho_contourImage; // 绘制有红色多边形闭合轮廓的大图
    QVector<double> curveValues;        // 精细校正后的白线全量离散点列
};

Q_DECLARE_METATYPE(PlateMqttReportData)
Q_DECLARE_METATYPE(DualCameraChunk)
Q_DECLARE_METATYPE(WidthResult)

