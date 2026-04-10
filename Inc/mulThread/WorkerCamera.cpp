#include "WorkerCamera.h"
#include "HalconCpp.h"
#include <QDebug>

WorkerCamera::WorkerCamera(QObject *parent) : QObject(parent),
    m_hDevLeft(NULL), m_hDevRight(NULL),
    m_bCameraLeftOnline(false), m_bCameraRightOnline(false) // [新增]
{
    // [新增] 初始化重连定时器
    m_pReconnectTimer = new QTimer(this);
    connect(m_pReconnectTimer, &QTimer::timeout, this, &WorkerCamera::onTryReconnect);

    // [新增] 将底层异常信号绑定到本类的处理槽上 (跨线程安全)
    connect(this, &WorkerCamera::signalExceptionFired, this, &WorkerCamera::onCameraDisconnected, Qt::QueuedConnection);
}

WorkerCamera::~WorkerCamera() {
    m_pReconnectTimer->stop(); // [新增]
    stopGrabbing();
    if(m_hDevLeft) { MV_CC_DestroyHandle(m_hDevLeft); m_hDevLeft = NULL; }
    if(m_hDevRight) { MV_CC_DestroyHandle(m_hDevRight); m_hDevRight = NULL; }
}

bool WorkerCamera::initCameras(const QString& leftSN, const QString& rightSN, const int height)
{
    // 保存到成员变量，留给断线重连使用
    m_leftSN = leftSN;
    m_rightSN = rightSN;
    m_cam_img_height = height;

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);

    if (stDeviceList.nDeviceNum == 0) {
        emit signalCameraLog("错误: 网络中未发现任何千兆网相机！");
        return false;
    }

    int leftIndex = -1;
    int rightIndex = -1;

    // 遍历所有在线的相机，通过出厂 SN 号进行精准身份确认
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            // 解析出硬件 SN 号
            QString currentSN = QString((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);

            if (currentSN == m_leftSN) leftIndex = i;
            if (currentSN == m_rightSN) rightIndex = i;
        }
    }

    if (leftIndex == -1 || rightIndex == -1) {
        emit signalCameraLog(QString("严重错误: 找不到指定的 SN 号！配置左:%1 右:%2").arg(leftSN).arg(rightSN));
        return false;
    }

    // 1. 精准创建句柄并打开
    MV_CC_CreateHandle(&m_hDevLeft, stDeviceList.pDeviceInfo[leftIndex]);
    MV_CC_OpenDevice(m_hDevLeft);

    MV_CC_CreateHandle(&m_hDevRight, stDeviceList.pDeviceInfo[rightIndex]);
    MV_CC_OpenDevice(m_hDevRight);

    // 2. 让相机每扫 1000 行，发一帧
    MV_CC_SetIntValue(m_hDevLeft, "Height", m_cam_img_height);
    MV_CC_SetIntValue(m_hDevRight, "Height", m_cam_img_height);

    // 3. 配置硬件主从与行频
    configureMasterSlave();

    // 4. 注册底层的异步回调与异常回调
    m_ctxLeft.pThis = this; m_ctxLeft.camIndex = 0;
    MV_CC_RegisterImageCallBackEx(m_hDevLeft, ImageCallBackEx, &m_ctxLeft);
    MV_CC_RegisterExceptionCallBack(m_hDevLeft, ExceptionCallBack, &m_ctxLeft);

    m_ctxRight.pThis = this; m_ctxRight.camIndex = 1;
    MV_CC_RegisterImageCallBackEx(m_hDevRight, ImageCallBackEx, &m_ctxRight);
    MV_CC_RegisterExceptionCallBack(m_hDevRight, ExceptionCallBack, &m_ctxRight);

    m_bCameraLeftOnline = true;
    m_bCameraRightOnline = true;

    emit signalCameraLog("双线阵相机 SN 匹配成功，初始化完毕！");
    return true;
}

void WorkerCamera::configureMasterSlave()
{
    // 【左相机 Master】：关闭软触发，开启内部行频，配置引脚输出脉冲
    MV_CC_SetEnumValueByString(m_hDevLeft, "TriggerMode", "Off");
    MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);
    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", 500.0f); // 默认行频
    MV_CC_SetEnumValueByString(m_hDevLeft, "LineSelector", "Line1");
    MV_CC_SetEnumValueByString(m_hDevLeft, "LineMode", "Output");
    MV_CC_SetEnumValueByString(m_hDevLeft, "LineSource", "ExposureActive");

    // 【右相机 Slave】：开启硬件外部触发，紧盯 Line1
    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerSelector", "LineStart");
    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerMode", "On");
    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerSource", "Line1");
    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerActivation", "RisingEdge");
}

void WorkerCamera::onUpdateSpeedFromPLC(double speed_m_s)
{
    float targetLineRate = speed_m_s * 1000.0f;
    if (targetLineRate < 100.0f) targetLineRate = 100.0f;
    // 动态调速：允许在拉流时直接写入！
    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", targetLineRate);
}

void WorkerCamera::startGrabbing()
{
    m_bufferMap.clear();
    MV_CC_StartGrabbing(m_hDevRight); //先启动从相机
    MV_CC_StartGrabbing(m_hDevLeft);  //后启动主相机
}

void WorkerCamera::stopGrabbing()
{
    MV_CC_StopGrabbing(m_hDevLeft);
    MV_CC_StopGrabbing(m_hDevRight);
}

// 静态回调入口
void __stdcall WorkerCamera::ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if (pUser) {
        CamContext* ctx = (CamContext*)pUser;
        ctx->pThis->processFrame(pData, pFrameInfo, ctx->camIndex);
    }
}
// ====== 核心：死锁对齐与 Halcon 图像生成 ======
void WorkerCamera::processFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, int camIndex)
{
    // 1. 获取网络包里的宏观帧号
    uint64_t frameID = pFrameInfo->nFrameNum;

    // 2. 直接从海康的裸内存生成 Halcon 图像对象！
    // GenImage1 会在 Halcon 内存池中深拷贝这块内存，极度安全，不怕海康释放 pData
    HalconCpp::HObject ho_Image;
    HalconCpp::GenImage1(&ho_Image, "byte", pFrameInfo->nWidth, pFrameInfo->nHeight, (Hlong)pData);

    // 3. 进入帧配对池
    m_mutex.lock();
    if (m_bufferMap.find(frameID) == m_bufferMap.end()) {
        DualCameraChunk newChunk;
        newChunk.frameID = frameID;
        newChunk.height = pFrameInfo->nHeight;
        m_bufferMap[frameID] = newChunk;
    }

    // 4. 将 HObject 赋值进缓存池，并改变到达标志位
    if (camIndex == 0) {
        m_bufferMap[frameID].imgLeft = ho_Image;
        m_bufferMap[frameID].hasLeft = true;
    } else {
        m_bufferMap[frameID].imgRight = ho_Image;
        m_bufferMap[frameID].hasRight = true;
    }

    // 5. 检查左右兄弟是否都到齐了？
    if (m_bufferMap[frameID].hasLeft && m_bufferMap[frameID].hasRight) {

        // 完美对齐！发射包含了 HObject 的包裹给算法线程
        emit signalDualChunkReady(m_bufferMap[frameID]);

        // 从 Map 中抹除，HObject 自身的智能指针会自动管理生命周期
        m_bufferMap.erase(frameID);
    }

    // 防爆池机制：超过 10 帧未配对成功的孤儿，直接清空
    if (m_bufferMap.size() > 10) {
        m_bufferMap.clear();
    }
    
    m_mutex.unlock();
}

// ==============================================================
// 工业级断线重连模块
// ==============================================================

// 1. 海康 SDK 底层异常回调 (注意：这里是在海康的内部线程，绝对不能在这里调用海康的销毁 API)
void __stdcall WorkerCamera::ExceptionCallBack(unsigned int nMsgType, void* pUser)
{
    if (nMsgType == MV_EXCEPTION_DEV_DISCONNECT) // 确认是设备断开异常
    {
        CamContext* ctx = (CamContext*)pUser;
        // 发送异步信号，让 Qt 自己的线程去处理善后，防止 SDK 内部死锁
        emit ctx->pThis->signalExceptionFired(ctx->camIndex);
    }
}

// 2. 接收到断线信号，进行拔管和善后
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

    // 只要有任何一台掉线了，我们就把拼图缓存池清空，防止死锁累积旧数据
    m_mutex.lock();
    m_bufferMap.clear();
    m_mutex.unlock();

    // 启动重连定时器 (每 3 秒尝试一次)
    if (!m_pReconnectTimer->isActive()) {
        m_pReconnectTimer->start(3000);
    }
}

// 3. 定时器驱动：尝试原地复活
void WorkerCamera::onTryReconnect()
{
    if (m_bCameraLeftOnline && m_bCameraRightOnline) {
        m_pReconnectTimer->stop();
        emit signalCameraLog("【恢复】双目系统已全部重新上线！");
        return;
    }

    // 重新枚举设备
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);

    // 寻找掉线的左相机
    if (!m_bCameraLeftOnline) {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            QString sn = QString((char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
            if (sn == m_leftSN) {
                MV_CC_CreateHandle(&m_hDevLeft, stDeviceList.pDeviceInfo[i]);
                if (MV_CC_OpenDevice(m_hDevLeft) == MV_OK) {
                    MV_CC_SetIntValue(m_hDevLeft, "Height", 1000);
                    MV_CC_SetEnumValueByString(m_hDevLeft, "TriggerMode", "Off");
                    MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);
                    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", 1000.0f);
                    MV_CC_SetEnumValueByString(m_hDevLeft, "LineSelector", "Line2");
                    MV_CC_SetEnumValueByString(m_hDevLeft, "LineMode", "Output");
                    MV_CC_SetEnumValueByString(m_hDevLeft, "LineSource", "ExposureActive");

                    MV_CC_RegisterImageCallBackEx(m_hDevLeft, ImageCallBackEx, &m_ctxLeft);
                    MV_CC_RegisterExceptionCallBack(m_hDevLeft, ExceptionCallBack, &m_ctxLeft);
                    MV_CC_StartGrabbing(m_hDevLeft);
                    m_bCameraLeftOnline = true;
                    emit signalCameraLog("左相机(Master)原地重连成功！");
                }
                break; // 找到了就跳出循环
            }
        }
    }

    // 寻找掉线的右相机
    if (!m_bCameraRightOnline) {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            QString sn = QString((char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
            if (sn == m_rightSN) {
                MV_CC_CreateHandle(&m_hDevRight, stDeviceList.pDeviceInfo[i]);
                if (MV_CC_OpenDevice(m_hDevRight) == MV_OK) {
                    MV_CC_SetIntValue(m_hDevRight, "Height", 1000);
                    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerSelector", "LineStart");
                    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerMode", "On");
                    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerSource", "Line1");
                    MV_CC_SetEnumValueByString(m_hDevRight, "TriggerActivation", "RisingEdge");

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
