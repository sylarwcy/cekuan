#include "workertrigger.h"
#include "MyApplication.h"
#include <QDir>
#include <QDateTime>
#pragma comment(lib, "winmm.lib")

WorkerTrigger::WorkerTrigger(QObject *parent)
    : QObject{parent} {
}

WorkerTrigger::~WorkerTrigger() {
}

void WorkerTrigger::init(const WorkStation_DATA &paramData) {
    m_location = paramData.m_location;
    m_serialNumber = paramData.m_serialNumber;
    m_loop_refresh_ms = paramData.m_loop_refresh_ms;
    m_mqtt_trigger_time = paramData.mqttTriggerTime;
    m_bufferPath = paramData.detBufferPath;
}

void WorkerTrigger::timerMsByCPU(double mSleepTime) {
    LARGE_INTEGER litmp;
    LONGLONG Qpart1, Qpart2;
    double dfMinus = 0, dfFreq = 0, dfTime = 0;

    //获得CPU计时器的时钟频率
    //取得高精度运行计数器的频率f,单位是每秒多少次（n/s）
    QueryPerformanceFrequency(&litmp);
    dfFreq = (double) litmp.QuadPart;

    //取得高精度运行计数器的数值
    QueryPerformanceCounter(&litmp);
    //开始计时
    Qpart1 = litmp.QuadPart;

    while ((mSleepTime - dfTime * 1000.000) > 0.0000001) {
        //取得高精度运行计数器的数值
        QueryPerformanceCounter(&litmp);
        //终止计时
        Qpart2 = litmp.QuadPart;

        //计算计数器值
        dfMinus = (double) (Qpart2 - Qpart1);
        //获得对应时间，单位为秒,可以乘1000000精确到微秒级（us）
        dfTime = dfMinus / dfFreq;
    }
    //qDebug()<<"ms"<<QString::number(dfTime*1000.00,'f',9);
}


void WorkerTrigger::on_doSomething() {
    MyApplication *pApp = (MyApplication *) qApp;
    static unsigned long time_num = 0;

    LARGE_INTEGER litmp;
    LONGLONG Qpart1, Qpart2, Useingtime;
    double dfMinus, dfFreq, dfTime;

    //心跳初始值
    pApp->m_mem_manager->GetToPLCData(m_serialNumber).beat_num = 0;

    int value = 0;
    while (!pApp->isStopTrigger) {
        //获得CPU计时器的时钟频率
        //取得高精度运行计数器的频率f,单位是每秒多少次（n/s）
        QueryPerformanceFrequency(&litmp);
        dfFreq = (double) litmp.QuadPart;
        //取得高精度运行计数器的数值
        QueryPerformanceCounter(&litmp);
        //开始计时
        Qpart1 = litmp.QuadPart;

        //PLC数据读
        pApp->m_mem_manager->ReadFromPLC();
        //各种触发功能编写
        // qDebug() << "beat_num: " << pApp->m_mem_manager->GetFromPLCData(m_serialNumber).beat_num;
        // qDebug() << "roller_conveyor_stop: " << pApp->m_mem_manager->GetFromPLCData(m_serialNumber).roller_conveyor_stop;
        // qDebug() << "roller_conveyor_speed: " << pApp->m_mem_manager->GetFromPLCData(m_serialNumber).roller_conveyor_speed;

        //界面触发显示
        if (time_num % pApp->pNodeData->setting.data_refresh_ms == 0) {
            if (!pApp->pNodeData->isDispProcessing) {
                //标志变量,由界面刷新函数执行完成复位
                pApp->pNodeData->isDispProcessing = true;

                emit sig_refreshDisplay();
            }
        }

        //mqtt数据处理
        /*if ((pApp->pNodeData->setting.mqtt_use == 1) && (time_num % m_mqtt_trigger_time == 0)) {
            if (!pApp->pNodeData->isMQTTProcessing) {
                //设置处理标记,由执行函数完成复位
                //在标志标量为true时，不让其它进程对原始图片/处理图片进行更新
                pApp->pNodeData->isMQTTProcessing = true;
                emit sig_mqtt_trigger();
            }
        }*/

        /*static int Scounter = 0, Ncounter = 0, SimgCount = 0, NimgCount = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).beat_num++;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_steel_plate = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).gundao_X_left = 0;
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).ID, "", 32);
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).img_name, "", 32);
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).recog_finished_flag = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_cam_broken = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak1 = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak2 = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak3 = 0;

        if (m_location == "hotCharging") {
            if (Scounter++ % 30 == 0) {
                std::string imgName, imgName_crop;
                HObject img, img_crop;
                ReadImage(&img, "test.jpg");
                ReadImage(&img_crop, "test_crop.jpg");

                //当热送用
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_steel_plate = 1;
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).gundao_X_left = 20000;
                memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).ID, "MN22222", 32);
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).recog_finished_flag = 1;
                imgName = "hotCharging_" + std::to_string(NimgCount) + ".jpg";
                imgName_crop = "hotCharging_" + std::to_string(NimgCount++) + "_crop.jpg";
                if (NimgCount>20) NimgCount = 1;

                memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).img_name, imgName.c_str(), 32);
                WriteImage(img, "jpeg", 0, ("\\\\192.168.0.195/Temp/" + imgName).c_str());
                WriteImage(img_crop, "jpeg", 0, ("\\\\192.168.0.195/Temp/" + imgName_crop).c_str());
                if (Scounter > 30) Scounter = 1;
            }
        }
        if (m_location == "stackerTable") {
            if (Ncounter++ % 30 == 0) {
                std::string imgName, imgName_crop;
                HObject img, img_crop;
                ReadImage(&img, "test.jpg");
                ReadImage(&img_crop, "test_crop.jpg");

                //当垛板台用
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_steel_plate = 1;
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).gundao_X_left = 10000;
                memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).ID, "MN11111", 32);
                pApp->m_mem_manager->GetToPLCData(m_serialNumber).recog_finished_flag = 1;
                imgName = "stackerTable_" + std::to_string(SimgCount) + ".jpg";
                imgName_crop = "stackerTable_" + std::to_string(SimgCount++) + "_crop.jpg";
                if (SimgCount>20) SimgCount = 1;


                memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).img_name, imgName.c_str(), 32);
                WriteImage(img, "jpeg", 0, ("\\\\192.168.0.195/Temp/" + imgName).c_str());
                WriteImage(img_crop, "jpeg", 0, ("\\\\192.168.0.195/Temp/" + imgName_crop).c_str());
                if (Ncounter > 30) Ncounter = 1;
            }
        }*/


        pApp->m_mem_manager->GetToPLCData(m_serialNumber).beat_num++;
        pApp->m_mem_manager->CommitWorkstationData(m_serialNumber);
        pApp->m_mem_manager->WriteToPLC();

        //******************************************************************
        QDateTime time = QDateTime::currentDateTime();
        int hour = time.time().hour();
        int min = time.time().minute();
        if (hour == 8 && min == 15) {
            clearCache(m_bufferPath+"/render",7);
            clearCache(m_bufferPath+"/src",7);
        }
        // m_bufferPath = "E:/test";
        // if (hour == 22 && min == 30 && m_location=="stagingNorth") {
        //     clearCache(m_bufferPath+"/render",7);
        //     clearCache(m_bufferPath+"/src",7);
        // }
        //******************************************************************

        //取得高精度运行计数器的数值
        QueryPerformanceCounter(&litmp);
        //终止计时
        Qpart2 = litmp.QuadPart;
        //计算计数器值
        dfMinus = (double) (Qpart2 - Qpart1);
        //获得对应时间，单位为秒,可以乘1000000精确到微秒级（us）
        dfTime = dfMinus / dfFreq;
        //换算为执行时间ms
        Useingtime = dfTime * 1000;

        //高精度延时,等待剩下的时间
        if (Useingtime < m_loop_refresh_ms)
            timerMsByCPU(m_loop_refresh_ms - Useingtime);

        //累计时间计数
        time_num += pApp->pNodeData->setting.sample_period;
    }
}

/**
 * @brief 删除指定路径下超过指定天数的文件
 * @param path 目标文件夹路径
 * @param days 保留的天数（例如输入7，则删除7天前的文件）
 */
void clearCache(const QString &path, int days)
{
    // 1. 检查路径是否存在
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Path does not exist:" << path;
        return;
    }

    // 2. 计算过期时间点
    // 当前时间减去指定天数，在这个时间点之前的文件即为过期
    QDateTime expiredTime = QDateTime::currentDateTime().addDays(-days);

    // 3. 获取文件夹内的所有文件信息
    // QDir::Files: 只列出文件，不列出文件夹
    // QDir::NoDotAndDotDot: 不列出 "." 和 ".."
    // QDir::Hidden: 如果需要删除隐藏文件，可以加上这个标志
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();

    // 4. 遍历并删除过期文件
    foreach (const QFileInfo &fileInfo, fileList) {
        if (fileInfo.lastModified() < expiredTime) {
            // 尝试删除文件
            if (!dir.remove(fileInfo.fileName()))
                qWarning() << "Failed to delete:" << fileInfo.fileName();
            // if (dir.remove(fileInfo.fileName())) {
            //     qDebug() << "Deleted:" << fileInfo.fileName();
            // } else {
            //     qWarning() << "Failed to delete:" << fileInfo.fileName();
            // }
        }
    }
}