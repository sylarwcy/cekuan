#ifndef _MY_ANGLE_PROCESS_H_
#define _MY_ANGLE_PROCESS_H_

#define ANGLE_LIST_MAX_NUM 2048

struct EachDetectInfo
{
    float row;
    float col;
    float angle;
    float parallel_angle_len;
    float vertical_angle_len;
    unsigned int tri_ms;
};

struct DetectInfo
{
    QString strID;
    float parallel_angle_len_max;
    float vertical_angle_len_max;

    QList<struct EachDetectInfo> data;
};

struct SortAngleDiff
{
    int idx;
    float angle_diff;
};

class myAngleProcess : public QObject
{
    Q_OBJECT

public:
    myAngleProcess(QString strName);
    ~myAngleProcess();
    void setParameters(int remain_ms,int protect_ms,int angle_critical,float use_fill_rate);

    void detectHavePlate(float row,float col,float rad,float length1,float length2,float rectRate);
    void detectNoHavePlate();

    void clearAllData();

public:
    struct DetectInfo info;
    QString cameraName;

    int remain_data_ms;
    int protect_data_ms;
    int recent_angle_critical;
    float get_size_use_fill_rate;


    QTime qTime_haveData;
    QTime qTime_noHaveData;

    unsigned int haveDataNum;
    unsigned int noHaveDataNum;

    unsigned int haveDataTime_ms;
    unsigned int noHaveDataTime_ms;
};


#endif
