#include "frmview2.h"
#include "ui_frmview2.h"
#include "appconfig.h"

#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QMessageBox>

#include "MyApplication.h"

//Halcon包含
#ifndef __APPLE__
#  include "HalconCpp.h"
#  include "HDevThread.h"
#else
#  ifndef HC_LARGE_IMAGES
#    include <HALCONCpp/HalconCpp.h>
#    include <HALCONCpp/HDevThread.h>
#    include <HALCON/HpThread.h>
#  else
#    include <HALCONCppxl/HalconCpp.h>
#    include <HALCONCppxl/HDevThread.h>
#    include <HALCONxl/HpThread.h>
#  endif
#  include <stdio.h>
#  include <CoreFoundation/CFRunLoop.h>
#endif

using namespace HalconCpp;
///////////////////////////////////////////////////////////////////////////////////////
frmView2::frmView2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmView2)
{
    ui->setupUi(this);

    initForm();
    loadCalibratePata();

    ui->widget->setPolygonColor(QColor(255,0,0,50));
    ui->widget->setSelectColor(QColor(0,255,0,50));

    isGetPointOK=false;

    connect(ui->widget, &CustomGraphics::receivePoints, this, &frmView2::receivePoints);

    PrintEvent("标定参数初始化装入完成...",true);

}

frmView2::~frmView2()
{
    delete ui;
}

void frmView2::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    widget_width=ui->widget->width();
    widget_height=ui->widget->height();

    QString str=QString("控件宽度:%1,控件高度:%2").arg(widget_width).arg(widget_height);
    PrintEvent(str,true);

    //加载机前图片
    on_pushButton_load_pic_front_clicked();
}

void frmView2::PrintEvent(QString str,bool isDispTime)
{
    char temp[50];
    char strBuff[1024];

    SYSTEMTIME sys;
    GetLocalTime(&sys);
    sprintf(temp, "%4d/%02d/%02d %02d:%02d:%02d.%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);

    if(isDispTime)
        sprintf(strBuff, "%s--->%s", temp, str.toStdString().c_str());
    else
        sprintf(strBuff, "%s", str.toStdString().c_str());

    msg_list.append(strBuff);
    msg_model->setStringList(msg_list);

    if(msg_list.count()>5000)
        msg_list.clear();

    ui->listView_info->scrollToBottom();

}

void frmView2::initForm()
{
    //tableView_cali_para_front
    //颜色设置
    ui->tableView_cali_para_front->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color:rgb(245, 245, 245);}");

    //单选
    ui->tableView_cali_para_front->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_cali_para_front->setSelectionBehavior(QAbstractItemView::SelectRows);

    //tableView_para
    model_cali_para_front = new QStandardItemModel(this);
    model_cali_para_front->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    model_cali_para_front->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr(u8"数值")));
    ui->tableView_cali_para_front->setModel(model_cali_para_front);

    ui->tableView_cali_para_front->setColumnWidth(0,150);
    ui->tableView_cali_para_front->setColumnWidth(1,110);

    model_cali_para_front->setItem(0, 0, new QStandardItem("点1原始横坐标 x0_1"));
    model_cali_para_front->setItem(1, 0, new QStandardItem("点1原始纵坐标 y0_1"));
    model_cali_para_front->setItem(2, 0, new QStandardItem("点2原始横坐标 x0_2"));
    model_cali_para_front->setItem(3, 0, new QStandardItem("点2原始纵坐标 y0_2"));
    model_cali_para_front->setItem(4, 0, new QStandardItem("点3原始横坐标 x0_3"));
    model_cali_para_front->setItem(5, 0, new QStandardItem("点3原始纵坐标 y0_3"));
    model_cali_para_front->setItem(6, 0, new QStandardItem("点4原始横坐标 x0_4"));
    model_cali_para_front->setItem(7, 0, new QStandardItem("点4原始纵坐标 y0_4"));

    model_cali_para_front->setItem(8, 0, new QStandardItem("点1变换后横坐标 x1_1"));
    model_cali_para_front->setItem(9, 0, new QStandardItem("点1变换后纵坐标 y1_1"));
    model_cali_para_front->setItem(10, 0, new QStandardItem("点2变换后横坐标 x1_2"));
    model_cali_para_front->setItem(11, 0, new QStandardItem("点2变换后纵坐标 y1_2"));
    model_cali_para_front->setItem(12, 0, new QStandardItem("点3变换后横坐标 x1_3"));
    model_cali_para_front->setItem(13, 0, new QStandardItem("点3变换后纵坐标 y1_3"));
    model_cali_para_front->setItem(14, 0, new QStandardItem("点4变换后横坐标 x1_4"));
    model_cali_para_front->setItem(15, 0, new QStandardItem("点4变换后纵坐标 y1_4"));

    //tableView_cal_para
    model_cal_para_front = new QStandardItemModel(this);
    model_cal_para_front->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    model_cal_para_front->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr(u8"数值")));
    ui->tableView_cal_para_front->setModel(model_cal_para_front);

    ui->tableView_cal_para_front->setColumnWidth(0,150);
    ui->tableView_cal_para_front->setColumnWidth(1,110);

    model_cal_para_front->setItem(0, 0, new QStandardItem("标定图像宽度缩放系数"));
    model_cal_para_front->setItem(1, 0, new QStandardItem("标定图像高度缩放系数"));
    model_cal_para_front->setItem(2, 0, new QStandardItem("宽度方向每个像素mm长度"));
    model_cal_para_front->setItem(3, 0, new QStandardItem("长度方向每个像素mm长度"));
    model_cal_para_front->setItem(4, 0, new QStandardItem("投影变换行偏移mm"));
    model_cal_para_front->setItem(5, 0, new QStandardItem("投影变换列偏移mm"));
    model_cal_para_front->setItem(6, 0, new QStandardItem("变换后裁剪图像宽度mm"));
    model_cal_para_front->setItem(7, 0, new QStandardItem("变换后裁剪图像高度mm"));

    //tableView_cali_para_back
    //颜色设置
    ui->tableView_cali_para_back->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color:rgb(245, 245, 245);}");

    //单选
    ui->tableView_cali_para_back->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_cali_para_back->setSelectionBehavior(QAbstractItemView::SelectRows);

    //tableView_para
    model_cali_para_back = new QStandardItemModel(this);
    model_cali_para_back->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    model_cali_para_back->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr(u8"数值")));
    ui->tableView_cali_para_back->setModel(model_cali_para_back);

    ui->tableView_cali_para_back->setColumnWidth(0,150);
    ui->tableView_cali_para_back->setColumnWidth(1,110);

    model_cali_para_back->setItem(0, 0, new QStandardItem("点1原始横坐标 x0_1"));
    model_cali_para_back->setItem(1, 0, new QStandardItem("点1原始纵坐标 y0_1"));
    model_cali_para_back->setItem(2, 0, new QStandardItem("点2原始横坐标 x0_2"));
    model_cali_para_back->setItem(3, 0, new QStandardItem("点2原始纵坐标 y0_2"));
    model_cali_para_back->setItem(4, 0, new QStandardItem("点3原始横坐标 x0_3"));
    model_cali_para_back->setItem(5, 0, new QStandardItem("点3原始纵坐标 y0_3"));
    model_cali_para_back->setItem(6, 0, new QStandardItem("点4原始横坐标 x0_4"));
    model_cali_para_back->setItem(7, 0, new QStandardItem("点4原始纵坐标 y0_4"));

    model_cali_para_back->setItem(8, 0, new QStandardItem("点1变换后横坐标 x1_1"));
    model_cali_para_back->setItem(9, 0, new QStandardItem("点1变换后纵坐标 y1_1"));
    model_cali_para_back->setItem(10, 0, new QStandardItem("点2变换后横坐标 x1_2"));
    model_cali_para_back->setItem(11, 0, new QStandardItem("点2变换后纵坐标 y1_2"));
    model_cali_para_back->setItem(12, 0, new QStandardItem("点3变换后横坐标 x1_3"));
    model_cali_para_back->setItem(13, 0, new QStandardItem("点3变换后纵坐标 y1_3"));
    model_cali_para_back->setItem(14, 0, new QStandardItem("点4变换后横坐标 x1_4"));
    model_cali_para_back->setItem(15, 0, new QStandardItem("点4变换后纵坐标 y1_4"));

    //tableView_cal_para
    model_cal_para_back = new QStandardItemModel(this);
    model_cal_para_back->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    model_cal_para_back->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr(u8"数值")));
    ui->tableView_cal_para_back->setModel(model_cal_para_back);

    ui->tableView_cal_para_back->setColumnWidth(0,150);
    ui->tableView_cal_para_back->setColumnWidth(1,110);

    model_cal_para_back->setItem(0, 0, new QStandardItem("标定图像宽度缩放系数"));
    model_cal_para_back->setItem(1, 0, new QStandardItem("标定图像高度缩放系数"));
    model_cal_para_back->setItem(2, 0, new QStandardItem("宽度方向每个像素mm长度"));
    model_cal_para_back->setItem(3, 0, new QStandardItem("长度方向每个像素mm长度"));
    model_cal_para_back->setItem(4, 0, new QStandardItem("投影变换行偏移mm"));
    model_cal_para_back->setItem(5, 0, new QStandardItem("投影变换列偏移mm"));
    model_cal_para_back->setItem(6, 0, new QStandardItem("变换后裁剪图像宽度mm"));
    model_cal_para_back->setItem(7, 0, new QStandardItem("变换后裁剪图像高度mm"));

    //初始化信息表listview
    msg_model=new QStringListModel(this);
    ui->listView_info->setModel(msg_model);
}

void frmView2::loadCalibratePata()
{
    MyApplication *pApp=(MyApplication *)qApp;

    //Calibrate para front
    model_cali_para_front->setItem(0, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x0_1)));
    model_cali_para_front->setItem(1, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y0_1)));
    model_cali_para_front->setItem(2, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x0_2)));
    model_cali_para_front->setItem(3, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y0_2)));
    model_cali_para_front->setItem(4, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x0_3)));
    model_cali_para_front->setItem(5, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y0_3)));
    model_cali_para_front->setItem(6, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x0_4)));
    model_cali_para_front->setItem(7, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y0_4)));

    model_cali_para_front->setItem(8, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x1_1)));
    model_cali_para_front->setItem(9, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y1_1)));
    model_cali_para_front->setItem(10, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x1_2)));
    model_cali_para_front->setItem(11, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y1_2)));
    model_cali_para_front->setItem(12, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x1_3)));
    model_cali_para_front->setItem(13, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y1_3)));
    model_cali_para_front->setItem(14, 1, new QStandardItem(QString("%1").arg(AppConfig::f_x1_4)));
    model_cali_para_front->setItem(15, 1, new QStandardItem(QString("%1").arg(AppConfig::f_y1_4)));

    //cal para front
    model_cal_para_front->setItem(0, 1, new QStandardItem(QString("%1").arg(AppConfig::f_calibrate_image_scale_width)));
    model_cal_para_front->setItem(1, 1, new QStandardItem(QString("%1").arg(AppConfig::f_calibrate_image_scale_height)));
    model_cal_para_front->setItem(2, 1, new QStandardItem(QString("%1").arg(AppConfig::f_each_pixel_equal_mm_width)));
    model_cal_para_front->setItem(3, 1, new QStandardItem(QString("%1").arg(AppConfig::f_each_pixel_equal_mm_height)));
    model_cal_para_front->setItem(4, 1, new QStandardItem(QString("%1").arg(AppConfig::f_row_offset_mm)));
    model_cal_para_front->setItem(5, 1, new QStandardItem(QString("%1").arg(AppConfig::f_col_offset_mm)));
    model_cal_para_front->setItem(6, 1, new QStandardItem(QString("%1").arg(AppConfig::f_crop_image_width_mm)));
    model_cal_para_front->setItem(7, 1, new QStandardItem(QString("%1").arg(AppConfig::f_crop_image_height_mm)));

    //Calibrate para back
    model_cali_para_back->setItem(0, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x0_1)));
    model_cali_para_back->setItem(1, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y0_1)));
    model_cali_para_back->setItem(2, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x0_2)));
    model_cali_para_back->setItem(3, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y0_2)));
    model_cali_para_back->setItem(4, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x0_3)));
    model_cali_para_back->setItem(5, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y0_3)));
    model_cali_para_back->setItem(6, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x0_4)));
    model_cali_para_back->setItem(7, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y0_4)));

    model_cali_para_back->setItem(8, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x1_1)));
    model_cali_para_back->setItem(9, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y1_1)));
    model_cali_para_back->setItem(10, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x1_2)));
    model_cali_para_back->setItem(11, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y1_2)));
    model_cali_para_back->setItem(12, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x1_3)));
    model_cali_para_back->setItem(13, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y1_3)));
    model_cali_para_back->setItem(14, 1, new QStandardItem(QString("%1").arg(AppConfig::b_x1_4)));
    model_cali_para_back->setItem(15, 1, new QStandardItem(QString("%1").arg(AppConfig::b_y1_4)));

    //cal para back
    model_cal_para_back->setItem(0, 1, new QStandardItem(QString("%1").arg(AppConfig::b_calibrate_image_scale_width)));
    model_cal_para_back->setItem(1, 1, new QStandardItem(QString("%1").arg(AppConfig::b_calibrate_image_scale_height)));
    model_cal_para_back->setItem(2, 1, new QStandardItem(QString("%1").arg(AppConfig::b_each_pixel_equal_mm_width)));
    model_cal_para_back->setItem(3, 1, new QStandardItem(QString("%1").arg(AppConfig::b_each_pixel_equal_mm_height)));
    model_cal_para_back->setItem(4, 1, new QStandardItem(QString("%1").arg(AppConfig::b_row_offset_mm)));
    model_cal_para_back->setItem(5, 1, new QStandardItem(QString("%1").arg(AppConfig::b_col_offset_mm)));
    model_cal_para_back->setItem(6, 1, new QStandardItem(QString("%1").arg(AppConfig::b_crop_image_width_mm)));
    model_cal_para_back->setItem(7, 1, new QStandardItem(QString("%1").arg(AppConfig::b_crop_image_height_mm)));


    //标定参数
    //投影坐标x为row或height，y为col或width
    float f_scale_coff_width=AppConfig::f_calibrate_image_scale_width;
    float f_scale_coff_height=AppConfig::f_calibrate_image_scale_height;

    //获取投射变换参数HomMat2D
    //标定图像缩放0.5，恢复原始像素坐标
    pApp->f_hv_x0.Clear();
    pApp->f_hv_x0[0] = AppConfig::f_x0_1/f_scale_coff_height;
    pApp->f_hv_x0[1] = AppConfig::f_x0_2/f_scale_coff_height;
    pApp->f_hv_x0[2] = AppConfig::f_x0_3/f_scale_coff_height;
    pApp->f_hv_x0[3] = AppConfig::f_x0_4/f_scale_coff_height;

    pApp->f_hv_y0.Clear();
    pApp->f_hv_y0[0] = AppConfig::f_y0_1/f_scale_coff_width;
    pApp->f_hv_y0[1] = AppConfig::f_y0_2/f_scale_coff_width;
    pApp->f_hv_y0[2] = AppConfig::f_y0_3/f_scale_coff_width;
    pApp->f_hv_y0[3] = AppConfig::f_y0_4/f_scale_coff_width;

    //mm变为像素
    float f_each_pixel_equal_mm_col=AppConfig::f_each_pixel_equal_mm_width;
    float f_each_pixel_equal_mm_row=AppConfig::f_each_pixel_equal_mm_height;
    float f_row_offset=AppConfig::f_row_offset_mm/AppConfig::f_each_pixel_equal_mm_height;
    float f_col_offset=AppConfig::f_col_offset_mm/AppConfig::f_each_pixel_equal_mm_width;

    pApp->f_hv_x1.Clear();
    pApp->f_hv_x1[0] = AppConfig::f_x1_1/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[1] = AppConfig::f_x1_2/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[2] = AppConfig::f_x1_3/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[3] = AppConfig::f_x1_4/f_each_pixel_equal_mm_row+f_row_offset;

    pApp->f_hv_y1.Clear();
    pApp->f_hv_y1[0] = AppConfig::f_y1_1/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[1] = AppConfig::f_y1_2/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[2] = AppConfig::f_y1_3/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[3] = AppConfig::f_y1_4/f_each_pixel_equal_mm_col+f_col_offset;


    //投影坐标x为row或height，y为col或width
    float b_scale_coff_width=AppConfig::b_calibrate_image_scale_width;
    float b_scale_coff_height=AppConfig::b_calibrate_image_scale_height;

    //获取投射变换参数HomMat2D
    //标定图像缩放0.5，恢复原始像素坐标
    pApp->b_hv_x0.Clear();
    pApp->b_hv_x0[0] = AppConfig::b_x0_1/b_scale_coff_height;
    pApp->b_hv_x0[1] = AppConfig::b_x0_2/b_scale_coff_height;
    pApp->b_hv_x0[2] = AppConfig::b_x0_3/b_scale_coff_height;
    pApp->b_hv_x0[3] = AppConfig::b_x0_4/b_scale_coff_height;

    pApp->b_hv_y0.Clear();
    pApp->b_hv_y0[0] = AppConfig::b_y0_1/b_scale_coff_width;
    pApp->b_hv_y0[1] = AppConfig::b_y0_2/b_scale_coff_width;
    pApp->b_hv_y0[2] = AppConfig::b_y0_3/b_scale_coff_width;
    pApp->b_hv_y0[3] = AppConfig::b_y0_4/b_scale_coff_width;

    //mm变为像素
    float b_each_pixel_equal_mm_col=AppConfig::b_each_pixel_equal_mm_width;
    float b_each_pixel_equal_mm_row=AppConfig::b_each_pixel_equal_mm_height;
    float b_row_offset=AppConfig::b_row_offset_mm/AppConfig::b_each_pixel_equal_mm_height;
    float b_col_offset=AppConfig::b_col_offset_mm/AppConfig::b_each_pixel_equal_mm_width;

    pApp->b_hv_x1.Clear();
    pApp->b_hv_x1[0] = AppConfig::b_x1_1/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[1] = AppConfig::b_x1_2/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[2] = AppConfig::b_x1_3/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[3] = AppConfig::b_x1_4/b_each_pixel_equal_mm_row+b_row_offset;

    pApp->b_hv_y1.Clear();
    pApp->b_hv_y1[0] = AppConfig::b_y1_1/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[1] = AppConfig::b_y1_2/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[2] = AppConfig::b_y1_3/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[3] = AppConfig::b_y1_4/b_each_pixel_equal_mm_col+b_col_offset;

 }

void frmView2::readDataFromTableView()
{
    //Front Calibrate
    AppConfig::f_x0_1 = model_cali_para_front->item(0,1)->text().toInt();
    AppConfig::f_y0_1 = model_cali_para_front->item(1,1)->text().toInt();
    AppConfig::f_x0_2 = model_cali_para_front->item(2,1)->text().toInt();
    AppConfig::f_y0_2 = model_cali_para_front->item(3,1)->text().toInt();
    AppConfig::f_x0_3 = model_cali_para_front->item(4,1)->text().toInt();
    AppConfig::f_y0_3 = model_cali_para_front->item(5,1)->text().toInt();
    AppConfig::f_x0_4 = model_cali_para_front->item(6,1)->text().toInt();
    AppConfig::f_y0_4 = model_cali_para_front->item(7,1)->text().toInt();

    AppConfig::f_x1_1 = model_cali_para_front->item(8,1)->text().toInt();
    AppConfig::f_y1_1 = model_cali_para_front->item(9,1)->text().toInt();
    AppConfig::f_x1_2 = model_cali_para_front->item(10,1)->text().toInt();
    AppConfig::f_y1_2 = model_cali_para_front->item(11,1)->text().toInt();
    AppConfig::f_x1_3 = model_cali_para_front->item(12,1)->text().toInt();
    AppConfig::f_y1_3 = model_cali_para_front->item(13,1)->text().toInt();
    AppConfig::f_x1_4 = model_cali_para_front->item(14,1)->text().toInt();
    AppConfig::f_y1_4 = model_cali_para_front->item(15,1)->text().toInt();

    //Back Calibrate
    AppConfig::b_x0_1 = model_cali_para_back->item(0,1)->text().toInt();
    AppConfig::b_y0_1 = model_cali_para_back->item(1,1)->text().toInt();
    AppConfig::b_x0_2 = model_cali_para_back->item(2,1)->text().toInt();
    AppConfig::b_y0_2 = model_cali_para_back->item(3,1)->text().toInt();
    AppConfig::b_x0_3 = model_cali_para_back->item(4,1)->text().toInt();
    AppConfig::b_y0_3 = model_cali_para_back->item(5,1)->text().toInt();
    AppConfig::b_x0_4 = model_cali_para_back->item(6,1)->text().toInt();
    AppConfig::b_y0_4 = model_cali_para_back->item(7,1)->text().toInt();

    AppConfig::b_x1_1 = model_cali_para_back->item(8,1)->text().toInt();
    AppConfig::b_y1_1 = model_cali_para_back->item(9,1)->text().toInt();
    AppConfig::b_x1_2 = model_cali_para_back->item(10,1)->text().toInt();
    AppConfig::b_y1_2 = model_cali_para_back->item(11,1)->text().toInt();
    AppConfig::b_x1_3 = model_cali_para_back->item(12,1)->text().toInt();
    AppConfig::b_y1_3 = model_cali_para_back->item(13,1)->text().toInt();
    AppConfig::b_x1_4 = model_cali_para_back->item(14,1)->text().toInt();
    AppConfig::b_y1_4 = model_cali_para_back->item(15,1)->text().toInt();
}

void frmView2::receivePoints(const QVector<QPoint> &points)
{
    MyApplication *pApp=(MyApplication *)qApp;

    int i,num;
    num=points.size();
    if(num==4)
    {
        for(i=0;i<4;i++)
        {
            x0_coord[i]=points[i].y();
            y0_coord[i]=points[i].x();

            PrintEvent(QString("拾取点 %1 坐标：x=%2 , y=%3").arg(i+1).arg(x0_coord[i]).arg(y0_coord[i]),true);
        }
        isGetPointOK=true;
    }
    else
    {
        isGetPointOK=false;

        PrintEvent(QString("要求标定形状为矩形，实际获取的点数=%1，请重新绘制").arg(num),true);
        ui->widget->clearAll();
    }
}

void frmView2::on_pushButton_clear_draw_clicked()
{
    ui->widget->clearAll();
}

void frmView2::on_pushButton_load_pic_front_clicked()
{
    MyApplication *pApp=(MyApplication *)qApp;
    HObject ho_image,ho_image_rot;

    QString strName_src(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_front.jpg"));
    QString strName_tar(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_front_rot.jpg"));

    ReadImage(&ho_image,strName_src.toStdString().c_str());
    WriteImage(ho_image, "jpeg", 0, strName_tar.toStdString().c_str());

    QString strName(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_front_rot.jpg"));

    //判断文件是否存在
    QFile file(strName);
    if(file.exists())
    {
        QPixmap pix;
        pix.load(strName);

        image_width=pix.width();
        image_height=pix.height();
        QString str=QString("机前图像宽度:%1,机前图像高度:%2").arg(image_width).arg(image_height);
        PrintEvent(str,true);

        //放缩系数
        width_scale_coff_front=widget_width*1.0/image_width;
        height_scale_coff_front=widget_height*1.0/image_height;
        str=QString("机前宽度缩放系数:%1,机前高度缩放系数:%2").arg(width_scale_coff_front).arg(height_scale_coff_front);
        PrintEvent(str,true);

        //Qt::IgnoreAspectRatio
        QPixmap scaledPix = pix.scaled(widget_width, widget_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        ui->widget->setAutoFillBackground(true);
        QPalette bgPalette = ui->widget->palette();
        bgPalette.setBrush(QPalette::Window,QBrush(scaledPix));
        ui->widget->setPalette(bgPalette);

        ui->widget->clearAll();

        QList<QPoint> listPoint;
        QPoint point;
        point.setX(AppConfig::f_y0_1);
        point.setY(AppConfig::f_x0_1);
        listPoint.append(point);
        point.setX(AppConfig::f_y0_2);
        point.setY(AppConfig::f_x0_2);
        listPoint.append(point);
        point.setX(AppConfig::f_y0_3);
        point.setY(AppConfig::f_x0_3);
        listPoint.append(point);
        point.setX(AppConfig::f_y0_4);
        point.setY(AppConfig::f_x0_4);
        listPoint.append(point);

        ui->widget->dispPoly(listPoint);
    }
    else
        PrintEvent("未找到标定图像文件: image_calibrate_front.jpg",true);
}

void frmView2::on_pushButton_load_pic_back_clicked()
{
    MyApplication *pApp=(MyApplication *)qApp;
    HObject ho_image,ho_image_rot;

    QString strName_src(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_back.jpg"));
    QString strName_tar(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_back_rot.jpg"));

    ReadImage(&ho_image,strName_src.toStdString().c_str());
    WriteImage(ho_image, "jpeg", 0, strName_tar.toStdString().c_str());

    QString strName(QDir(QCoreApplication::applicationDirPath()+"/"+AppConfig::calibrate_path).filePath("image_calibrate_back_rot.jpg"));

    //判断文件是否存在
    QFile file(strName);
    if(file.exists())
    {
        QPixmap pix;
        pix.load(strName);

        image_width=pix.width();
        image_height=pix.height();
        QString str=QString("机后图像宽度:%1,机后图像高度:%2").arg(image_width).arg(image_height);
        PrintEvent(str,true);

        //放缩系数
        width_scale_coff_back=widget_width*1.0/image_width;
        height_scale_coff_back=widget_height*1.0/image_height;
        str=QString("机后宽度缩放系数:%1,机后高度缩放系数:%2").arg(width_scale_coff_back).arg(height_scale_coff_back);
        PrintEvent(str,true);

        //Qt::KeepAspectRatio
        QPixmap scaledPix = pix.scaled(widget_width, widget_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        ui->widget->setAutoFillBackground(true);
        QPalette bgPalette = ui->widget->palette();
        bgPalette.setBrush(QPalette::Window,QBrush(scaledPix));
        ui->widget->setPalette(bgPalette);

        ui->widget->clearAll();

        QList<QPoint> listPoint;
        QPoint point;
        point.setX(AppConfig::b_y0_1);
        point.setY(AppConfig::b_x0_1);
        listPoint.append(point);
        point.setX(AppConfig::b_y0_2);
        point.setY(AppConfig::b_x0_2);
        listPoint.append(point);
        point.setX(AppConfig::b_y0_3);
        point.setY(AppConfig::b_x0_3);
        listPoint.append(point);
        point.setX(AppConfig::b_y0_4);
        point.setY(AppConfig::b_x0_4);
        listPoint.append(point);

        ui->widget->dispPoly(listPoint);
    }
    else
        PrintEvent("未找到标定图像文件: image_calibrate_back.jpg",true);
}

void frmView2::on_pushButton_get_point_clicked()
{
    ui->widget->getPoints();
}

void frmView2::on_pushButton_save_front_clicked()
{
    if(!isGetPointOK)
    {
        PrintEvent(u8"首先需要画出四边形，再点击[获取点四个顶点]按钮",true);
        return;
    }

    if(QMessageBox::Ok==QMessageBox::information(NULL, u8"注意", u8"确认覆盖相机标定参数吗？",QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel))
    {
        //先从tableview中读出数据
        readDataFromTableView();

        PrintEvent(u8"开始对机前相机标定参数赋值",true);
        AppConfig::f_x0_1=x0_coord[0];
        AppConfig::f_x0_2=x0_coord[1];
        AppConfig::f_x0_3=x0_coord[2];
        AppConfig::f_x0_4=x0_coord[3];

        AppConfig::f_y0_1=y0_coord[0];
        AppConfig::f_y0_2=y0_coord[1];
        AppConfig::f_y0_3=y0_coord[2];
        AppConfig::f_y0_4=y0_coord[3];

        AppConfig::f_calibrate_image_scale_width=width_scale_coff_front;
        AppConfig::f_calibrate_image_scale_height=height_scale_coff_front;

        PrintEvent(u8"开始存储机前相机标定参数...",true);
        AppConfig::writeConfig_front_data();

        PrintEvent(u8"机前标定参数存储完成，重新载入...",true);
        AppConfig::readConfig();
        loadCalibratePata();
    }
}

void frmView2::on_pushButton_save_back_clicked()
{
    if(!isGetPointOK)
    {
        PrintEvent(u8"首先需要画出四边形，再点击[获取点四个顶点]按钮",true);
        return;
    }

    if(QMessageBox::Ok==QMessageBox::information(NULL, u8"注意", u8"确认覆盖相机标定参数吗？",QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel))
    {
        //先从tableview中读出数据
        readDataFromTableView();

        PrintEvent(u8"开始对机后相机标定参数赋值",true);
        AppConfig::b_x0_1=x0_coord[0];
        AppConfig::b_x0_2=x0_coord[1];
        AppConfig::b_x0_3=x0_coord[2];
        AppConfig::b_x0_4=x0_coord[3];

        AppConfig::b_y0_1=y0_coord[0];
        AppConfig::b_y0_2=y0_coord[1];
        AppConfig::b_y0_3=y0_coord[2];
        AppConfig::b_y0_4=y0_coord[3];

        AppConfig::b_calibrate_image_scale_width=width_scale_coff_back;
        AppConfig::b_calibrate_image_scale_height=height_scale_coff_back;

        PrintEvent(u8"开始存储机后相机标定参数...",true);
        AppConfig::writeConfig_back_data();

        PrintEvent(u8"机后标定参数存储完成，重新载入...",true);
        AppConfig::readConfig();
        loadCalibratePata();
    }
}

