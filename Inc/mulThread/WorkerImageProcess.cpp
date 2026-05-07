#include "WorkerImageProcess.h"
#include "appconfig.h"
#include "HalconCpp.h"
HalconCpp::HTuple winHandle_pro;
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

// 确保能拿到主线程绑定的窗口句柄 (根据你的实际变量名调整)


void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {
    // 线程锁：算法忙时直接丢弃新帧（抽帧保护，防止队列阻塞）
    if (m_isProcessing) return;
    m_isProcessing = true;

    HObject imgLeft = chunk.imgLeft;
    HObject imgRight = chunk.imgRight;

    if (!m_algo || !imgLeft.IsInitialized() || !imgRight.IsInitialized()) {
        m_isProcessing = false;
        return;
    }

    HObject ho_ImagesConcat, ho_ImageFull;

    try {
        // ==========================================================
        // 🚄 轨道 1：核心算法轨 (先测后算，保留绝对亚像素精度)
        // ==========================================================
        // 你的算法类 (m_algo) 需要修改为：分别在左右图找边缘，并返回像素坐标
        // 假设它现在能分别返回左边和右边的像素坐标 Px

        // 1. 获取两边真实的亚像素坐标 (运算耗时通常在 2~5ms)
        double leftEdgePx = m_algo->findLeftEdgePixel(imgLeft);
        double rightEdgePx = m_algo->findRightEdgePixel(imgRight);

        // 2. 转换为物理坐标 (mm)
        double leftPhysicalX = leftEdgePx * AppConfig::StitchScale;
        double rightPhysicalX = rightEdgePx * AppConfig::StitchScale;

        // 3. 极速纯数学拼接：(右侧相对左侧的绝对物理偏移 + 右边缘物理坐标) - 左边缘物理坐标
        double finalWidthMM = (AppConfig::StitchOffsetX + rightPhysicalX) - leftPhysicalX;

        // 4. 将完美的测量结果封装并发送给主逻辑 (入库/PLC)
        WidthResult res;
        res.isOk = true;
        res.widthValue = finalWidthMM;
        emit sigMeasureReady(res);

        // ==========================================================
        // 📺 轨道 2：UI 视觉呈现轨 (先拼后显，专供人类肉眼监控)
        // ==========================================================
        // 1. 将原图合并打包
        GenEmptyObj(&ho_ImagesConcat);
        ConcatObj(imgLeft, imgRight, &ho_ImagesConcat);

        // 2. 加载 UI 拼接偏移量 (读配置文件的毫米转像素)
        HTuple hv_RowOffsets, hv_ColOffsets, hv_MinusOne;
        hv_RowOffsets.Append(0).Append(AppConfig::StitchOffsetY / AppConfig::StitchScale);
        hv_ColOffsets.Append(0).Append(AppConfig::StitchOffsetX / AppConfig::StitchScale);
        hv_MinusOne.Append(-1).Append(-1);

        // 3. 图像拼接
        TileImagesOffset(ho_ImagesConcat, &ho_ImageFull,
                         hv_RowOffsets, hv_ColOffsets,
                         hv_MinusOne, hv_MinusOne, hv_MinusOne, hv_MinusOne,
                         AppConfig::StitchTotalWidth, AppConfig::StitchTotalHeight);

        // 4. 直接跨线程底层渲染 (无延时上屏)
        if (winHandle_pro.Length() > 0)  {
            HDevWindowStack::SetActive(winHandle_pro);
            if (HDevWindowStack::IsOpen()) {
                // 冻结显卡刷新
                SetSystem("flush_graphic", "false");

                // 自适应视口大小
                HTuple hv_FullW, hv_FullH;
                GetImageSize(ho_ImageFull, &hv_FullW, &hv_FullH);
                SetPart(winHandle_pro, 0, 0, hv_FullH - 1, hv_FullW - 1);

                // 渲染 4.5 米拼接巨图
                DispObj(ho_ImageFull, winHandle_pro);

                // 【锦上添花】：在 UI 拼接图上叠加两条绿线，直观显示机器测在哪里
                SetColor(winHandle_pro, "green");
                SetLineWidth(winHandle_pro, 3);
                // 画左边缘线
                DispLine(winHandle_pro, 0, leftEdgePx, hv_FullH, leftEdgePx);
                // 画右边缘线 (注意：右线在拼接图上的X坐标 = 右图内像素 + UI拼接的像素偏移量)
                double rightLineDisplayX = rightEdgePx + hv_ColOffsets[1].D();
                DispLine(winHandle_pro, 0, rightLineDisplayX, hv_FullH, rightLineDisplayX);

                // 释放显卡刷新，全景画面+标定线瞬间闪现
                SetSystem("flush_graphic", "true");
            }
        }

    } catch (HalconCpp::HException &except) {
        // 工业级容错：打印 Halcon 异常代码，防止子线程静默死亡
        // QString errStr = QString("Halcon Error: %1").arg(except.ErrorMessage().Text());
        // emit sigLogError(errStr);
    }

    // ==========================================================
    // 🧹 内存终极防线
    // ==========================================================
    // 线阵相机扫出来的高清长图必须手动 Clear，绝不依赖 C++ 作用域回收！
    ho_ImagesConcat.Clear();
    ho_ImageFull.Clear();

    // 释放状态锁
    m_isProcessing = false;
}
// void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {

// if (m_isProcessing) return; // 算法忙，直接把这张图丢弃（抽帧）
//     m_isProcessing = true;
// // 1. 定义输出变量
// HObject ho_ImageFull;
// HObject ho_ImagesConcat;
//     HObject imgLeft, imgRight;
//     imgLeft = chunk.imgLeft;
//     imgRight = chunk.imgRight;

//     // Halcon 判空使用 IsInitialized()
//     if (!m_algo || !imgLeft.IsInitialized() || !imgRight.IsInitialized()) {
//         return;
//     }

//     try {
//         // 1. 调用 Halcon 算法
//         WidthResult res = m_algo->process(imgLeft, imgRight);


//         // ==========================================================
//         // 🚄 轨道 1：核心算法轨 (先测后算，保留绝对亚像素精度)
//         // ==========================================================
//         // 你的算法类 (m_algo) 需要修改为：分别在左右图找边缘，并返回像素坐标
//         // 假设它现在能分别返回左边和右边的像素坐标 Px

//         // 1. 获取两边真实的亚像素坐标 (运算耗时通常在 2~5ms)
//         double leftEdgePx = m_algo->findLeftEdgePixel(imgLeft);
//         double rightEdgePx = m_algo->findRightEdgePixel(imgRight);

//         // 2. 转换为物理坐标 (mm)
//         double leftPhysicalX = leftEdgePx * AppConfig::StitchScale;
//         double rightPhysicalX = rightEdgePx * AppConfig::StitchScale;

//         // 3. 极速纯数学拼接：(右侧相对左侧的绝对物理偏移 + 右边缘物理坐标) - 左边缘物理坐标
//         double finalWidthMM = (AppConfig::StitchOffsetX + rightPhysicalX) - leftPhysicalX;

//         // 4. 将完美的测量结果封装并发送给主逻辑 (入库/PLC)
//         WidthResult res;
//         res.isOk = true;
//         res.widthValue = finalWidthMM;
//         emit sigMeasureReady(res);

//         // ==========================================================
//         // 📺 轨道 2：UI 视觉呈现轨 (先拼后显，专供人类肉眼监控)
//         // ==========================================================
//         // 1. 将原图合并打包
//         GenEmptyObj(&ho_ImagesConcat);
//         ConcatObj(imgLeft, imgRight, &ho_ImagesConcat);

//         // 2. 加载 UI 拼接偏移量 (读配置文件的毫米转像素)
//         HTuple hv_RowOffsets, hv_ColOffsets, hv_MinusOne;
//         hv_RowOffsets.Append(0).Append(AppConfig::StitchOffsetY / AppConfig::StitchScale);
//         hv_ColOffsets.Append(0).Append(AppConfig::StitchOffsetX / AppConfig::StitchScale);
//         hv_MinusOne.Append(-1).Append(-1);

//         // 3. 图像拼接
//         TileImagesOffset(ho_ImagesConcat, &ho_ImageFull,
//                          hv_RowOffsets, hv_ColOffsets,
//                          hv_MinusOne, hv_MinusOne, hv_MinusOne, hv_MinusOne,
//                          AppConfig::StitchTotalWidth, AppConfig::StitchTotalHeight);

//         // 4. 直接跨线程底层渲染 (无延时上屏)
//         if (!HtupleIsEmpty(winHandle_pro)) {
//             HDevWindowStack::SetActive(winHandle_pro);
//             if (HDevWindowStack::IsOpen()) {
//                 // 冻结显卡刷新
//                 SetSystem("flush_graphic", "false");

//                 // 自适应视口大小
//                 HTuple hv_FullW, hv_FullH;
//                 GetImageSize(ho_ImageFull, &hv_FullW, &hv_FullH);
//                 SetPart(winHandle_pro, 0, 0, hv_FullH - 1, hv_FullW - 1);

//                 // 渲染 4.5 米拼接巨图
//                 DispObj(ho_ImageFull, winHandle_pro);

//                 // 【锦上添花】：在 UI 拼接图上叠加两条绿线，直观显示机器测在哪里
//                 // SetColor(winHandle_pro, "green");
//                 // SetLineWidth(winHandle_pro, 3);
//                 // 画左边缘线
//                 // DispLine(winHandle_pro, 0, leftEdgePx, hv_FullH, leftEdgePx);
//                 // 画右边缘线 (注意：右线在拼接图上的X坐标 = 右图内像素 + UI拼接的像素偏移量)
//                 // double rightLineDisplayX = rightEdgePx + hv_ColOffsets[1].D();
//                 // DispLine(winHandle_pro, 0, rightLineDisplayX, hv_FullH, rightLineDisplayX);

//                 // 释放显卡刷新，全景画面+标定线瞬间闪现
//                 SetSystem("flush_graphic", "true");
//             }

//           // 2. 获取结果
//         if (res.isOk) {
//             // 向主界面 / 数据库线程 / PLC通讯线程 发送有效数据
//             emit sigMeasureReady(res);

//             // 可选：在此处将提取到的边缘线用 DispLine 画到图像上，发给 UI 显示
//             // emit displayImageReady(renderedImg);
//         }

//         // 3. 发送结果
//         // emit resultReady(width, ok);

//         // 4. 发送用于 UI 渲染的图像
//         // emit displayImageReady(mergedDispImg);

//     } catch (HalconCpp::HException& e) {
//         qDebug() << "Halcon Exception in ImageProcess:" << e.ErrorMessage().Text();
//         emit sigProcessError(QString("Halcon Error: %1").arg(e.ErrorMessage().Text()));
//     }
//     m_isProcessing = false;
// }