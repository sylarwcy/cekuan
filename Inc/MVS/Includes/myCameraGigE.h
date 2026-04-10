#ifndef _MY_CAMERA_GIGE_H_
#define _MY_CAMERA_GIGE_H_

#include <QObject>

#include "MvCamera.h"
#include <windows.h>
#include "GlobalData.h"

#include <QList>

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

#include "opencv2/opencv.hpp"
using namespace cv;
//////////////////////////////////////////////////////////////////////////
class MyCameraGigE : public QObject
{
    Q_OBJECT
public:
    // MyCameraGigE(QString strIP,struct Camera_Init_Para &para,bool isFront);
    MyCameraGigE(QString strIP);
    ~MyCameraGigE();

    static bool HtupleIsEmpty(HTuple& value);
    void SetHandle(HTuple& ori, HTuple& pro);

    // int EnumCameraGigE();  //枚举相机
    int FindCameraGigE();
    bool CheckStatus();

    bool OpenCamera(void);
    int CloseCamera(void);

    bool StartGrabbing(void);
    void StopGrabbing(void);

    int SetTriggerMode(int triMode);
    int GetTriggerMode(void);
    void SoftTriggerOnce(void);

    int GetTriggerSource(void);
    int SetTriggerSource(void);

    int GetExposureTime(void);
    int SetExposureTime(float expTime);

    int GetGain(void);
    int SetGain(int gain);

    int GetFrameRate(void);
    int SetFrameRate(float frameRate);

    int GetMirror(bool &isWidthMirror,bool &isHeightMirror);
    int SetMirror(bool isWidthMirror,bool isHeightMirror);

    int GetAOI(void);
    int SetAOI(int width,int height);

    void GetParameter(void);
    void SetParameter(float expTime,int gain,float frameRate);

    int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType);

    static DWORD GrabThread(void* pUser);
    int GrabThreadProcess(void);
    // void GrabFrame();

    void ImgToHObject(MV_FRAME_OUT &stImageInfo, HObject &ho_image,int &nWidth,int &nHeight);
    bool ProjectiveTrans(HObject &ho_image,int width,int height,HObject &ho_imageCrop, int &trans_width, int &trans_height);

    void SaveImgToDisk(HObject &ho_image,QString ID,QString strFlag);
    bool SaveMp4Name(unsigned short ID,int width,int height);

    bool DirExist(QString fullPath);
    bool DirExistEx(QString fullPath);

    void processFrontImage(HObject &ho_image,int width,int height);
    void processBackImage(HObject &ho_image,int width,int height);
    static void __stdcall ReconnectDevice(unsigned int nMsgType, void* pUser);
    void reconnect();

public:
    QString cameraIP;
    int cameraIndex;

    int thread_cycle_ms;

    //利用OpenCV写每块钢坯的转钢视频
    VideoWriter writer;

    int frame_num_front;
    int frame_num_back;

public:
    bool isAtFront;

    // HWND  m_hwndDisplay;                        //显示句柄
    HTuple winHandle_ori;
    HTuple winHandle_pro;

    bool  m_bOpenDevice;                        //是否打开设备
    bool  m_bStartGrabbing;                     //是否开始抓图
    int   m_nTriggerMode;                       //触发模式
    int   m_nTriggerSource;                     //触发源

    CMvCamera* m_pcMyCamera;                    //CMyCamera封装了常用接口



    CRITICAL_SECTION        m_hSaveImageMux;
    unsigned char*          m_pSaveImageBuf;
    unsigned int    m_nSaveImageBufSize;
    MV_FRAME_OUT_INFO_EX m_stImageInfo;    

    void*           m_hGrabThread;              //取流线程句柄
    bool            m_bThreadState;

    bool m_bSoftWareTrigger;        //软触发源
    float m_dExposureTime;          //曝光时间
    int m_dGain;                    //增益
    float m_dFrameRate;             //帧率

    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    MV_CC_DEVICE_INFO       m_stDevInfo;
    QList<QString> strIPList;

signals:

};


#endif
