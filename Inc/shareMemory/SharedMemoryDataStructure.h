//
// Created by sylar on 25-10-9.
//

#ifndef SHAREDMEMORYDATASTRUCTURE_H
#define SHAREDMEMORYDATASTRUCTURE_H

// PLC到OCR系统的数据结构体（存储PLC发送给OCR系统的实时数据）
struct From_PLC_DATA
{
    int beat_num;            // 垛板台-心跳
    int motor_signal;        // 推钢机电机运转信号 <1-转|0-停>
    int motor_direction;     // 推钢机电机转动方向 <1-正|2-反|0-停>  //热送辊道速度
    int cold_inspection;     // 加热炉边冷检 <1-有|0-无>
};

// OCR系统到PLC的数据结构体（存储OCR系统发送给PLC的控制数据）
struct TO_PLC_DATA
{
    int beat_num;         // 翻板台-心跳
    int has_steel_plate;  // 是否有钢板 <1-有|0-无>
    int gundao_X_left;    // 沿辊道方向<左侧头部>坐标
    char ID[32];          // 板号
    char img_name[32];    // 图片名
    int recog_finished_flag; // 该块钢板识别完成 <1-完成|0-未完成>
    int has_cam_broken;   // 相机是否故障 <1-有|0-无>
    int bak1;             // 热送，删除反向回流板子的信号
    int bak2;             //备用2
    int bak3;             //备用3
};

#endif //SHAREDMEMORYDATASTRUCTURE_H
