// ==================== [ WorkerCamera.cpp ] ====================
#include "WorkerCamera.h"
#include "HalconCpp.h"
#include <QDebug>
#include <QsLog.h>
#include <QSettings>
#include <QCoreApplication>

WorkerCamera::WorkerCamera(QObject *parent) : QObject(parent),
    m_hDevLeft(NULL), m_hDevRight(NULL),
    m_bCameraLeftOnline(false), m_bCameraRightOnline(false)
{
    m_pReconnectTimer = new QTimer(this);
    connect(m_pReconnectTimer, &QTimer::timeout, this, &WorkerCamera::onTryReconnect);
    connect(this, &WorkerCamera::signalExceptionFired, this, &WorkerCamera::onCameraDisconnected, Qt::QueuedConnection);

    m_camList.append(m_hDevLeft);
    m_camList.append(m_hDevRight);
}

WorkerCamera::~WorkerCamera() {
    m_pReconnectTimer->stop();
    stopGrabbing();
    if(m_hDevLeft) { MV_CC_DestroyHandle(m_hDevLeft); m_hDevLeft = NULL; }
    if(m_hDevRight) { MV_CC_DestroyHandle(m_hDevRight); m_hDevRight = NULL; }
}

bool WorkerCamera::initCameras(const QString& leftSN, const QString& rightSN,  const int imgWidth, const int imgHeight)
{
    m_leftSN = leftSN;
    m_rightSN = rightSN;
    m_imgWidth = imgWidth;
    m_imgHeight = imgHeight;

    QSettings settings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat);
    m_exposureTime_us = settings.value("CameraFront/front_expTime", 5000).toInt();

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);

    if (stDeviceList.nDeviceNum == 0) {
        emit signalCameraLog("错误: 网络中未发现任何千兆网相机！");
        return false;
    }

    int leftIndex = -1;
    int rightIndex = -1;

    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            QString currentSN = QString((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
            if (currentSN == m_leftSN) leftIndex = i;
            if (currentSN == m_rightSN) rightIndex = i;
        }
    }

    if (leftIndex == -1 || rightIndex == -1) {
        emit signalCameraLog(QString("严重错误: 找不到指定的 SN 号！配置左:%1 右:%2").arg(leftSN).arg(rightSN));
        return false;
    }

    MV_CC_CreateHandle(&m_hDevLeft, stDeviceList.pDeviceInfo[leftIndex]);
    MV_CC_OpenDevice(m_hDevLeft);

    MV_CC_CreateHandle(&m_hDevRight, stDeviceList.pDeviceInfo[rightIndex]);
    MV_CC_OpenDevice(m_hDevRight);

    MV_CC_SetIntValue(m_hDevLeft, "Height", m_imgHeight);
    MV_CC_SetIntValue(m_hDevRight, "Height", m_imgHeight);

    // 调用配置方法
    configureMasterSlave();

    m_ctxLeft.pThis = this; m_ctxLeft.camIndex = 0;
    MV_CC_RegisterImageCallBackEx(m_hDevLeft, ImageCallBackEx, &m_ctxLeft);
    MV_CC_RegisterExceptionCallBack(m_hDevLeft, ExceptionCallBack, &m_ctxLeft);

    m_ctxRight.pThis = this; m_ctxRight.camIndex = 1;
    MV_CC_RegisterImageCallBackEx(m_hDevRight, ImageCallBackEx, &m_ctxRight);
    MV_CC_RegisterExceptionCallBack(m_hDevRight, ExceptionCallBack, &m_ctxRight);

    m_bCameraLeftOnline = true;
    m_bCameraRightOnline = true;

    MVCC_INTVALUE stIntParam = {0};
    MV_CC_GetIntValue(m_hDevLeft, "Width", &stIntParam);
    int camWidth = stIntParam.nCurValue;

    emit sigImageReadyTOUI(camWidth, m_imgHeight);
    emit signalCameraLog(QString("初始化完毕: 画幅 %1 x %2").arg(camWidth).arg(m_imgHeight));
    return true;
}

// 🌟 初始化核心配置：纯粹主导主相机，不干涉从相机
void WorkerCamera::configureMasterSlave()
{
    if (m_hDevLeft) {
        // 强制主相机为内部时钟曝光模式，并开启行频使能
        MV_CC_SetEnumValueByString(m_hDevLeft, "ExposureAuto", "Off");
        MV_CC_SetEnumValueByString(m_hDevLeft, "ExposureMode", "Timed");
        MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);

        m_lineRate = (1000000.0f / m_exposureTime_us) * 0.95f;

        // 主相机双写突破锁死
        MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);
        MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
        MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
        MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);
    }

    // 🌟 彻底删除了从相机 (m_hDevRight) 的 ExposureTime 写入代码！
    // 它的曝光将完全由主相机发出的硬线脉冲宽度物理决定！
}

// ==========================================================
// 🌟 核心热更新：只调主相机，从相机自然跟随硬件脉冲同步变亮/变暗！
// ==========================================================
void WorkerCamera::onUpdateExposureTime(int expTime_us) {
    if (expTime_us < 10) return;

    m_exposureTime_us = expTime_us;
    float targetLineRate = (1000000.0f / m_exposureTime_us) * 0.95f;
    m_lineRate = targetLineRate;

    if (m_hDevLeft) {
        // 主相机双写机制
        MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);
        MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
        MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
        MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);
    }

    // 🌟 不干涉从相机

    QLOG_DEBUG() << "[手动介入] 主相机曝光时间已设为:" << m_exposureTime_us << "us, 行频锁定为:" << m_lineRate << "Hz。(从相机由硬线脉冲同步完成曝光)";
}

void WorkerCamera::onUpdateSpeedFromPLC(double speed_m_s)
{
    if (speed_m_s <= 0.0 || m_mmPerPixelX <= 0.0) return;

    m_currentSpeed_mm_s = speed_m_s * 1000.0;
    float targetLineRate = static_cast<float>(m_currentSpeed_mm_s / m_mmPerPixelX);

    // 护城河：硬件行频绝对不能突破当前主相机曝光时间的天花板限制！
    float MAX_LINE_RATE = (1000000.0f / m_exposureTime_us) * 0.95f;

    if (targetLineRate > MAX_LINE_RATE) {
        targetLineRate = MAX_LINE_RATE;
    }
    if (targetLineRate < 100.0f) targetLineRate = 100.0f;

    m_lineRate = targetLineRate;

    if (m_hDevLeft) {
        // 调速时同样只改变主相机的行频
        MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
    }
}

void WorkerCamera::startGrabbing()
{
    m_bufferMap.clear();
    // 硬件触发架构的铁律：必须先启动从相机(被动接收脉冲)，再启动主相机(发出脉冲)
    MV_CC_StartGrabbing(m_hDevRight);
    MV_CC_StartGrabbing(m_hDevLeft);
}

void WorkerCamera::stopGrabbing()
{
    MV_CC_StopGrabbing(m_hDevLeft);
    MV_CC_StopGrabbing(m_hDevRight);
}

void __stdcall WorkerCamera::ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if (pUser) {
        CamContext* ctx = (CamContext*)pUser;
        ctx->pThis->processFrame(pData, pFrameInfo, ctx->camIndex);
    }
}

void WorkerCamera::processFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, int camIndex)
{
    uint64_t frameID = pFrameInfo->nFrameNum;
    HalconCpp::HObject ho_Image;
    try {
        HalconCpp::GenImage1(&ho_Image, "byte", pFrameInfo->nWidth, pFrameInfo->nHeight, (Hlong)pData);
    } catch (const HalconCpp::HException& e) {
        qWarning() << "[WorkerCamera] GenImage1 异常丢帧:" << e.ErrorMessage().Text();
        return;
    }

    DualCameraChunk readyChunk;
    bool isPairReady = false;

    {
        QMutexLocker locker(&m_mutex);

        if (m_bufferMap.find(frameID) == m_bufferMap.end()) {
            DualCameraChunk newChunk;
            newChunk.frameID = frameID;
            newChunk.height = pFrameInfo->nHeight;
            m_bufferMap[frameID] = newChunk;
        }

        if (camIndex == 0) {
            m_bufferMap[frameID].imgLeft = ho_Image;
            m_bufferMap[frameID].hasLeft = true;
        } else {
            m_bufferMap[frameID].imgRight = ho_Image;
            m_bufferMap[frameID].hasRight = true;
        }

        if (m_bufferMap[frameID].hasLeft && m_bufferMap[frameID].hasRight) {
            readyChunk = m_bufferMap[frameID];
            isPairReady = true;
            m_bufferMap.erase(frameID);
        }

        if (m_bufferMap.size() > 10) {
            m_bufferMap.clear();
            qWarning() << "[WorkerCamera] 缓存池溢出，已强制清空孤儿帧！";
        }
    }

    if (isPairReady) {
        emit sigImageReadyToAlg(readyChunk);
        emit sigDisplayRawImage(readyChunk);
    }
}

void __stdcall WorkerCamera::ExceptionCallBack(unsigned int nMsgType, void* pUser)
{
    if (nMsgType == MV_EXCEPTION_DEV_DISCONNECT)
    {
        CamContext* ctx = (CamContext*)pUser;
        emit ctx->pThis->signalExceptionFired(ctx->camIndex);
    }
}

void WorkerCamera::onCameraDisconnected(int camIndex)
{
    QString camName = (camIndex == 0) ? "左相机(Master)" : "右相机(Slave)";
    emit signalCameraLog(QString("【严重警告】%1 发生物理断开！正在准备抢救...").arg(camName));

    if (camIndex == 0) {
        m_bCameraLeftOnline = false;
        if (m_hDevLeft) {
            MV_CC_StopGrabbing(m_hDevLeft);
            MV_CC_CloseDevice(m_hDevLeft);
            MV_CC_DestroyHandle(m_hDevLeft);
            m_hDevLeft = NULL;
        }
    } else {
        m_bCameraRightOnline = false;
        if (m_hDevRight) {
            MV_CC_StopGrabbing(m_hDevRight);
            MV_CC_CloseDevice(m_hDevRight);
            MV_CC_DestroyHandle(m_hDevRight);
            m_hDevRight = NULL;
        }
    }

    m_mutex.lock();
    m_bufferMap.clear();
    m_mutex.unlock();

    if (!m_pReconnectTimer->isActive()) {
        m_pReconnectTimer->start(3000);
    }
}

void WorkerCamera::onTryReconnect()
{
    if (m_bCameraLeftOnline && m_bCameraRightOnline) {
        m_pReconnectTimer->stop();
        emit signalCameraLog("【恢复】双目系统已全部重新上线！");
        return;
    }

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);

    if (!m_bCameraLeftOnline) {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            QString sn = QString((char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
            if (sn == m_leftSN) {
                MV_CC_CreateHandle(&m_hDevLeft, stDeviceList.pDeviceInfo[i]);
                if (MV_CC_OpenDevice(m_hDevLeft) == MV_OK) {
                    MV_CC_SetIntValue(m_hDevLeft, "Height", m_imgHeight);

                    // 主相机重连恢复参数
                    MV_CC_SetEnumValueByString(m_hDevLeft, "ExposureAuto", "Off");
                    MV_CC_SetEnumValueByString(m_hDevLeft, "ExposureMode", "Timed");
                    MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);
                    MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);
                    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
                    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
                    MV_CC_SetFloatValue(m_hDevLeft, "ExposureTime", (float)m_exposureTime_us);

                    MV_CC_RegisterImageCallBackEx(m_hDevLeft, ImageCallBackEx, &m_ctxLeft);
                    MV_CC_RegisterExceptionCallBack(m_hDevLeft, ExceptionCallBack, &m_ctxLeft);
                    MV_CC_StartGrabbing(m_hDevLeft);
                    m_bCameraLeftOnline = true;
                    emit signalCameraLog("左相机(Master)原地重连成功！");
                }
                break;
            }
        }
    }

    if (!m_bCameraRightOnline) {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            QString sn = QString((char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
            if (sn == m_rightSN) {
                MV_CC_CreateHandle(&m_hDevRight, stDeviceList.pDeviceInfo[i]);
                if (MV_CC_OpenDevice(m_hDevRight) == MV_OK) {
                    MV_CC_SetIntValue(m_hDevRight, "Height", m_imgHeight);

                    // 🌟 从相机重连不再写入曝光时间

                    MV_CC_RegisterImageCallBackEx(m_hDevRight, ImageCallBackEx, &m_ctxRight);
                    MV_CC_RegisterExceptionCallBack(m_hDevRight, ExceptionCallBack, &m_ctxRight);
                    MV_CC_StartGrabbing(m_hDevRight);
                    m_bCameraRightOnline = true;
                    emit signalCameraLog("右相机(Slave)原地重连成功！");
                }
                break;
            }
        }
    }
}

bool WorkerCamera::HtupleIsEmpty(HalconCpp::HTuple &value) {
    HalconCpp::HTuple length;
    TupleLength(value, &length);
    return length.I() == 0;
}