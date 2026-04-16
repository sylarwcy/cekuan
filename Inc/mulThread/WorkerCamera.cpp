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

    m_camList.append(m_hDevLeft);
    m_camList.append(m_hDevRight);
}

WorkerCamera::~WorkerCamera() {
    m_pReconnectTimer->stop(); // [新增]
    stopGrabbing();
    if(m_hDevLeft) { MV_CC_DestroyHandle(m_hDevLeft); m_hDevLeft = NULL; }
    if(m_hDevRight) { MV_CC_DestroyHandle(m_hDevRight); m_hDevRight = NULL; }
}

// void WorkerCamera::SetHandle(HalconCpp::HTuple &ori, HalconCpp::HTuple &pro) {
//     m_winHandle_ori = ori;
//     m_winHandle_pro = pro;
// }

bool WorkerCamera::initCameras(const QString& leftSN, const QString& rightSN,  const int imgWidth, const int imgHeight)
{
    // 保存到成员变量，留给断线重连使用
    m_leftSN = leftSN;
    m_rightSN = rightSN;
    m_imgWidth = imgWidth;
    m_imgHeight = imgHeight;

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
    MV_CC_SetIntValue(m_hDevLeft, "Height", m_imgHeight);
    MV_CC_SetIntValue(m_hDevRight, "Height", m_imgHeight);

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

    // [新增] 获取相机的最大宽度（假设现场是 4096），连同高度一起发给 UI
    MVCC_INTVALUE stIntParam = {0};
    MV_CC_GetIntValue(m_hDevLeft, "Width", &stIntParam);
    int camWidth = stIntParam.nCurValue;

    emit sigImageReadyTOUI(camWidth, m_imgHeight);
    emit signalCameraLog(QString("初始化完毕: 画幅 %1 x %2").arg(camWidth).arg(m_imgHeight));
    return true;
}

void WorkerCamera::configureMasterSlave()
{
    // 【左相机 Master】：关闭软触发，开启内部行频，配置引脚输出脉冲
    MV_CC_SetEnumValueByString(m_hDevLeft, "TriggerMode", "Off");
    MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);
    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate); // 默认行频
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

    // 2. 生成 Halcon 图像对象 (放在锁外面，不占用宝贵的互斥时间)
    HalconCpp::HObject ho_Image;
    try {
        HalconCpp::GenImage1(&ho_Image, "byte", pFrameInfo->nWidth, pFrameInfo->nHeight, (Hlong)pData);
    } catch (const HalconCpp::HException& e) {
        qWarning() << "[WorkerCamera] GenImage1 异常丢帧:" << e.ErrorMessage().Text();
        return; // 生成失败直接退出，不影响系统
    }

    DualCameraChunk readyChunk; // 用于搬运配对成功的包裹
    bool isPairReady = false;

    // ==========================================================
    // 【核心修复】：使用 QMutexLocker 划定极小作用域的临界区
    // 好处：无论发生什么(哪怕抛出异常或return)，离开大括号时自动解锁！绝对不发生死锁。
    // ==========================================================
    {
        QMutexLocker locker(&m_mutex);

        // 3. 查单与建档
        if (m_bufferMap.find(frameID) == m_bufferMap.end()) {
            DualCameraChunk newChunk;
            newChunk.frameID = frameID;
            newChunk.height = pFrameInfo->nHeight;
            m_bufferMap[frameID] = newChunk;
        }

        // 4. 存入对应相机的图像
        if (camIndex == 0) {
            m_bufferMap[frameID].imgLeft = ho_Image;
            m_bufferMap[frameID].hasLeft = true;
        } else {
            m_bufferMap[frameID].imgRight = ho_Image;
            m_bufferMap[frameID].hasRight = true;
        }

        // 5. 检查是否配对成功
        if (m_bufferMap[frameID].hasLeft && m_bufferMap[frameID].hasRight) {
            // 【关键】：把数据拷贝出来，立刻清理 map
            readyChunk = m_bufferMap[frameID];
            isPairReady = true;
            m_bufferMap.erase(frameID);
        }

        // 6. 防爆池机制 (防止单相机断线导致内存泄漏)
        if (m_bufferMap.size() > 10) {
            m_bufferMap.clear();
            qWarning() << "[WorkerCamera] 缓存池溢出，已强制清空孤儿帧！";
        }
    } // <--- 运行到这里，locker 超出生命周期，互斥锁自动安全释放！

    // ==========================================================
    // 临界区之外：处理耗时操作和跨线程通讯
    // ==========================================================

    if (isPairReady) {
        // 1. 发给算法线程
        emit sigImageReadyToAlg(readyChunk);

        // 2. 【修复 UI 冲突】：这里绝对不能直接调用 DispObj！
        // 如果你需要显示原始图像，应该增加一个发给主界面的信号：
        emit sigDisplayRawImage(readyChunk);
        // 让主控 Workstation 在 GUI 主线程中去调用 DispObj 显示。
    }
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
                    MV_CC_SetIntValue(m_hDevLeft, "Height", m_imgHeight);
                    MV_CC_SetEnumValueByString(m_hDevLeft, "TriggerMode", "Off");
                    MV_CC_SetBoolValue(m_hDevLeft, "AcquisitionLineRateEnable", true);
                    MV_CC_SetFloatValue(m_hDevLeft, "AcquisitionLineRate", m_lineRate);
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
                    MV_CC_SetIntValue(m_hDevRight, "Height", m_imgHeight);
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

bool WorkerCamera::HtupleIsEmpty(HalconCpp::HTuple &value) {
    HalconCpp::HTuple length;
    TupleLength(value, &length);

    return length.I() == 0;
}