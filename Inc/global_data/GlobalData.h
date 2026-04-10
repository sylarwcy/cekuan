#ifndef DEF_NODE_DATA  // 防止头文件重复包含的宏定义
#define DEF_NODE_DATA

#include <QString>
#include "halcon_inc.h"  // Halcon图像处理库头文件

// 宏定义：标签数量和最大PLC数量
#define TAG_NUM  1024       // 标签数组的最大容量
#define MAX_PLC_NUM	10      // 最大PLC设备数量

/////////////////////////////////////////////////////////////////////////////////////////////////
// 根据实际业务需求定义的全局变量和结构体
/////////////////////////////////////////////////////////////////////////////////////////////////

// 图像数据结构体（存储坯料检测的核心数据）
// x方向定义为从上到下，y方向定义为从左到右
struct IMG_DATA
{
    unsigned int heart_beat;    // 触发计数（累加值，用于判断数据新鲜度）
    int valid_data;             // 测量数据有效性标识：1-有效，0-无效

    float slab_length;          // 坯料长度（初始沿轧制方向）
    float slab_width;           // 坯料宽度（初始沿轧辊方向）
    float center_x;             // 坯料中心x坐标
    float center_y;             // 坯料中心y坐标
    float rotate_angle;         // 当前旋转角度（以水平方向为x轴，范围±180度）

    int int_back[8];            // 预留整数参数（8个）
    float float_back[8];        // 预留浮点数参数（8个）
};

/////////////////////////////////////////////////////////////////////////////////////////////////

// 标签项结构体（用于存储标签相关信息）
typedef struct
{
    int index;              // 标签索引
    int type;               // 标签类型
    int length;             // 标签长度
    int toTopSize;          // 到顶部的大小

    char strIndex[80];      // 索引字符串
    char strName[80];       // 标签名称字符串
    char strType[80];       // 类型字符串
    char strLength[80];     // 长度字符串
    char strNote[80];       // 备注信息

    QString stringName;     // 标签名称（Qt字符串类型）
}TAGITEM;

/*// PLC到L2系统的数据结构体（存储PLC发送给L2系统的实时数据）
struct PLC_OCR_DATA
{
    unsigned short beat_num;    // 节拍数（用于同步）
    short short_back1;          // 预留短整数参数1

    char ID[16];                // 坯料ID
    char steel_grade[16];       // 钢种

    float slab_thk;             // 坯料厚度
    float slab_wid;             // 坯料宽度
    float slab_len;             // 坯料长度
    float tar_thk;              // 目标厚度
    float tar_wid;              // 目标宽度
    float tar_len;              // 目标长度
    float start_rolling_T;      // 开始轧制温度
    float end_rolling_T;        // 结束轧制温度

    short signal1;              // 信号1（自定义）
    short signal2;              // 信号2（自定义）

    short curr_pass;            // 当前道次
    short total_pass;           // 总道次
    short bit_signal;           // 位信号（二进制状态）
    short status_Set;           // 设定状态

    // 设定参数
    float thk_set;          // 设定厚度（单位：mm）
    float gap_set;          // 设定辊缝（单位：mm）
    float force_set;        // 设定力（单位：kN）
    float len_set;          // 设定长度（单位：mm）
    float oil_diff_set_by_zero; // 调零后的油量差（nds-ds，单位：mm）
    float oil_tilt_set;     // 油量倾斜设定（nds-ds，单位：mm）
    float Torque_set;       // 设定扭矩（单位：kN.m）
    float Temp_Set;         // 设定温度（单位：摄氏度）
    float wid_Set;          // 设定宽度（单位：mm）

    // 实际参数
    float force_nds_zero;   // 工作侧力调零值
    float force_ds_zero;    // 传动侧力调零值

    float thk_nds_act;      // 工作侧实际厚度
    float thk_ds_act;       // 传动侧实际厚度
    float gap_nds_act;      // 工作侧实际辊缝
    float gap_ds_act;       // 传动侧实际辊缝

    float force_nds_act;    // 工作侧实际力
    float force_ds_act;     // 传动侧实际力

    float oil_nds;          // 工作侧油量
    float oil_ds;           // 传动侧油量

    float len_act;          // 实际长度

    float torque_up_act;    // 上辊实际扭矩
    float torque_dw_act;    // 下辊实际扭矩

    float temp_act;         // 实际温度
    float wid_act;          // 实际宽度

    float up_roller_spd_set;// 上辊设定速度
    float up_roller_spd_act;// 上辊实际速度
    float dw_roller_spd_set;// 下辊设定速度
    float dw_roller_spd_act;// 下辊实际速度

    // 换辊与调零触发信号
    short roller_para_cfg;  // 换辊参数配置触发
    short adj_zero_cfg;     // 调零配置触发

    float nds_stiffness;    // 工作侧刚度
    float ds_stiffness;     // 传动侧刚度

    char up_work_roller_id[16];    // 上工作辊ID
    float up_work_roller_diam;     // 上工作辊直径
    float up_work_roller_crn;      // 上工作辊辊径

    char dw_work_roller_id[16];    // 下工作辊ID
    float dw_work_roller_diam;     // 下工作辊直径
    float dw_work_roller_crn;      // 下工作辊辊径

    char up_back_roller_id[16];    // 上支撑辊ID
    float up_back_roller_diam;     // 上支撑辊直径
    float up_back_roller_crn;      // 上支撑辊辊径

    char dw_back_roller_id[16];    // 下支撑辊ID
    float dw_back_roller_diam;     // 下支撑辊直径
    float dw_back_roller_crn;      // 下支撑辊辊径
};

// L2系统到PLC的数据结构体（存储L2系统发送给PLC的控制数据）
struct OCR_PLC_DATA
{
    unsigned short beat_num;    // 节拍数（用于同步）
    short short_back1;          // 预留短整数参数1
    short short_back2;          // 预留短整数参数2
    short short_back3;          // 预留短整数参数3

    char ID[16];                // 坯料ID
    float oil_tilt_set;         // 油量倾斜设定（nds-ds，单位：mm）
    float main_drv_spd_set;     // 主传动速度设定

    float cal_length_pos;       // 长度校准位置
    float cal_pos_offset;       // 位置校准偏移量

    short short_back4;          // 预留短整数参数4
    short short_back5;          // 预留短整数参数5
};*/

// MQTT到L2系统的数据结构体（MQTT协议传输给L2的数据）
/*struct MQTT_L2_DATA
{
    float f1;   // 示例参数（可根据实际需求扩展）
};

// L2系统到MQTT的数据结构体（L2系统通过MQTT协议发送的数据）
struct L2_MQTT_DATA
{
    float f1;   // 示例参数（可根据实际需求扩展）
};*/

// 头部检测信息结构体（存储图像检测的基础配置）
typedef struct HEAD_DETECT_INFO
{
    float each_pixel_equal_mm_width;   // 每个像素对应的实际宽度（单位：mm）
    float each_pixel_equal_mm_height;  // 每个像素对应的实际高度（单位：mm）
    float img_width;                   // 图像宽度
    float img_height;                  // 图像高度
    float smooth_threshold;            // 平滑处理阈值
}HEAD_DETECT_INFO;

// 轧制完成计划结构体（存储轧制过程的计划与实际数据）
/*typedef struct FINISH_SCHEDULE
{
    char ID[32];                // 坯料ID
    char steel_grade[32];       // 钢种

    float slab_thk;             // 坯料厚度
    float slab_wid;             // 坯料宽度
    float slab_len;             // 坯料长度
    float tar_thk;              // 目标厚度
    float tar_wid;              // 目标宽度
    float tar_len;              // 目标长度
    float start_rolling_T;      // 开始轧制温度
    float end_rolling_T;        // 结束轧制温度

    int total_pass;             // 总道次

    // 设定参数（按道次存储，最多30道次）
    float thk_set[30];          // 厚度设定
    float gap_set[30];          // 辊缝设定
    float force_set[30];        // 力设定
    float len_set[30];          // 长度设定
    float oil_tilt_set[30];     // 油量倾斜设定
    short torque_set[30];       // 扭矩设定
    short temp_set[30];         // 温度设定
    short wid_set[30];          // 宽度设定
    short status_set[30];       // 状态设定

    // 实际参数（按道次存储，最多30道次）
    float thk_nds_act[30];      // 工作侧实际厚度
    float thk_ds_act[30];       // 传动侧实际厚度
    float gap_nds_act[30];      // 工作侧实际辊缝
    float gap_ds_act[30];       // 传动侧实际辊缝
    float force_nds_act[30];    // 工作侧实际力
    float force_ds_act[30];     // 传动侧实际力
    float len_act[30];          // 实际长度
    short torque_up_act[30];    // 上辊实际扭矩
    short torque_dw_act[30];    // 下辊实际扭矩
    short temp_act[30];         // 实际温度
    short wid_act[30];          // 实际宽度
    short status_act[30];       // 实际状态
}FINISH_SCHEDULE;*/

// 图像参数结构体（存储图像分辨率等计算参数）
typedef struct ImgPara
{
    double widthResolution;     // 宽度方向分辨率
    double lengthResolution;    // 长度方向分辨率

    double img_area;            // 图像面积
    double img_height;          // 图像高度
    double height;              // 实际高度
    double lr_ratio;            // 左右比例

    double left_height;         // 左侧高度
    double right_height;        // 右侧高度
    double l_r_pos_ratio;       // 左右位置比例

    QString coordX;             // X坐标（字符串形式）
    QString coordY;             // Y坐标（字符串形式）
}IMGPara;

//////////////////////////////////////////////////////////

// 节点配置结构体（存储系统的配置参数）
struct NodeSetting
{
    int style_mode;                 // 界面风格模式

    char local_window_name[128];    // 本地窗口名称
    char comm_window_name[128];     // 通信窗口名称

    char to_plc_share_mem_name[128];// L2到PLC共享内存名称
    char from_plc_share_mem_name[128];// PLC到L2共享内存名称

    int work_station_num;           //工位数量

    int data_refresh_ms;            // 数据刷新间隔（单位：ms）
    int sample_period;              // 采样周期（单位：ms）
    int test_mode;                  // 测试模式标识（0-正常模式，1-测试模式）
    int write_rot_video;            // 是否写入旋转视频（0-不写入，1-写入）

    // 数据库1配置
    char DbType1[128];              // 数据库类型
    char DbName1[128];              // 数据库名称
    char HostName1[128];            // 主机名
    int HostPort1;                  // 端口号
    char UserName1[128];            // 用户名
    char UserPwd1[128];             // 密码

    // 前相机配置
    char front_ip_addr[128];        // IP地址
    float front_expTime;            // 曝光时间
    int front_gain;                 // 增益
    float front_frameRate;          // 帧率
    int front_camera_res_width;     // 分辨率宽度
    int front_camera_res_height;    // 分辨率高度

    // 后相机配置
    char back_ip_addr[128];         // IP地址
    float back_expTime;             // 曝光时间
    int back_gain;                  // 增益
    float back_frameRate;           // 帧率
    int back_camera_res_width;      // 分辨率宽度
    int back_camera_res_height;     // 分辨率高度

    // 路径配置
    char calibrate_path[128];       // 校准文件路径
    char test_path[128];            // 测试文件路径
    char save_path[128];            // 保存路径

    // MQTT配置
    int mqtt_use;                   // 是否启用MQTT（0-禁用，1-启用）
    int mqtt_trigger_time;          // MQTT触发时间（单位：ms）
    QString mqtt_public_data;       //【号池】MQTT发布数据主题
    QString mqtt_public_msg;        //【错误识别的消息】MQTT发布消息主题
    QString mqtt_hostname;          // MQTT服务器主机名
    int mqtt_port;                  // MQTT服务器端口
    QString mqtt_save_image_path;   // MQTT图像保存路径
    int mqtt_image_cycle_num;       // MQTT图像循环数量
};

// 相机初始化参数结构体（存储相机窗口句柄等初始化信息）
struct Camera_Init_Para
{
    HTuple winHandle_ori;   // 原始图像窗口句柄（Halcon类型）
    HTuple winHandle_pro;   // 处理后图像窗口句柄（Halcon类型）
    HWND hwnd;              // 窗口句柄（Windows类型）
};

////////////////////////////////////////////////////////

// 节点数据类（管理全局数据和配置）
class NodeData
{
public:
    NodeData();         // 构造函数
    ~NodeData();        // 析构函数

    int ReadSetting(void);  // 读取配置文件（从setting.ini加载参数）

public:
    NodeSetting setting;    // 系统配置参数

    // 全局图像存储
    HObject ho_image_front_ori;    // 前相机原始图像（Halcon对象）
    HObject ho_image_front_pro;    // 前相机处理后图像（Halcon对象）
    HObject ho_image_back_ori;     // 后相机原始图像（Halcon对象）
    HObject ho_image_back_pro;     // 后相机处理后图像（Halcon对象）

    bool isMQTTProcessing;  // MQTT处理状态标识（true-正在处理）
    bool isDispProcessing;  // 显示处理状态标识（true-正在处理）

    // 检测信息
    HEAD_DETECT_INFO head_front_info;   // 前相机头部检测信息
    HEAD_DETECT_INFO head_back_info;    // 后相机头部检测信息

    // 相机初始化参数
    struct Camera_Init_Para camera_1_para;  // 前相机初始化参数
    struct Camera_Init_Para camera_2_para;   // 后相机初始化参数

    // FINISH_SCHEDULE finishSave;    // 保存的轧制计划数据
    // FINISH_SCHEDULE finishAct;     // 实际的轧制计划数据
};

/////////////////////////////////////////////////////////////////////////////////////////////////
#endif