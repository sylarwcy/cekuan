#include "myCameraGigE.h"
#include "appconfig.h"
#include "MyApplication.h"

#include <process.h>
#include <QString>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
/////////////////////////////////////////////////////////////////
///
// MV_CC_DEVICE_INFO_LIST MyCameraGigE::m_stDevList;
// QList<QString> MyCameraGigE::strIPList;

MyCameraGigE::MyCameraGigE(QString strIP) {
    cameraIP = strIP;

    m_bStartGrabbing = false;
    m_bOpenDevice = false;
    m_pcMyCamera = NULL;
    m_bSoftWareTrigger = false;
    m_pSaveImageBuf = NULL;

    m_hGrabThread = NULL;

    thread_cycle_ms = 0;

    frame_num_front = 0;
    frame_num_back = 0;

    InitializeCriticalSection(&m_hSaveImageMux);
    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
}

MyCameraGigE::~MyCameraGigE() {
    writer.release();

    CloseCamera();
    DeleteCriticalSection(&m_hSaveImageMux);
}

bool MyCameraGigE::HtupleIsEmpty(HTuple &value) {
    HTuple length;
    TupleLength(value, &length);

    return length.I() == 0;
}

void MyCameraGigE::SetHandle(HTuple &ori, HTuple &pro) {
    winHandle_ori = ori;
    winHandle_pro = pro;
}

// int MyCameraGigE::EnumCameraGigE() {
//     // if (strIPList.size()!=0)
//     //     return -1;
//
//     int nRet;
//     QString strIP, strMsg;
//
//     strIPList.clear();
//     memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
//
//     //枚举子网内所有设备
//     nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE, &m_stDevList);
//
//     if (MV_OK != nRet) {
//         QLOG_ERROR() << QString(u8"FindCameraGigE->CMvCamera::EnumDevices函数执行失败！");
//         return -1;
//     }
//
//     //在信息中发现指定IP索引
//     for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++) {
//         MV_CC_DEVICE_INFO *pDeviceInfo = m_stDevList.pDeviceInfo[i];
//         if (NULL == pDeviceInfo) {
//             continue;
//         }
//
//         if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
//             int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
//             int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
//             int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
//             int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
//
//             strIP = QString("%1.%2.%3.%4").arg(nIp1).arg(nIp2).arg(nIp3).arg(nIp4);
//             strIPList.append(strIP);
//
//             if (strcmp("", (LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0) {
//                 strMsg = QString("FindCameraGigE->[%1]GigE: %2  (%3.%4.%5.%6)").arg(i)
//                         .arg((LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName))
//                         .arg(nIp1)
//                         .arg(nIp2)
//                         .arg(nIp3)
//                         .arg(nIp4);
//                 QLOG_INFO() << strMsg;
//             } else {
//                 strMsg = QString("FindCameraGigE->[%1]GigE: %2  (%3.%4.%5.%6)").arg(i)
//                         .arg((LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chModelName))
//                         .arg(nIp1)
//                         .arg(nIp2)
//                         .arg(nIp3)
//                         .arg(nIp4);
//                 QLOG_INFO() << strMsg;
//             }
//         } else {
//             QLOG_INFO() << QString(u8"FindCameraGigE->发现非GigE相机，抛弃！");
//         }
//     }
//
//     return m_stDevList.nDeviceNum;
// }

int MyCameraGigE::FindCameraGigE() {
    int nRet;
    QString strIP, strMsg;

    cameraIndex = -1;

    strIPList.clear();
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    //枚举子网内所有设备
    nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE, &m_stDevList);

    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"FindCameraGigE->CMvCamera::EnumDevices函数执行失败！");
        return -1;
    }

    //在信息中发现指定IP索引
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO *pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo) {
            continue;
        }

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            strIP = QString("%1.%2.%3.%4").arg(nIp1).arg(nIp2).arg(nIp3).arg(nIp4);
            strIPList.append(strIP);

            if (strcmp("", (LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0) {
                strMsg = QString("FindCameraGigE->[%1]GigE: %2  (%3.%4.%5.%6)").arg(i)
                        .arg((LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName))
                        .arg(nIp1)
                        .arg(nIp2)
                        .arg(nIp3)
                        .arg(nIp4);
                QLOG_INFO() << strMsg;
            } else {
                strMsg = QString("FindCameraGigE->[%1]GigE: %2  (%3.%4.%5.%6)").arg(i)
                        .arg((LPCSTR) (pDeviceInfo->SpecialInfo.stGigEInfo.chModelName))
                        .arg(nIp1)
                        .arg(nIp2)
                        .arg(nIp3)
                        .arg(nIp4);
                QLOG_INFO() << strMsg;
            }
        } else {
            QLOG_INFO() << QString(u8"FindCameraGigE->发现非GigE相机，抛弃！");
        }
    }


    if (0 == m_stDevList.nDeviceNum) {
        QLOG_INFO() << QString(u8"FindCameraGigE->未发现GigE相机！");
        return -1;
    }

    //按IP确定索引
    for (int i = 0; i < strIPList.count(); i++) {
        if (strIPList[i] == cameraIP) {
            cameraIndex = i;
            QLOG_INFO() << QString(u8"FindCameraGigE->找到GigE相机，索引=%1").arg(i);
            return cameraIndex;
        }
    }

    QLOG_INFO() << QString(u8"FindCameraGigE->未找到IP=%1的GigE相机！").arg(cameraIP);
    return -1;
}

bool MyCameraGigE::OpenCamera() {
    if (NULL != m_pcMyCamera) {
        // QLOG_ERROR() << QString(u8"OpenCamera->m_pcMyCamera变量不为NULL");
        m_pcMyCamera->Close();
    }
    if (m_bOpenDevice) {
        QLOG_ERROR() << QString(u8"OpenCamera->m_bOpenDevice变量为TRUE");
        return false;
    }

    int nIndex = cameraIndex;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM)) {
        QLOG_ERROR() << QString(u8"OpenCamera->相机索引不在正常范围内,index=%1").arg(nIndex);
        return false;
    }

    //由设备信息创建设备实例
    if (NULL == m_stDevList.pDeviceInfo[nIndex]) {
        QLOG_ERROR() << QString(u8"OpenCamera->Device does not exist");
        return false;
    }

    m_pcMyCamera = new CMvCamera;
    if (NULL == m_pcMyCamera) {
        QLOG_ERROR() << QString(u8"OpenCamera->m_pcMyCamera变量建立失败");
        return false;
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet) {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        QLOG_ERROR() << QString(u8"OpenCamera->打开相机失败");
        return false;
    }

    //探测网络最佳包大小(只对GigE相机有效)
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) {
        unsigned int nPacketSize = 0;
        nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
        if (nRet == MV_OK) {
            nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize", nPacketSize);
            if (nRet != MV_OK) {
                QLOG_WARN() << QString(u8"OpenCamera->Set Packet Size fail!");
            }
        } else {
            QLOG_WARN() << QString(u8"OpenCamera->Get Packet Size fail!");
        }
    }

    m_bOpenDevice = TRUE;

    //重连回调m_stDevList m_stDevInfo
    m_pcMyCamera->RegisterExceptionCallBack(ReconnectDevice, this);

    return true;
    //获取参数
    //GetParameter();
}

bool MyCameraGigE::CheckStatus() {
    if (FALSE == m_bOpenDevice) {
        QLOG_ERROR() << QString(u8"StartGrabbing->m_bOpenDevice变量为FALSE");
        return false;
    }

    if (TRUE == m_bStartGrabbing) {
        QLOG_ERROR() << QString(u8"StartGrabbing->m_bStartGrabbing变量为TRUE");
        return false;
    }

    if (NULL == m_pcMyCamera) {
        QLOG_ERROR() << QString(u8"StartGrabbing->m_pcMyCamera变量为NULL");
        return false;
    }
    m_bThreadState = TRUE;

    int nRet = m_pcMyCamera->StartGrabbing();
    if (MV_OK != nRet) {
        m_bThreadState = FALSE;
        QLOG_ERROR() << QString("StartGrabbing->Start grabbing fail");
        return false;
    }

    return true;
}

int MyCameraGigE::CloseCamera(void) {
    m_bThreadState = FALSE;
    if (m_hGrabThread) {
        WaitForSingleObject(m_hGrabThread, INFINITE);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    if (m_pcMyCamera) {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    m_bStartGrabbing = FALSE;
    m_bOpenDevice = FALSE;

    if (m_pSaveImageBuf) {
        free(m_pSaveImageBuf);
        m_pSaveImageBuf = NULL;
    }
    m_nSaveImageBufSize = 0;

    QLOG_INFO() << QString(u8"CloseCamera->相机关闭完成！");

    return MV_OK;
}

bool MyCameraGigE::StartGrabbing(void) {
    MyApplication *pApp = (MyApplication *) qApp;

    //如果不是测试模式，做下面的检查
    if (pApp->pNodeData->setting.test_mode == 0) {
        if (FALSE == m_bOpenDevice) {
            QLOG_ERROR() << QString(u8"StartGrabbing->m_bOpenDevice变量为FALSE");
            return false;
        }

        if (TRUE == m_bStartGrabbing) {
            QLOG_ERROR() << QString(u8"StartGrabbing->m_bStartGrabbing变量为TRUE");
            return false;
        }

        if (NULL == m_pcMyCamera) {
            QLOG_ERROR() << QString(u8"StartGrabbing->m_pcMyCamera变量为NULL");
            return false;
        }
    }

    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    m_bThreadState = TRUE;
    unsigned int nThreadID = 0;
    m_hGrabThread = (void *) _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *)) MyCameraGigE::GrabThread,
                                            this, 0, &nThreadID);
    if (NULL == m_hGrabThread) {
        m_bThreadState = FALSE;
        QLOG_ERROR() << QString("StartGrabbing->Create thread fail");
        return false;
    }

    //如果不是测试模式，做下面的检查
    if (pApp->pNodeData->setting.test_mode == 0) {
        int nRet = m_pcMyCamera->StartGrabbing();
        if (MV_OK != nRet) {
            m_bThreadState = FALSE;
            QLOG_ERROR() << QString("StartGrabbing->Start grabbing fail");
            return false;
        }
    }

    m_bStartGrabbing = TRUE;

    return true;
}

// bool MyCameraGigE::Grabbing(void) {
//
// }

//结束采集
void MyCameraGigE::StopGrabbing(void) {
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing || NULL == m_pcMyCamera) {
        m_bThreadState = FALSE;
        m_bStartGrabbing = FALSE;

        QLOG_INFO() << QString(u8"StopGrabbing->相机未启动，不用停止，返回！");
        return;
    }

    m_bThreadState = FALSE;
    if (m_hGrabThread) {
        WaitForSingleObject(m_hGrabThread, INFINITE);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString("StopGrabbing->Stop grabbing fail");
        return;
    }
    m_bStartGrabbing = FALSE;
}


//设置触发模式
//触发模式: MV_TRIGGER_MODE_ON
//连续模式: MV_TRIGGER_MODE_OFF
int MyCameraGigE::SetTriggerMode(int triMode) {
    m_nTriggerMode = triMode;

    if (MV_TRIGGER_MODE_ON == triMode)
        QLOG_INFO() << QString(u8"SetTriggerMode->相机设置为触发模式");
    else
        QLOG_INFO() << QString(u8"SetTriggerMode->相机设置为连续模式");

    return m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
}

//获取触发模式
int MyCameraGigE::GetTriggerMode(void) {
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumValue);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetTriggerMode->获取触发模式失败");
        return nRet;
    }

    m_nTriggerMode = stEnumValue.nCurValue;

    if (MV_TRIGGER_MODE_ON == m_nTriggerMode)
        QLOG_INFO() << QString(u8"GetTriggerMode->相机处于触发模式");
    else
        QLOG_INFO() << QString(u8"GetTriggerMode->相机处于连续模式");

    return MV_OK;
}

//按下软触发一次
void MyCameraGigE::SoftTriggerOnce(void) {
    if (TRUE != m_bStartGrabbing) {
        QLOG_ERROR() << QString(u8"SoftTriggerOnce->m_bStartGrabbing变量为false");
        return;
    }

    m_pcMyCamera->CommandExecute("TriggerSoftware");
    QLOG_INFO() << QString(u8"oftTriggerOnce->相机设置为按下软触发一次");
}

//获取触发源
int MyCameraGigE::GetTriggerSource(void) {
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSource", &stEnumValue);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetTriggerSource->获取触发源失败");
        return nRet;
    }

    if ((unsigned int) MV_TRIGGER_SOURCE_SOFTWARE == stEnumValue.nCurValue) {
        m_bSoftWareTrigger = TRUE;
        QLOG_INFO() << QString(u8"GetTriggerSource->相机触发源为软触发");
    } else {
        m_bSoftWareTrigger = FALSE;
        QLOG_INFO() << QString(u8"GetTriggerSource->相机触发源为硬触发");
    }

    return MV_OK;
}

//设置触发源
//软触发 MV_TRIGGER_SOURCE_SOFTWARE
//硬触发 MV_TRIGGER_SOURCE_LINE0
int MyCameraGigE::SetTriggerSource(void) {
    int nRet = MV_OK;
    if (m_bSoftWareTrigger) {
        m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
        nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString("SetTriggerSource->Set Software Trigger Fail");
            return nRet;
        } else
            QLOG_INFO() << QString(u8"SetTriggerSource->相机触发源设置为软触发");
    } else {
        m_nTriggerSource = MV_TRIGGER_SOURCE_LINE0;
        nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString("SetTriggerSource->Set Hardware Trigger Fail");
            return nRet;
        } else
            QLOG_INFO() << QString(u8"SetTriggerSource->相机触发源设置为硬触发");
    }

    return nRet;
}

//获取曝光时间
int MyCameraGigE::GetExposureTime(void) {
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTimeAbs", &stFloatValue);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetExposureTime->获取曝光时间失败！");
        return nRet;
    }

    m_dExposureTime = stFloatValue.fCurValue;
    QLOG_INFO() << QString(u8"GetExposureTime->相机当前曝光时间：%1").arg(m_dExposureTime);

    return MV_OK;
}

//设置曝光时间
int MyCameraGigE::SetExposureTime(float expTime) {
    //调节这两个曝光模式，才能让曝光时间生效
    //    int nRet = m_pcMyCamera->SetEnumValue("ExposureMode", MV_EXPOSURE_MODE_TIMED);
    //    if (MV_OK != nRet)
    //    {
    //        QLOG_ERROR() << u8"SetExposureTime->SetEnumValue函数执行失败";
    //        return nRet;
    //    }

    m_pcMyCamera->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    QLOG_INFO() << QString(u8"SetExposureTime->相机曝光设置为：%1").arg(expTime);

    return m_pcMyCamera->SetFloatValue("ExposureTimeAbs", expTime);
}

//获取增益
int MyCameraGigE::GetGain(void) {
    //MVCC_FLOATVALUE stFloatValue = {0};
    MVCC_INTVALUE_EX stIntValue = {0};

    //int nRet = m_pcMyCamera->GetFloatValue("GainRaw", &stFloatValue);
    int nRet = m_pcMyCamera->GetIntValue("GainRaw", &stIntValue);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetGain->获取增益失败！");
        return nRet;
    }
    m_dGain = (int) stIntValue.nCurValue;
    QLOG_INFO() << QString(u8"GetGain->相机当前增益：%1").arg(m_dGain);
    return MV_OK;
}

//设置增益
int MyCameraGigE::SetGain(int gain) {
    //设置增益前先把自动增益关闭，失败无需返回
    m_pcMyCamera->SetEnumValue("GainAuto", 0);
    QLOG_INFO() << QString(u8"SetGain->相机增益设置为：%1").arg(gain);

    //return m_pcMyCamera->SetFloatValue("GainRaw", gain);
    return m_pcMyCamera->SetIntValue("GainRaw", gain);
}

//获取帧率
int MyCameraGigE::GetFrameRate(void) {
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("AcquisitionFrameRateAbs", &stFloatValue);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetFrameRate->获取帧率失败！");
        return nRet;
    }
    m_dFrameRate = stFloatValue.fCurValue;
    QLOG_INFO() << QString(u8"GetFrameRate->相机当前帧率：%1").arg(m_dFrameRate);

    return MV_OK;
}

//设置帧率
int MyCameraGigE::SetFrameRate(float frameRate) {
    int nRet = m_pcMyCamera->SetBoolValue("AcquisitionFrameRateEnable", true);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"SetFrameRate->SetBoolValue函数执行失败");
        return nRet;
    }

    QLOG_INFO() << QString(u8"SetFrameRate->相机帧率设置为：%1").arg(frameRate);
    return m_pcMyCamera->SetFloatValue("AcquisitionFrameRateAbs", frameRate);
}

int MyCameraGigE::GetMirror(bool &isWidthMirror, bool &isHeightMirror) {
    int nRet;
    bool isOK = true;

    nRet = m_pcMyCamera->GetBoolValue("ReverseX", &isWidthMirror);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"ReverseX->GetBoolValue函数执行失败");
        isOK = false;
    }

    nRet = m_pcMyCamera->GetBoolValue("ReverseY", &isHeightMirror);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"ReverseY->GetBoolValue函数执行失败");
        isOK = false;
    }

    return isOK;
}

int MyCameraGigE::SetMirror(bool isWidthMirror, bool isHeightMirror) {
    int nRet;
    bool isOK = true;

    if (isWidthMirror) {
        nRet = m_pcMyCamera->SetBoolValue("ReverseX", true);
        QLOG_INFO() << QString(u8"ReverseX->设置为:true");
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString(u8"ReverseX->SetBoolValue函数执行失败");
            isOK = false;
        }
    } else {
        nRet = m_pcMyCamera->SetBoolValue("ReverseX", false);
        QLOG_INFO() << QString(u8"ReverseX->设置为:false");
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString(u8"ReverseX->SetBoolValue函数执行失败");
            isOK = false;
        }
    }

    if (isHeightMirror) {
        nRet = m_pcMyCamera->SetBoolValue("ReverseY", true);
        QLOG_INFO() << QString(u8"ReverseY->设置为:true");
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString(u8"ReverseY->SetBoolValue函数执行失败");
            isOK = false;
        }
    } else {
        nRet = m_pcMyCamera->SetBoolValue("ReverseY", false);
        QLOG_INFO() << QString(u8"ReverseY->设置为:false");
        if (MV_OK != nRet) {
            QLOG_ERROR() << QString(u8"ReverseY->SetBoolValue函数执行失败");
            isOK = false;
        }
    }

    return isOK;
}

int MyCameraGigE::GetAOI(void) {
    MVCC_INTVALUE_EX stIntValue_width = {0};
    MVCC_INTVALUE_EX stIntValue_height = {0};

    int nRet = m_pcMyCamera->GetIntValue("Width", &stIntValue_width);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetAOI->获取AOI宽度失败！");
        return nRet;
    }
    nRet = m_pcMyCamera->GetIntValue("Height", &stIntValue_height);
    if (MV_OK != nRet) {
        QLOG_ERROR() << QString(u8"GetAOI->获取AOI高度失败！");
        return nRet;
    }

    int width = (int) stIntValue_width.nCurValue;
    int height = (int) stIntValue_height.nCurValue;

    QLOG_INFO() << QString(u8"GetAOI->相机当前宽与高度：%1*%2").arg(width).arg(height);

    return MV_OK;
}

//目前有问题，设定出错
int MyCameraGigE::SetAOI(int width, int height) {
    int nRet;

    //设置AOI先把CenterX,CenterY打开
    nRet = m_pcMyCamera->SetBoolValue("CenterX", true);
    nRet = m_pcMyCamera->SetBoolValue("CenterY", true);

    nRet = m_pcMyCamera->SetIntValue("Width", width);
    nRet = m_pcMyCamera->SetIntValue("Height", height);
    QLOG_INFO() << QString(u8"SetGain->相机宽与高度设置为：%1*%2").arg(width).arg(height);

    return nRet;
}

void MyCameraGigE::GetParameter(void) {
    int nRet = GetTriggerMode();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get Trigger Mode Fail");
    }

    nRet = GetExposureTime();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get Exposure Time Fail");
    }

    nRet = GetGain();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get Gain Fail");
    }

    nRet = GetFrameRate();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get Frame Rate Fail");
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get Trigger Source Fail");
    }

    nRet = GetAOI();
    if (nRet != MV_OK) {
        QLOG_ERROR() << QString("GetParameter->Get AOI Fail");
    }
}

//设置常用参数
void MyCameraGigE::SetParameter(float expTime, int gain, float frameRate) {
    bool bIsSetSucceed = true;
    int nRet;

    nRet = SetExposureTime(expTime);
    if (nRet != MV_OK)
        bIsSetSucceed = false;

    nRet = SetGain(gain);
    if (nRet != MV_OK)
        bIsSetSucceed = false;

    nRet = SetFrameRate(frameRate);
    if (nRet != MV_OK)
        bIsSetSucceed = false;

    if (true == bIsSetSucceed)
        QLOG_INFO() << QString("SetParameter->Set Parameter Succeed!");
    else
        QLOG_ERROR() << QString("SetParameter->Set Parameter Failed!");
}

//保存图片
//MV_Image_Bmp
//MV_Image_Jpeg
//MV_Image_Tif
//MV_Image_Png
int MyCameraGigE::SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType) {
    MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

    EnterCriticalSection(&m_hSaveImageMux);
    if (m_pSaveImageBuf == NULL || m_stImageInfo.enPixelType == 0) {
        LeaveCriticalSection(&m_hSaveImageMux);
        return MV_E_NODATA;
    }

    stSaveFileParam.enImageType = enSaveImageType; //需要保存的图像类型
    stSaveFileParam.enPixelType = m_stImageInfo.enPixelType; //相机对应的像素格式
    stSaveFileParam.nWidth = m_stImageInfo.nWidth; //相机对应的宽
    stSaveFileParam.nHeight = m_stImageInfo.nHeight; //相机对应的高
    stSaveFileParam.nDataLen = m_stImageInfo.nFrameLen;
    stSaveFileParam.pData = m_pSaveImageBuf;
    stSaveFileParam.iMethodValue = 0;

    // ch:jpg图像质量范围为(50-99], png图像质量范围为[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
    if (MV_Image_Bmp == stSaveFileParam.enImageType) {
        sprintf(stSaveFileParam.pImagePath, "Image_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight,
                m_stImageInfo.nFrameNum);
    } else if (MV_Image_Jpeg == stSaveFileParam.enImageType) {
        stSaveFileParam.nQuality = 80;
        sprintf(stSaveFileParam.pImagePath, "Image_w%d_h%d_fn%03d.jpg", stSaveFileParam.nWidth, stSaveFileParam.nHeight,
                m_stImageInfo.nFrameNum);
    } else if (MV_Image_Tif == stSaveFileParam.enImageType) {
        sprintf(stSaveFileParam.pImagePath, "Image_w%d_h%d_fn%03d.tif", stSaveFileParam.nWidth, stSaveFileParam.nHeight,
                m_stImageInfo.nFrameNum);
    } else if (MV_Image_Png == stSaveFileParam.enImageType) {
        stSaveFileParam.nQuality = 8;
        sprintf(stSaveFileParam.pImagePath, "Image_w%d_h%d_fn%03d.png", stSaveFileParam.nWidth, stSaveFileParam.nHeight,
                m_stImageInfo.nFrameNum);
    }

    int nRet = m_pcMyCamera->SaveImageToFile(&stSaveFileParam);
    LeaveCriticalSection(&m_hSaveImageMux);

    return nRet;
}

DWORD MyCameraGigE::GrabThread(void *pUser) {
    if (pUser) {
        MyCameraGigE *pCam = (MyCameraGigE *) pUser;
        pCam->GrabThreadProcess();
        return 0;
    }
    return -1;
}

// void MyCameraGigE::GrabFrame() {
//     MyApplication *pApp = (MyApplication *) qApp;
//
//     if (!CheckStatus()) return;
//
//     QString str;
//     QTime cycle_time;
//
//     MV_FRAME_OUT stImageInfo = {0};
//     int nRet = MV_OK;
//
//     HObject ho_image;
//     HObject ho_imageCrop;
//     HObject ho_imageScale;
//     HObject ho_ImageScaleMax;
//     HTuple hv_height, hv_width;
//     int nHeight, nWidth;
//
//     while (m_bThreadState) {
//         // while (!m_pcMyCamera->IsDeviceConnected()) {
//         //     QLOG_INFO()<< "相机断连，等待重连...";
//         //     QThread::msleep(500);
//         // }
//
//         cycle_time.start();
//
//         //非测试模式处理
//         nRet = m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
//
//         if (nRet == MV_OK) {
//             //用于保存图片
//             EnterCriticalSection(&m_hSaveImageMux);
//             if (NULL == m_pSaveImageBuf || stImageInfo.stFrameInfo.nFrameLen > m_nSaveImageBufSize) {
//                 if (m_pSaveImageBuf) {
//                     free(m_pSaveImageBuf);
//                     m_pSaveImageBuf = NULL;
//                 }
//
//                 m_pSaveImageBuf = (unsigned char *) malloc(
//                     sizeof(unsigned char) * stImageInfo.stFrameInfo.nFrameLen);
//                 if (m_pSaveImageBuf == NULL) {
//                     LeaveCriticalSection(&m_hSaveImageMux);
//                     return;
//                 }
//                 m_nSaveImageBufSize = stImageInfo.stFrameInfo.nFrameLen;
//             }
//             memcpy(m_pSaveImageBuf, stImageInfo.pBufAddr, stImageInfo.stFrameInfo.nFrameLen);
//             memcpy(&m_stImageInfo, &(stImageInfo.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
//             LeaveCriticalSection(&m_hSaveImageMux);
//
//             //转换为halcon格式,返回ho_image,nWidth,nHeight
//             ImgToHObject(stImageInfo, ho_image, nWidth, nHeight);
//
//             //重新获取图像尺寸
//             GetImageSize(ho_image, &hv_width, &hv_height);
//             nWidth = hv_width[0];
//             nHeight = hv_height[0];
//
//
//             //显示原始图像
//             if (!HtupleIsEmpty(winHandle_ori)) {
//                 if (HDevWindowStack::IsOpen()) {
//                     SetPart(winHandle_ori, 0, 0, nHeight, nWidth);
//                     DispObj(ho_image, winHandle_ori);
//                 }
//             }
//             //显示图像
//             if (!HtupleIsEmpty(winHandle_pro)) {
//                 HDevWindowStack::SetActive(winHandle_pro);
//                 if (HDevWindowStack::IsOpen()) {
//                     //解决刷新问题
//                     SetSystem("flush_graphic", "false");
//                     SetPart(winHandle_pro, 0, 0, nHeight, nWidth);
//                     DispObj(ho_image, winHandle_pro);
//                     SetSystem("flush_graphic", "true");
//                 }
//             }
//             //图像处理
//             // processFrontImage(ho_imageCrop, trans_width, trans_height);
//             m_pcMyCamera->FreeImageBuffer(&stImageInfo); //释放本次缓存
//         } else {
//             if (MV_TRIGGER_MODE_ON == m_nTriggerMode)
//                 Sleep(5);
//
//             QLOG_ERROR() << QString("m_pcMyCamera->GetImageBuffer函数发生错误,等待中...");
//         }
//
//         //等待时间
//         timeBeginPeriod(1);
//         Sleep(pApp->pNodeData->setting.sample_period);
//         timeEndPeriod(1);
//
//         //总循环时间计算
//         thread_cycle_ms = cycle_time.elapsed();
//     }
// }

void __stdcall MyCameraGigE::ReconnectDevice(unsigned int nMsgType, void *pUser) {
    if (nMsgType == MV_EXCEPTION_DEV_DISCONNECT) {
        MyCameraGigE *pThis = static_cast<MyCameraGigE *>(pUser);
        QLOG_INFO() << QString(u8"IP=%1的GigE相机重连！").arg(pThis->cameraIP);

        pThis->reconnect();
    }
}

void MyCameraGigE::reconnect() {
    if (m_bOpenDevice)
        m_pcMyCamera->Close();
    while (true) {
        FindCameraGigE();
        bool openRes = OpenCamera();
        if (openRes) {
            m_pcMyCamera->RegisterExceptionCallBack(MyCameraGigE::ReconnectDevice, this);
            m_pcMyCamera->StartGrabbing();
            break;
        }else {
            // m_pcMyCamera->Close();
            m_bOpenDevice = false;
            QThread::msleep(100);
        }
    }
}
