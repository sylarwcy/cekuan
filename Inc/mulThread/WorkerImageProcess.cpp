#include "WorkerImageProcess.h"

WorkerImageProcess::WorkerImageProcess(QObject *parent) : QObject(parent), m_algo(nullptr) {
    // 【关键】：注册 Halcon 类型，允许在跨线程信号槽(QueuedConnection)中传递
    qRegisterMetaType<HalconCpp::HObject>("HalconCpp::HObject");
}

WorkerImageProcess::~WorkerImageProcess() {
    if (m_algo) {
        delete m_algo;
    }
}

void WorkerImageProcess::init(const WorkStation_DATA &paramData) {
    ;
}

void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {

if (m_isProcessing) return; // 算法忙，直接把这张图丢弃（抽帧）
    m_isProcessing = true;

    HObject imgLeft, imgRight;
    imgLeft = chunk.imgLeft;
    imgRight = chunk.imgRight;

    // Halcon 判空使用 IsInitialized()
    if (!m_algo || !imgLeft.IsInitialized() || !imgRight.IsInitialized()) {
        return;
    }

    try {
        // 1. 调用 Halcon 算法
        WidthResult res = m_algo->process(imgLeft, imgRight);

        // 2. 获取结果
        if (res.isOk) {
            // 向主界面 / 数据库线程 / PLC通讯线程 发送有效数据
            emit sigMeasureReady(res);

            // 可选：在此处将提取到的边缘线用 DispLine 画到图像上，发给 UI 显示
            // emit displayImageReady(renderedImg);
        }

        // 3. 发送结果
        // emit resultReady(width, ok);

        // 4. 发送用于 UI 渲染的图像
        // emit displayImageReady(mergedDispImg);

    } catch (HalconCpp::HException& e) {
        qDebug() << "Halcon Exception in ImageProcess:" << e.ErrorMessage().Text();
        emit sigProcessError(QString("Halcon Error: %1").arg(e.ErrorMessage().Text()));
    }
    m_isProcessing = false;
}