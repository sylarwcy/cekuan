#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "head.h"

class AppConfig
{
public:
    static QString ConfigFile;      //配置文件文件路径及名称

    //基本参数
    static QString calibrate_path;

    //机前参数
    static float f_calibrate_image_scale_width;
    static float f_calibrate_image_scale_height;
    static float f_each_pixel_equal_mm_width;
    static float f_each_pixel_equal_mm_height;
    static float f_row_offset_mm;
    static float f_col_offset_mm;
    static float f_crop_image_width_mm;
    static float f_crop_image_height_mm;

    //机前标定数据
    static int f_x0_1;
    static int f_x0_2;
    static int f_x0_3;
    static int f_x0_4;

    static int f_y0_1;
    static int f_y0_2;
    static int f_y0_3;
    static int f_y0_4;

    static int f_x1_1;
    static int f_x1_2;
    static int f_x1_3;
    static int f_x1_4;

    static int f_y1_1;
    static int f_y1_2;
    static int f_y1_3;
    static int f_y1_4;

    //机后参数
    static float b_calibrate_image_scale_width;
    static float b_calibrate_image_scale_height;
    static float b_each_pixel_equal_mm_width;
    static float b_each_pixel_equal_mm_height;
    static float b_row_offset_mm;
    static float b_col_offset_mm;
    static float b_crop_image_width_mm;
    static float b_crop_image_height_mm;

    //机后标定数据
    static int b_x0_1;
    static int b_x0_2;
    static int b_x0_3;
    static int b_x0_4;

    static int b_y0_1;
    static int b_y0_2;
    static int b_y0_3;
    static int b_y0_4;

    static int b_x1_1;
    static int b_x1_2;
    static int b_x1_3;
    static int b_x1_4;

    static int b_y1_1;
    static int b_y1_2;
    static int b_y1_3;
    static int b_y1_4;


    static void readConfig();                   //读取配置文件,在main函数最开始加载程序载入
    static void writeConfig_front_data();      //写入配置文件,在更改配置文件程序关闭时调用
    static void writeConfig_back_data();      //写入配置文件,在更改配置文件程序关闭时调用
};

#endif // APPCONFIG_H
