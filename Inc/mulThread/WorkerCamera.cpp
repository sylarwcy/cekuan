// ==================== [ WorkerCamera.cpp ] ====================
#include "WorkerCamera.h"
#include "HalconCpp.h"
#include <QDebug>
#include <QsLog.h>
#include <QSettings>
#include <QCoreApplication>
#include <algorithm>

namespace {

int SafeSetEnumString(void* handle, const char* node, const char* value) {
    if (!handle) return -1;
    return MV_CC_SetEnumValueByString(handle, node, value);
}

void SafePrepareManualTimedExposure(void* handle) {
    if (!handle) return;

    // Basler 通过 GenICam 暴露到 MVS 时，ExposureTime 只有在 Timed + 手动曝光模式下才一定生效。
    SafeSetEnumString(handle, "ExposureMode", "Timed");
    SafeSetEnumString(handle, "ExposureAuto", "Off");
    MV_CC_SetEnumValue(handle, "ExposureAuto", 0);
    SafeSetEnumString(handle, "ExposureTimeMode", "Common");
    SafeSetEnumString(handle, "ExposureTimeSelector", "Common");
}

// 兼容 Basler / 海康 / 大恒等不同厂家的 GenICam 曝光写入
int SafeSetExposureTime(void* handle, float exp_us) {
    if (!handle) return -1;
    int ret = MV_CC_SetFloatValue(handle, "ExposureTime", exp_us);       // GenICam SFNC 标准
    if (ret != MV_OK) ret = MV_CC_SetFloatValue(handle, "ExposureTimeAbs", exp_us); // Basler 旧节点
    if (ret != MV_OK) ret = MV_CC_SetIntValue(handle, "ExposureTimeRaw", static_cast<unsigned int>(exp_us));
    return ret;
}

// 兼容不同厂家/代际的线扫行频写入
int SafeSetLineRate(void* handle, float lineRate) {
    if (!handle) return -1;
    int ret = MV_CC_SetFloatValue(handle, "AcquisitionLineRate", lineRate);
    if (ret != MV_OK) ret = MV_CC_SetFloatValue(handle, "AcquisitionLineRateAbs", lineRate);
    return ret;
}

bool SafeGetExposureTime(void* handle, float& out_us) {
    if (!handle) return false;
    MVCC_FLOATVALUE f = {0};
    MVCC_INTVALUE i = {0};
    if (MV_CC_GetFloatValue(handle, "ExposureTime", &f) == MV_OK) { out_us = f.fCurValue; return true; }
    if (MV_CC_GetFloatValue(handle, "ExposureTimeAbs", &f) == MV_OK) { out_us = f.fCurValue; return true; }
    if (MV_CC_GetIntValue(handle, "ExposureTimeRaw", &i) == MV_OK) { out_us = static_cast<float>(i.nCurValue); return true; }
    return false;
}

bool SafeGetLineRate(void* handle, float& out_hz) {
    if (!handle) return false;
    MVCC_FLOATVALUE f = {0};
    if (MV_CC_GetFloatValue(handle, "AcquisitionLineRate", &f) == MV_OK) { out_hz = f.fCurValue; return true; }
    if (MV_CC_GetFloatValue(handle, "AcquisitionLineRateAbs", &f) == MV_OK) { out_hz = f.fCurValue; return true; }
    return false;
}

// 🌟 核心修复 1：严格的双向物理时序匹配逻辑 (仅限主相机使用)
void ApplyExposureToCamera(void* handle, const char* name, int exp_us, float& cachedLineRate) {
    if (!handle) return;

    SafePrepareManualTimedExposure(handle);

    // 目标行频：新曝光时间的 95%
    float targetLineRate = (1000000.0f / static_cast<float>(exp_us)) * 0.95f;

    // 获取相机当前的真实行频
    float currentLineRate = 0.0f;
    SafeGetLineRate(handle, currentLineRate);

    int retExp = -1, retRate = -1;

    // 必须严格遵守物理时序法则：曝光时间绝不能大于单行周期！
    if (targetLineRate < currentLineRate) {
        // 新行频更低（说明曝光变长了）：必须先降行频，腾出周期时间，然后再升曝光！
        retRate = SafeSetLineRate(handle, targetLineRate);
        retExp = SafeSetExposureTime(handle, static_cast<float>(exp_us));
    } else {
        // 新行频更高（说明曝光变短了）：必须先降曝光，腾出周期时间，然后再升行频！
        retExp = SafeSetExposureTime(handle, static_cast<float>(exp_us));
        retRate = SafeSetLineRate(handle, targetLineRate);
    }

    cachedLineRate = targetLineRate;

    // 验证结果并打印
    float actualExp = 0.0f;
    float actualLineRate = 0.0f;
    SafeGetExposureTime(handle, actualExp);
    SafeGetLineRate(handle, actualLineRate);

    QLOG_INFO() << "[曝光热更新]" << name
                << "目标:" << exp_us << "us /" << targetLineRate << "Hz |"
                << "实际生效:" << actualExp << "us /" << actualLineRate << "Hz |"
                << "写曝光RET:0x" << QString::number(retExp, 16)
                << "写行频RET:0x" << QString::number(retRate, 16);
}

} // namespace

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

// 🌟 初始化核心配置：主从策略分离
void WorkerCamera::configureMasterSlave()
{
    // 【1. 主相机】：全盘接管（曝光模式 + 曝光时间 + 极限行频）
    if (m_hDevLeft) {
        m_lineRate = (1000000.0f / static_cast<float>(m_exposureTime_us)) * 0.95f;
        SafePrepareManualTimedExposure(m_hDevLeft);

        int retRate = SafeSetLineRate(m_hDevLeft, m_lineRate);
        int retExp = SafeSetExposureTime(m_hDevLeft, static_cast<float>(m_exposureTime_us));

        float actualExp = 0.0f;
        float actualLineRate = 0.0f;
        SafeGetExposureTime(m_hDevLeft, actualExp);
        SafeGetLineRate(m_hDevLeft, actualLineRate);

        QLOG_INFO() << "[相机初始化] 主相机(Left/Master)"
                    << "目标:" << m_exposureTime_us << "us"
                    << "实际生效:" << actualExp << "us"
                    << "实际行频:" << actualLineRate << "Hz";
    }

    // 【2. 从相机】：佛系接管（绝对不碰其已固化的触发模式和行频，仅下发曝光时间）
    if (m_hDevRight) {
        int retExp = SafeSetExposureTime(m_hDevRight, static_cast<float>(m_exposureTime_us));

        float actualExp = 0.0f;
        SafeGetExposureTime(m_hDevRight, actualExp);

        QLOG_INFO() << "[相机初始化] 从相机(Right/Slave)"
                    << "目标曝光:" << m_exposureTime_us << "us"
                    << "实际生效:" << actualExp << "us"
                    << "写曝光RET:0x" << QString::number(retExp, 16);
    }
}

// ==========================================================
// 🌟 热更新下发：主从分别执行不同的安全下发策略
// ==========================================================
void WorkerCamera::onUpdateExposureTime(int expTime_us) {
    if (expTime_us < 10) return;

    m_exposureTime_us = expTime_us;

    // 【1. 主相机】：时序安全检查 + 行频动态同步
    if (m_hDevLeft) {
        ApplyExposureToCamera(m_hDevLeft, "Left/Master", expTime_us, m_lineRate);
    }

    // 【2. 从相机】：直接下发曝光时间，绝不动行频！
    if (m_hDevRight) {
        int retExp = SafeSetExposureTime(m_hDevRight, static_cast<float>(m_exposureTime_us));

        float actualExp = 0.0f;
        SafeGetExposureTime(m_hDevRight, actualExp);

        QLOG_INFO() << "[曝光热更新] Right/Slave 目标曝光:" << expTime_us
                    << "us | 实际生效:" << actualExp
                    << "us | 写曝光RET: 0x" << QString::number(retExp, 16);
    }

    emit signalCameraLog(QString("曝光时间已同步下发至主从双相机：%1 us").arg(expTime_us));
}

void WorkerCamera::onUpdateSpeedFromPLC(double speed_m_s)
{
    if (speed_m_s <= 0.0 || m_mmPerPixelX <= 0.0) return;

    m_currentSpeed_mm_s = speed_m_s * 1000.0;
    float targetLineRate = static_cast<float>(m_currentSpeed_mm_s / m_mmPerPixelX);

    float maxLineRateByExposure = (1000000.0f / static_cast<float>(m_exposureTime_us)) * 0.95f;
    if (targetLineRate > maxLineRateByExposure) targetLineRate = maxLineRateByExposure;
    if (targetLineRate < 100.0f) targetLineRate = 100.0f;

    m_lineRate = targetLineRate;

    // 测宽调速时，仅改变主相机（脉冲发生器）的行频
    if (m_hDevLeft)  SafeSetLineRate(m_hDevLeft,  m_lineRate);
}

void WorkerCamera::startGrabbing()
{
    m_bufferMap.clear();
    // 物理同步铁律：先开从相机的抓图（准备接客），再开主相机的抓图（开始发射脉冲）
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
        qWarning() << "[WorkerCamera] GenImage1 异常:" << e.ErrorMessage().Text();
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

        if (m_bufferMap.size() > 10) m_bufferMap.clear();
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
    if (camIndex == 0) {
        m_bCameraLeftOnline = false;
        if (m_hDevLeft) { MV_CC_StopGrabbing(m_hDevLeft); MV_CC_CloseDevice(m_hDevLeft); MV_CC_DestroyHandle(m_hDevLeft); m_hDevLeft = NULL; }
    } else {
        m_bCameraRightOnline = false;
        if (m_hDevRight) { MV_CC_StopGrabbing(m_hDevRight); MV_CC_CloseDevice(m_hDevRight); MV_CC_DestroyHandle(m_hDevRight); m_hDevRight = NULL; }
    }
    m_mutex.lock(); m_bufferMap.clear(); m_mutex.unlock();
    if (!m_pReconnectTimer->isActive()) m_pReconnectTimer->start(3000);
}

void WorkerCamera::onTryReconnect()
{
    if (m_bCameraLeftOnline && m_bCameraRightOnline) { m_pReconnectTimer->stop(); return; }

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
                    ApplyExposureToCamera(m_hDevLeft, "Left/Master_Recon", m_exposureTime_us, m_lineRate);

                    MV_CC_RegisterImageCallBackEx(m_hDevLeft, ImageCallBackEx, &m_ctxLeft);
                    MV_CC_RegisterExceptionCallBack(m_hDevLeft, ExceptionCallBack, &m_ctxLeft);
                    MV_CC_StartGrabbing(m_hDevLeft);
                    m_bCameraLeftOnline = true;
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

                    // 🌟 重连时对从相机也仅恢复曝光时间，不碰其他配置
                    SafeSetExposureTime(m_hDevRight, static_cast<float>(m_exposureTime_us));

                    MV_CC_RegisterImageCallBackEx(m_hDevRight, ImageCallBackEx, &m_ctxRight);
                    MV_CC_RegisterExceptionCallBack(m_hDevRight, ExceptionCallBack, &m_ctxRight);
                    MV_CC_StartGrabbing(m_hDevRight);
                    m_bCameraRightOnline = true;
                }
                break;
            }
        }
    }
}

bool WorkerCamera::HtupleIsEmpty(HalconCpp::HTuple &value) {
    HalconCpp::HTuple length; TupleLength(value, &length); return length.I() == 0;
}