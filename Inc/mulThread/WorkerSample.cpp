#include "WorkerSample.h"
#include <QDebug>
#include <QThread>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "MyApplication.h"
////////////////////////////////////////////////////////////////////
WorkerSample::WorkerSample(QObject *parent) : QObject(parent)
{
    MyApplication *pApp=(MyApplication *)qApp;

}

WorkerSample::~WorkerSample()
{
    MyApplication *pApp=(MyApplication *)qApp;

}

//单级文件夹是否存储，不存在则创建
bool WorkerSample::DirExist(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
        //存在当前文件夹
        return true;
    }
    else
    {
        //不存在则创建
        bool ok = dir.mkdir(fullPath); //只创建一级子目录，即必须保证上级目录存在
        return ok;
    }
}

//文件夹是否存储，不存在则创建，支持多级文件夹
bool WorkerSample::DirExistEx(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        //不存在当前目录，创建，可创建多级目录
        bool ok = dir.mkpath(fullPath);
        return ok;
    }
}

QString WorkerSample::SaveImgToDisk(bool isFront,QString ID,QString strFlag)
{
    MyApplication *pApp=(MyApplication *)qApp;

    static unsigned int saveProcessNum=0;
    static QString processSavePath;
    static int last_track_size=0;

    bool isOK;

    //获取时间
    QDateTime time = QDateTime::currentDateTime();
    int year = time.date().year();
    int month = time.date().month();
    int day = time.date().day();
    int hour = time.time().hour();
    int min = time.time().minute();
    int sec = time.time().second();
    int msec = time.time().msec();

    QString save_path=pApp->pNodeData->setting.save_path;
    QString saveIDPath=QString("%1/%2_%3_%4/%5").arg(save_path)
            .arg(year)
            .arg(month,2,10,QLatin1Char('0'))
            .arg(day,2,10,QLatin1Char('0'))
            .arg(ID);
    QString fileName=QString("%1_%2_%3_%4_%5_%6_%7_%8.jpg")
            .arg(ID)
            .arg(strFlag)
            .arg(year)
            .arg(month,2,10,QLatin1Char('0'))
            .arg(day,2,10,QLatin1Char('0'))
            .arg(hour,2,10,QLatin1Char('0'))
            .arg(min,2,10,QLatin1Char('0'))
            .arg(sec,3,10,QLatin1Char('0'));
    QString pathName=saveIDPath+"/"+fileName;

//    //不存在就创建
//    isOK=DirExistEx(saveIDPath);
//    if(isOK)
//    {
//        if(isFront)
//            WriteImage(pApp->pNodeData->ho_image_head, "jpeg", 0, pathName.toStdString().c_str());
//        else
//            WriteImage(pApp->pNodeData->ho_image_tail, "jpeg", 0, pathName.toStdString().c_str());
//    }

    return fileName;
}


void WorkerSample::processFrontImage(int row1,int row2)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeSetting *pNodeData=&pApp->pNodeData->setting;
    QString id;
    QString strName;

//    QLOG_INFO()<<QString(u8"开始处理机前图像,ID=%1,row1=%2,row2=%3").arg(pApp->head_ID).arg(row1).arg(row2);

//    //存储图片
//    if(pApp->head_ID!="")
//    {
//        id=pApp->head_ID;
//        strName=SaveImgToDisk(true,id,"head");
//    }
//    else
//    {
//        id="0000000000";
//        strName=SaveImgToDisk(true,id,"head");
//    }

//    //通知存储至数据库
//    emit sig_save_head_data(id,strName);

//    //完成图片存储
//    emit sig_finish_head_image();

}
void WorkerSample::processBackImage(int row1,int row2)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeSetting *pNodeData=&pApp->pNodeData->setting;
    QString id;
    QString strName;

//    QLOG_INFO()<<QString(u8"开始处理机后图像,ID=%1,row1=%2,row2=%3").arg(pApp->head_ID).arg(row1).arg(row2);

//    //存储
//    if(pApp->head_ID!="")
//    {
//        id=pApp->head_ID;
//        strName=SaveImgToDisk(false,id,"tail");
//    }
//    else
//    {
//        id="0000000000";
//        strName=SaveImgToDisk(false,id,"tail");
//    }

//    //通知存储至数据库
//    emit sig_save_tail_data(id,strName);

//    //完成图片存储
//    emit sig_finish_tail_image();
}
