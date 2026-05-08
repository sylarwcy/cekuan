#include "WorkerImageProcess.h"
#include "appconfig.h"
#include "HalconCpp.h"
HalconCpp::HTuple winHandle_pro;
WorkerImageProcess::WorkerImageProcess(QObject *parent) : QObject(parent), m_algo(nullptr) {
    qRegisterMetaType<WidthResult>("WidthResult");
}

WorkerImageProcess::~WorkerImageProcess() {
    if (m_algo) {
        delete m_algo;
    }
}

void WorkerImageProcess::init(const WorkStation_DATA &paramData) {
    if (!m_algo) {
        m_algo = new DualLineScanWidthImgPro();
    }

    // 加载字典 (路径根据你实际的部署位置调整，这里以程序当前运行目录为例)
    QString masterDict = "./Camera_Master_1DLUT.hdict";
    QString slaveDict  = "./Camera_Slave_1DLUT_Global.hdict";

    // 参数 0.09473 是你的线阵走带物理当量
    if (m_algo->initAlgorithm(masterDict, slaveDict, 0.09473)) {
        qInfo() << "[系统通知] Halcon 测宽算法初始化成功，字典已加载。";
    }
}

// 确保能拿到主线程绑定的窗口句柄 (根据你的实际变量名调整)
void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {
    // 线程锁：算法忙时直接丢弃新帧（抽帧保护，防止队列阻塞）
    if (m_isProcessing) return;
    m_isProcessing = true;
    // 【新增日志 1：看是否收到相机的双图】
    qDebug() << "[Pipeline-1] 算法工收到图像 -> 帧号:" << chunk.frameID
             << " 左图OK:" << chunk.hasLeft << " 右图OK:" << chunk.hasRight
             << " 算法已初始化:" << (m_algo != nullptr);

    if (m_algo && chunk.hasLeft && chunk.hasRight) {
        // 调用我们刚刚写好的核心算法
        WidthResult res = m_algo->processFrame(chunk.imgLeft, chunk.imgRight);
        res.frameID = chunk.frameID;

        // 【新增日志 2：看算法算完之后的状态】
        qDebug() << "[Pipeline-2] 算法处理完毕 -> 测宽是否有效:" << res.isValid
                 << " 宽度值:" << res.widthValue
                 << " 图像是否打包:" << res.dispImage.IsInitialized();
        // 算完了，发送给 UI 线程
        emit sigMeasureReady(res);
    }
    m_isProcessing = false;
}