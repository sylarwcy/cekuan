#include "QsLog.h"           // 日志库头文件
#define _CRT_SECURE_NO_WARNINGS  // 禁用VS的安全函数警告（如strcpy）
#include <stdio.h>           // 标准输入输出库
#include <string.h>          // 字符串处理库
#include <QDebug>            // Qt调试库
#include <wtypes.h>          // Windows类型定义（如HWND）

#include "GlobalData.h"      // 全局数据头文件
#include "ini/settings.h"    // INI配置文件处理类

///////////////////////////////////////////////////////////

// NodeData类构造函数
NodeData::NodeData()
{
    // 初始化轧制计划结构体（清零）
    // memset(&finishSave, 0, sizeof(FINISH_SCHEDULE));
    // memset(&finishAct, 0, sizeof(FINISH_SCHEDULE));

    // 初始化处理状态标识
    isMQTTProcessing = false;  // MQTT处理未进行
    isDispProcessing = false;  // 显示处理未进行
}

// NodeData类析构函数
NodeData::~NodeData()
{
    // 空实现（无动态内存需要释放）
}

// 读取配置文件（从setting.ini加载参数到setting结构体）
int NodeData::ReadSetting(void)
{
    // 创建INI配置文件处理对象（指定配置文件为setting.ini）
    IniSettings setting_ini("setting.ini");

    // 读取程序通用设定（[General]节点）
    setting.style_mode = setting_ini.getValue("General", "style_mode").toInt();  // 界面风格模式

    QString str;  // 临时字符串变量
    // 本地窗口名称（字符串需转换为C风格并复制到字符数组）
    strcpy(setting.local_window_name, setting_ini.getValue("General", "local_window_name").toStdString().c_str());
    // 通信窗口名称
    strcpy(setting.comm_window_name, setting_ini.getValue("General", "comm_window_name").toStdString().c_str());

    // 共享内存名称（L2<->PLC通信）
    strcpy(setting.to_plc_share_mem_name, setting_ini.getValue("General", "to_plc_share_mem_name").toStdString().c_str());
    strcpy(setting.from_plc_share_mem_name, setting_ini.getValue("General", "from_plc_share_mem_name").toStdString().c_str());

    setting.work_station_num = setting_ini.getValue("General", "work_station_num").toInt();  // 工位数量

    // 数据刷新间隔（最小200ms，防止过短导致性能问题）
    setting.data_refresh_ms = setting_ini.getValue("General", "data_refresh_ms").toInt();
    if (setting.data_refresh_ms < 200)
        setting.data_refresh_ms = 200;

    // 采样周期（最小20ms，限制采样频率上限）
    setting.sample_period = setting_ini.getValue("General", "sample_period").toInt();
    if (setting.sample_period < 20)
        setting.sample_period = 20;

    // 测试模式和旋转视频写入开关
    setting.test_mode = setting_ini.getValue("General", "test_mode").toInt();
    setting.write_rot_video = setting_ini.getValue("General", "write_rot_video").toInt();

    // 读取数据库1配置（[Db1Config]节点）
    // strcpy(setting.DbType1, setting_ini.getValue("Db1Config", "DbType").toStdString().c_str());       // 数据库类型
    // strcpy(setting.DbName1, setting_ini.getValue("Db1Config", "DbName").toStdString().c_str());       // 数据库名称
    // strcpy(setting.HostName1, setting_ini.getValue("Db1Config", "HostName").toStdString().c_str());   // 主机名
    // setting.HostPort1 = setting_ini.getValue("Db1Config", "HostPort").toInt();                        // 端口号
    // strcpy(setting.UserName1, setting_ini.getValue("Db1Config", "UserName").toStdString().c_str());   // 用户名
    // strcpy(setting.UserPwd1, setting_ini.getValue("Db1Config", "UserPwd").toStdString().c_str());     // 密码

    // 读取前相机配置（[CameraFront]节点）
    strcpy(setting.front_ip_addr, setting_ini.getValue("CameraFront", "front_ip_addr").toStdString().c_str());  // IP地址
    setting.front_expTime = setting_ini.getValue("CameraFront", "front_expTime").toFloat();                     // 曝光时间
    setting.front_gain = setting_ini.getValue("CameraFront", "front_gain").toInt();                             // 增益
    setting.front_frameRate = setting_ini.getValue("CameraFront", "front_frameRate").toFloat();                 // 帧率
    setting.front_camera_res_width = setting_ini.getValue("CameraFront", "front_camera_res_width").toInt();     // 分辨率宽度
    setting.front_camera_res_height = setting_ini.getValue("CameraFront", "front_camera_res_height").toInt();   // 分辨率高度

    // 读取后相机配置（[CameraBack]节点）
    strcpy(setting.back_ip_addr, setting_ini.getValue("CameraBack", "back_ip_addr").toStdString().c_str());     // IP地址
    setting.back_expTime = setting_ini.getValue("CameraBack", "back_expTime").toFloat();                         // 曝光时间
    setting.back_gain = setting_ini.getValue("CameraBack", "back_gain").toInt();                                 // 增益
    setting.back_frameRate = setting_ini.getValue("CameraBack", "back_frameRate").toFloat();                     // 帧率
    setting.back_camera_res_width = setting_ini.getValue("CameraBack", "back_camera_res_width").toInt();         // 分辨率宽度
    setting.back_camera_res_height = setting_ini.getValue("CameraBack", "back_camera_res_height").toInt();       // 分辨率高度

    // 读取检测通用配置（[DetectGeneral]节点）
    strcpy(setting.calibrate_path, setting_ini.getValue("DetectGeneral", "calibrate_path").toStdString().c_str());  // 校准文件路径
    strcpy(setting.test_path, setting_ini.getValue("DetectGeneral", "test_path").toStdString().c_str());          // 测试文件路径
    strcpy(setting.save_path, setting_ini.getValue("DetectGeneral", "save_path").toStdString().c_str());          // 保存路径

    // 读取MQTT配置（[MQTT]节点）
    // setting.mqtt_use = setting_ini.getValue("MQTT", "mqtt_use").toInt();                                         // 是否启用MQTT
    // setting.mqtt_trigger_time = setting_ini.getValue("MQTT", "mqtt_trigger_time").toInt();                       // 触发时间
    // setting.mqtt_public_data = setting_ini.getValue("MQTT", "mqtt_public_data");                                 // 发布数据主题
    // setting.mqtt_public_msg = setting_ini.getValue("MQTT", "mqtt_public_msg");                                   // 发布消息主题
    // setting.mqtt_hostname = setting_ini.getValue("MQTT", "mqtt_hostname");                                       // 服务器主机名
    // setting.mqtt_port = setting_ini.getValue("MQTT", "mqtt_port").toInt();                                       // 服务器端口
    // setting.mqtt_save_image_path = setting_ini.getValue("MQTT", "mqtt_save_image_path");                         // 图像保存路径
    // setting.mqtt_image_cycle_num = setting_ini.getValue("MQTT", "mqtt_image_cycle_num").toInt();                 // 图像循环数量

    return 0;  // 读取成功返回0
}