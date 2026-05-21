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

// // 确保能拿到主线程绑定的窗口句柄 (根据你的实际变量名调整)
// void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {
//     // 线程锁：算法忙时直接丢弃新帧（抽帧保护，防止队列阻塞）
//     if (m_isProcessing) return;
//     m_isProcessing = true;
//     // 【新增日志 1：看是否收到相机的双图】
//     qDebug() << "[Pipeline-1] 算法工收到图像 -> 帧号:" << chunk.frameID
//              << " 左图OK:" << chunk.hasLeft << " 右图OK:" << chunk.hasRight
//              << " 算法已初始化:" << (m_algo != nullptr);

//     if (m_algo && chunk.hasLeft && chunk.hasRight) {
//         // 调用我们刚刚写好的核心算法
//         WidthResult res = m_algo->processFrame(chunk.imgLeft, chunk.imgRight);
//         res.frameID = chunk.frameID;

//         // 【新增日志 2：看算法算完之后的状态】
//         qDebug() << "[Pipeline-2] 算法处理完毕 -> 测宽是否有效:" << res.isValid
//                  << " 宽度值:" << res.widthValue
//                  << " 图像是否打包:" << res.dispImage.IsInitialized();
//         // 算完了，发送给 UI 线程
//         emit sigMeasureReady(res);
//     }
//     m_isProcessing = false;
// }
void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {
    if (m_isProcessing) return;
    m_isProcessing = true;

    if (m_algo && chunk.hasLeft && chunk.hasRight) {
        WidthResult res = m_algo->processFrame(chunk.imgLeft, chunk.imgRight);
        res.frameID = chunk.frameID;

        // =========================================================
        // 2. 基于单帧结果，进行整板长宽统计
        // =========================================================
        if (res.isValid) {
            // --- 状态 A：检测到有效边缘 (有钢板) ---
            if (!m_isPlateActive) {
                // 【入头】初始化所有统计变量
                m_isPlateActive = true;
                m_sumWidth = 0.0;
                m_validFrameCount = 0;
                m_totalRows = 0;
                m_maxWidth = 0.0;        // 重置最大值
                m_minWidth = 99999.0;    // 重置最小值
            }

            // 累加数据
            m_sumWidth += res.widthValue;
            m_validFrameCount++;

            // 【新增】：找最大值和最小值
            if (res.widthValue > m_maxWidth) {
                m_maxWidth = res.widthValue;
            }
            if (res.widthValue < m_minWidth) {
                m_minWidth = res.widthValue;
            }

            // 累加长度 (行数)
            HalconCpp::HTuple w, h;
            HalconCpp::GetImageSize(chunk.imgLeft, &w, &h);
            m_totalRows += h[0].I();

        } else {
            // --- 状态 B：没有检测到有效边缘 ---
            if (m_isPlateActive) {
                // 【出尾】计算最终结果
                m_isPlateActive = false;

                if (m_validFrameCount > 0) {
                    double avgWidth = m_sumWidth / m_validFrameCount;
                    double totalLength = m_totalRows * m_mm_per_row;

                    // 【核心】将四个计算好的数据通过信号发送给 UI 主界面
                    emit sigPlateFinished(avgWidth, totalLength, m_maxWidth, m_minWidth);
                }
            }
        }
        // =========================================================

        emit sigMeasureReady(res);
    }
    m_isProcessing = false;
}