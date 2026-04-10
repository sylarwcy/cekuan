#include "myAngleProcess.h"
#include "MyApplication.h"

//日志
#include "QsLog.h"
#include "QsLogDest.h"
using namespace QsLogging;


myAngleProcess::myAngleProcess(QString strName)
{
    cameraName=strName;

    haveDataNum=0;
    haveDataTime_ms=0;

    noHaveDataNum=0;
    noHaveDataTime_ms=0;

    info.parallel_angle_len_max=0;
    info.vertical_angle_len_max=0;

    qTime_haveData.start();
    qTime_noHaveData.start();
}

myAngleProcess::~myAngleProcess()
{

}

void myAngleProcess::setParameters(int remain_ms,int protect_ms,int angle_critical,float use_fill_rate)
{
    remain_data_ms=remain_ms;
    protect_data_ms=protect_ms;
    recent_angle_critical=angle_critical;
    get_size_use_fill_rate=use_fill_rate;
}

void myAngleProcess::clearAllData()
{
    haveDataNum=0;
    haveDataTime_ms=0;

    info.parallel_angle_len_max=0;
    info.vertical_angle_len_max=0;

    info.data.clear();
}

//排序比较函数
bool fieldCompare(const struct SortAngleDiff &info1, const struct SortAngleDiff &info2)
{
    //按angle_diff升序排序
    return info1.angle_diff < info2.angle_diff;
}


void myAngleProcess::detectHavePlate(float row,float col,float rad,float length1,float length2,float rectRate)
{
    MyApplication *pApp=(MyApplication *)qApp;

    static int slab_rot_pos_ok_num;

    float angle=rad*180.0/3.14159265;
    struct EachDetectInfo eachInfo,lastEachInfo;
    float last_angle;
    int time_diff;
    struct SortAngleDiff myAngle;
    QList<struct SortAngleDiff> myList;

    noHaveDataNum=0;
    haveDataNum++;
    if(haveDataNum>3000)
        haveDataNum=3000;

    //到位上升沿处理
//    if(pApp->m_mqtt_l2_data.turn_ready==1)
//        slab_rot_pos_ok_num++;
//    else
//        slab_rot_pos_ok_num=0;

//    if(slab_rot_pos_ok_num>1000)
//        slab_rot_pos_ok_num=1000;

//    //到位上升沿清理队列，初始值为0
//    if(slab_rot_pos_ok_num==1)
//    {
//        clearAllData();
//        angle=0.0;
//        QLOG_INFO()<<QString("detectHavePlate->板坯位置准备好,初始角度=0,钢坯准备好=%1").arg(pApp->m_mqtt_l2_data.turn_ready);
//    }

//    if(info.data.size()>0)
//    {
//        lastEachInfo=info.data.last();
//        last_angle=lastEachInfo.angle;


//        //计算触发数据经历时间
//        haveDataTime_ms=qTime_haveData.elapsed();
//        time_diff=haveDataTime_ms-lastEachInfo.tri_ms;

//        //超出限制删除第一个元素
//        if(info.data.size()>ANGLE_LIST_MAX_NUM)
//            info.data.removeFirst();

//        //如果两次触发时间超出限制，清空数据
//        if(time_diff>remain_data_ms)
//        {
//            clearAllData();
//        }
//        else
//        {
//            eachInfo.row=row;
//            eachInfo.col=col;
//            eachInfo.tri_ms=qTime_haveData.elapsed();

//            //角度是否连续，如果角度连续使用角度判断，不连续选择最接近的角度
//            if(fabs(last_angle-angle)<recent_angle_critical)
//            {
//                eachInfo.angle=angle;
//                eachInfo.parallel_angle_len=length1;
//                eachInfo.vertical_angle_len=length2;
//            }
//            else if(fabs(last_angle-(angle+90.0))<recent_angle_critical)
//            {
//                eachInfo.angle=angle+90.0;
//                eachInfo.parallel_angle_len=length2;
//                eachInfo.vertical_angle_len=length1;
//            }
//            else if(fabs(last_angle-(angle-90.0))<recent_angle_critical)
//            {
//                eachInfo.angle=angle-90.0;
//                eachInfo.parallel_angle_len=length2;
//                eachInfo.vertical_angle_len=length1;
//            }
//            else if(fabs(last_angle-(angle+180.0))<recent_angle_critical)
//            {
//                eachInfo.angle=angle+180.0;
//                eachInfo.parallel_angle_len=length1;
//                eachInfo.vertical_angle_len=length2;
//            }
//            else if(fabs(last_angle-(angle-180.0))<recent_angle_critical)
//            {
//                eachInfo.angle=angle-180.0;
//                eachInfo.parallel_angle_len=length1;
//                eachInfo.vertical_angle_len=length2;
//            }
//            else
//            {
//                //保护时间内如果角度跳变>recent_angle_critical,认为当前角度有问题，用上次的角度
//                if(time_diff<protect_data_ms)
//                {
//                    eachInfo=lastEachInfo;
//                    QLOG_TRACE() << QString("%1@detectHavePlate函数->保护时间内触发,时间=%2,上次角度=%3,当前角度=%4")
//                                    .arg(cameraName)
//                                    .arg(time_diff)
//                                    .arg(last_angle)
//                                    .arg(angle);
//                }
//                else //超出保护时间取最接近的角度
//                {
//                    myAngle.idx=0;
//                    myAngle.angle_diff=fabs(last_angle-angle);
//                    myList.append(myAngle);
//                    myAngle.idx=1;
//                    myAngle.angle_diff=fabs(last_angle-(angle+90.0));
//                    myList.append(myAngle);
//                    myAngle.idx=2;
//                    myAngle.angle_diff=fabs(last_angle-(angle-90.0));
//                    myList.append(myAngle);
//                    myAngle.idx=3;
//                    myAngle.angle_diff=fabs(last_angle-(angle+180.0));
//                    myList.append(myAngle);
//                    myAngle.idx=4;
//                    myAngle.angle_diff=fabs(last_angle-(angle-180.0));
//                    myList.append(myAngle);

//                    //按an_diff升序排序
//                    qSort(myList.begin(), myList.end(), fieldCompare);

//                    int idx=myList.first().idx;
//                    if(idx==0)
//                    {
//                        eachInfo.angle=angle;
//                        eachInfo.parallel_angle_len=length1;
//                        eachInfo.vertical_angle_len=length2;
//                    }
//                    else if(idx==1)
//                    {
//                        eachInfo.angle=angle+90.0;
//                        eachInfo.parallel_angle_len=length2;
//                        eachInfo.vertical_angle_len=length1;
//                    }
//                    else if(idx==2)
//                    {
//                        eachInfo.angle=angle-90.0;
//                        eachInfo.parallel_angle_len=length2;
//                        eachInfo.vertical_angle_len=length1;
//                    }
//                    else if(idx==3)
//                    {
//                        eachInfo.angle=angle+180;
//                        eachInfo.parallel_angle_len=length1;
//                        eachInfo.vertical_angle_len=length2;
//                    }
//                    else
//                    {
//                        eachInfo.angle=angle-180;
//                        eachInfo.parallel_angle_len=length1;
//                        eachInfo.vertical_angle_len=length2;
//                    }

//                    QLOG_TRACE() << QString("%1@detectHavePlate函数->超出保护触发,时间=%2ms,上次角度=%3,本次使用角度=%4")
//                                    .arg(cameraName)
//                                    .arg(time_diff)
//                                    .arg(last_angle)
//                                    .arg( eachInfo.angle);
//                }
//            }

//            //数据加入list
//            info.data.append(eachInfo);

//            //进入区域，尺寸合适，并且矩形率足够，更新检测尺寸
//            if((eachInfo.col>100 && eachInfo.col<540) &&
//               (rectRate>get_size_use_fill_rate))
//            {
//                if(info.parallel_angle_len_max<eachInfo.parallel_angle_len)
//                    info.parallel_angle_len_max=eachInfo.parallel_angle_len;
//                if(info.vertical_angle_len_max<eachInfo.vertical_angle_len)
//                    info.vertical_angle_len_max=eachInfo.vertical_angle_len;

////                 QLOG_TRACE() << QString(u8"%1@detectHavePlate->更新尺寸:平行长度=%2,垂直长度=%3,矩形率=%4")
////                                 .arg(cameraName)
////                                 .arg(info.parallel_angle_len_max)
////                                 .arg(info.vertical_angle_len_max)
////                                 .arg(rectRate);

//            }
//        }
//    }

//    if(info.data.size()==0)
//    {
//        if(angle<-45.0)
//        {
//            eachInfo.angle=angle+90.0;
//            eachInfo.parallel_angle_len=length2;
//            eachInfo.vertical_angle_len=length1;
//        }
//        else if(angle>45.0)
//        {
//            eachInfo.angle=angle-90.0;
//            eachInfo.parallel_angle_len=length2;
//            eachInfo.vertical_angle_len=length1;
//        }
//        else
//        {
//            eachInfo.angle=angle;
//            eachInfo.parallel_angle_len=length1;
//            eachInfo.vertical_angle_len=length2;
//        }
//        eachInfo.row=row;
//        eachInfo.col=col;
//        eachInfo.tri_ms=0;

//        //数据加入list
//        info.data.append(eachInfo);

//        info.parallel_angle_len_max=0.0;
//        info.vertical_angle_len_max=0.0;

//        //启动计时
//        qTime_haveData.start();
//    }
}

void myAngleProcess::detectNoHavePlate()
{
    haveDataNum=0;

    noHaveDataNum++;
    if(noHaveDataNum>3000)
        noHaveDataNum=3000;

    if(noHaveDataNum==1)
        qTime_noHaveData.start();

    //计算无数据经历时间
    noHaveDataTime_ms=qTime_noHaveData.elapsed();

    //超过时长清除数据
    if(noHaveDataTime_ms>remain_data_ms)
    {
        if(info.data.size()>0)
        {
            clearAllData();
            QLOG_TRACE() << QString("%1@detectNoHavePlate函数->无数据时间=%2ms，超时清除跟踪数据").arg(cameraName).arg(noHaveDataTime_ms);
        }
    }
}
