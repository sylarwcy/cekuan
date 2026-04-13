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

void WorkerImageProcess::onSyncedImagesReady(const HalconCpp::HObject& imgLeft, const HalconCpp::HObject& imgRight) {
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
            emit sigWidthDataReady(res.widthValue, res.centerOffset);

            // 可选：在此处将提取到的边缘线用 DispLine 画到图像上，发给 UI 显示
            // emit displayImageReady(renderedImg);
        } else {
            // 抛出报警或异常帧
            emit sigProcessError(res.errorMsg);
        }

        // 3. 发送结果
        // emit resultReady(width, ok);

        // 4. 发送用于 UI 渲染的图像
        // emit displayImageReady(mergedDispImg);

    } catch (HalconCpp::HException& e) {
        // Halcon 异常捕获非常重要，否则会导致线程崩溃
        qDebug() << "Halcon Exception in ImageProcess:" << e.ErrorMessage().Text();
    }
}