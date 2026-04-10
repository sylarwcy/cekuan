#include "appconfig.h"
#include "quihelper.h"

QString AppConfig::ConfigFile = "Calibrate.ini";


//general
QString AppConfig::calibrate_path ="image_calibrate";

//机前
float AppConfig::f_calibrate_image_scale_width=0.5;
float AppConfig::f_calibrate_image_scale_height=0.5;
float AppConfig::f_each_pixel_equal_mm_width=10;
float AppConfig::f_each_pixel_equal_mm_height=20;
float AppConfig::f_row_offset_mm=500;
float AppConfig::f_col_offset_mm=500;
float AppConfig::f_crop_image_width_mm=4200;
float AppConfig::f_crop_image_height_mm=3700;

//CalibrateFront
int AppConfig::f_x0_1=246;
int AppConfig::f_x0_2=127;
int AppConfig::f_x0_3=276;
int AppConfig::f_x0_4=473;

int AppConfig::f_y0_1=58;
int AppConfig::f_y0_2=372;
int AppConfig::f_y0_3=610;
int AppConfig::f_y0_4=232;

int AppConfig::f_x1_1=0;
int AppConfig::f_x1_2=0;
int AppConfig::f_x1_3=480;
int AppConfig::f_x1_4=480;

int AppConfig::f_y1_1=0;
int AppConfig::f_y1_2=640;
int AppConfig::f_y1_3=640;
int AppConfig::f_y1_4=0;

//机后
float AppConfig::b_calibrate_image_scale_width=0.5;
float AppConfig::b_calibrate_image_scale_height=0.5;
float AppConfig::b_each_pixel_equal_mm_width=10;
float AppConfig::b_each_pixel_equal_mm_height=20;
float AppConfig::b_row_offset_mm=500;
float AppConfig::b_col_offset_mm=500;
float AppConfig::b_crop_image_width_mm=4200;
float AppConfig::b_crop_image_height_mm=3700;

//CalibrateFront
int AppConfig::b_x0_1=246;
int AppConfig::b_x0_2=127;
int AppConfig::b_x0_3=276;
int AppConfig::b_x0_4=473;

int AppConfig::b_y0_1=58;
int AppConfig::b_y0_2=372;
int AppConfig::b_y0_3=610;
int AppConfig::b_y0_4=232;

int AppConfig::b_x1_1=0;
int AppConfig::b_x1_2=0;
int AppConfig::b_x1_3=480;
int AppConfig::b_x1_4=480;

int AppConfig::b_y1_1=0;
int AppConfig::b_y1_2=640;
int AppConfig::b_y1_3=640;
int AppConfig::b_y1_4=0;


void AppConfig::readConfig()
{
    //配置文件不存在或者有问题则重新生成
    if (!QUIHelper::checkIniFile(ConfigFile))
    {
        writeConfig_front_data();
        writeConfig_back_data();
        return;
    }

    QSettings set(ConfigFile, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    set.setIniCodec("utf-8");
#endif

    set.beginGroup("General");
    calibrate_path=set.value("calibrate_path", calibrate_path).toString();
    set.endGroup();

    set.beginGroup("Front_Para");
    f_calibrate_image_scale_width=set.value("calibrate_image_scale_width", f_calibrate_image_scale_width).toFloat();
    f_calibrate_image_scale_height=set.value("calibrate_image_scale_height", f_calibrate_image_scale_height).toFloat();
    f_each_pixel_equal_mm_width=set.value("each_pixel_equal_mm_width", f_each_pixel_equal_mm_width).toFloat();
    f_each_pixel_equal_mm_height=set.value("each_pixel_equal_mm_height", f_each_pixel_equal_mm_height).toFloat();
    f_row_offset_mm=set.value("row_offset_mm", f_row_offset_mm).toFloat();
    f_col_offset_mm=set.value("col_offset_mm", f_col_offset_mm).toFloat();
    f_crop_image_width_mm=set.value("crop_image_width_mm", f_crop_image_width_mm).toFloat();
    f_crop_image_height_mm=set.value("crop_image_height_mm", f_crop_image_height_mm).toFloat();
    set.endGroup();

    //Calibrate
    set.beginGroup("Front_Data");
    f_x0_1 = set.value("x0_1", f_x0_1).toInt();
    f_x0_2 = set.value("x0_2", f_x0_2).toInt();
    f_x0_3 = set.value("x0_3", f_x0_3).toInt();
    f_x0_4 = set.value("x0_4", f_x0_4).toInt();

    f_y0_1 = set.value("y0_1", f_y0_1).toInt();
    f_y0_2 = set.value("y0_2", f_y0_2).toInt();
    f_y0_3 = set.value("y0_3", f_y0_3).toInt();
    f_y0_4 = set.value("y0_4", f_y0_4).toInt();

    f_x1_1 = set.value("x1_1", f_x1_1).toInt();
    f_x1_2 = set.value("x1_2", f_x1_2).toInt();
    f_x1_3 = set.value("x1_3", f_x1_3).toInt();
    f_x1_4 = set.value("x1_4", f_x1_4).toInt();

    f_y1_1 = set.value("y1_1", f_y1_1).toInt();
    f_y1_2 = set.value("y1_2", f_y1_2).toInt();
    f_y1_3 = set.value("y1_3", f_y1_3).toInt();
    f_y1_4 = set.value("y1_4", f_y1_4).toInt();
    set.endGroup();

    set.beginGroup("Back_Para");
    b_calibrate_image_scale_width=set.value("calibrate_image_scale_width", b_calibrate_image_scale_width).toFloat();
    b_calibrate_image_scale_height=set.value("calibrate_image_scale_height", b_calibrate_image_scale_height).toFloat();
    b_each_pixel_equal_mm_width=set.value("each_pixel_equal_mm_width", b_each_pixel_equal_mm_width).toFloat();
    b_each_pixel_equal_mm_height=set.value("each_pixel_equal_mm_height", b_each_pixel_equal_mm_height).toFloat();
    b_row_offset_mm=set.value("row_offset_mm", b_row_offset_mm).toFloat();
    b_col_offset_mm=set.value("col_offset_mm", b_col_offset_mm).toFloat();
    b_crop_image_width_mm=set.value("crop_image_width_mm", b_crop_image_width_mm).toFloat();
    b_crop_image_height_mm=set.value("crop_image_height_mm", b_crop_image_height_mm).toFloat();
    set.endGroup();

    //Calibrate
    set.beginGroup("Back_Data");
    b_x0_1 = set.value("x0_1", b_x0_1).toInt();
    b_x0_2 = set.value("x0_2", b_x0_2).toInt();
    b_x0_3 = set.value("x0_3", b_x0_3).toInt();
    b_x0_4 = set.value("x0_4", b_x0_4).toInt();

    b_y0_1 = set.value("y0_1", b_y0_1).toInt();
    b_y0_2 = set.value("y0_2", b_y0_2).toInt();
    b_y0_3 = set.value("y0_3", b_y0_3).toInt();
    b_y0_4 = set.value("y0_4", b_y0_4).toInt();

    b_x1_1 = set.value("x1_1", b_x1_1).toInt();
    b_x1_2 = set.value("x1_2", b_x1_2).toInt();
    b_x1_3 = set.value("x1_3", b_x1_3).toInt();
    b_x1_4 = set.value("x1_4", b_x1_4).toInt();

    b_y1_1 = set.value("y1_1", b_y1_1).toInt();
    b_y1_2 = set.value("y1_2", b_y1_2).toInt();
    b_y1_3 = set.value("y1_3", b_y1_3).toInt();
    b_y1_4 = set.value("y1_4", b_y1_4).toInt();
    set.endGroup();

}

void AppConfig::writeConfig_front_data()
{
    QSettings set(ConfigFile, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    set.setIniCodec("utf-8");
#endif

    //Front_data
    set.beginGroup("Front_Data");
    set.setValue("x0_1", f_x0_1);
    set.setValue("x0_2", f_x0_2);
    set.setValue("x0_3", f_x0_3);
    set.setValue("x0_4", f_x0_4);

    set.setValue("y0_1", f_y0_1);
    set.setValue("y0_2", f_y0_2);
    set.setValue("y0_3", f_y0_3);
    set.setValue("y0_4", f_y0_4);

    set.setValue("x1_1", f_x1_1);
    set.setValue("x1_2", f_x1_2);
    set.setValue("x1_3", f_x1_3);
    set.setValue("x1_4", f_x1_4);

    set.setValue("y1_1", f_y1_1);
    set.setValue("y1_2", f_y1_2);
    set.setValue("y1_3", f_y1_3);
    set.setValue("y1_4", f_y1_4);
    set.endGroup();

    set.beginGroup("Front_Para");
    set.setValue("calibrate_image_scale_width",QString("%1").arg(f_calibrate_image_scale_width));
    set.setValue("calibrate_image_scale_height",QString("%1").arg(f_calibrate_image_scale_height));
    set.endGroup();

}

void AppConfig::writeConfig_back_data()
{
    QSettings set(ConfigFile, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    set.setIniCodec("utf-8");
#endif

    //Back_data
    set.beginGroup("Back_Data");
    set.setValue("x0_1", b_x0_1);
    set.setValue("x0_2", b_x0_2);
    set.setValue("x0_3", b_x0_3);
    set.setValue("x0_4", b_x0_4);

    set.setValue("y0_1", b_y0_1);
    set.setValue("y0_2", b_y0_2);
    set.setValue("y0_3", b_y0_3);
    set.setValue("y0_4", b_y0_4);

    set.setValue("x1_1", b_x1_1);
    set.setValue("x1_2", b_x1_2);
    set.setValue("x1_3", b_x1_3);
    set.setValue("x1_4", b_x1_4);

    set.setValue("y1_1", b_y1_1);
    set.setValue("y1_2", b_y1_2);
    set.setValue("y1_3", b_y1_3);
    set.setValue("y1_4", b_y1_4);
    set.endGroup();


    set.beginGroup("Back_Para");
    set.setValue("calibrate_image_scale_width",QString("%1").arg(b_calibrate_image_scale_width));
    set.setValue("calibrate_image_scale_height",QString("%1").arg(b_calibrate_image_scale_height));
    set.endGroup();
}
