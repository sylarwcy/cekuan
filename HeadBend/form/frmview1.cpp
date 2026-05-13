#include "frmview1.h"
#include "ui_frmview1.h"
#include "MyApplication.h"

frmView1::frmView1(QWidget *parent) : QWidget(parent),
                                      ui(new Ui::frmView1) {
    MyApplication *pApp = (MyApplication *) qApp;

    ui->setupUi(this);
    initForm();

    //变量初始化
    QTimer::singleShot(1000, this,SLOT(varInit()));
}

frmView1::~frmView1() {
    delete ui;

    // 清理相机资源 (如果后台仍然在使用)
    if (p_camera_front) {
        p_camera_front->StopGrabbing();
        p_camera_front->CloseCamera();
        delete p_camera_front;
    }
    if (p_camera_back) {
        p_camera_back->StopGrabbing();
        p_camera_back->CloseCamera();
        delete p_camera_back;
    }
}

void frmView1::resizeEvent(QResizeEvent *event) {
    static unsigned long tri_num;
    int width, height;

    tri_num++;

    //第一次触发会出错
    if (tri_num > 1) {
        width = ui->gView_front_ori->width();
        height = ui->gView_front_ori->height();
        SetWindowExtents(winHandle_cam1_ori, 0, 0, width, height);

        width = ui->gView_front_pro->width();
        height = ui->gView_front_pro->height();
        SetWindowExtents(winHandle_cam2_ori, 0, 0, width, height);

        width = ui->gView_back_ori->width();
        height = ui->gView_back_ori->height();
        SetWindowExtents(winHandle_cam1_pro, 0, 0, width, height);
    }
}

void frmView1::logSlot(const QString &message, int level) {
    ui->textBrowser->append(qUtf8Printable(message));
}

void frmView1::varInit() {
    MyApplication *pApp = (MyApplication *) qApp;

    // 绑定原生图控件
    Hlong winId_front_ori = (Hlong) ui->gView_front_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height(), winId_front_ori, "visible", "", &winHandle_cam1_ori);
    HDevWindowStack::Push(winHandle_cam1_ori);

    // 绑定拼接图控件
    Hlong winId_front_pro = (Hlong) ui->gView_front_pro->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height(), winId_front_pro, "visible", "", &winHandle_cam1_pro);
    HDevWindowStack::Push(winHandle_cam1_pro);

    // 绑定右侧图控件
    Hlong winId_back_ori = (Hlong) ui->gView_back_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_back_ori->width(), ui->gView_back_ori->height(), winId_back_ori, "visible", "", &winHandle_cam2_ori);
    HDevWindowStack::Push(winHandle_cam2_ori);

    // 初始化全景拼接窗口
    Hlong winId_fusion = (Hlong)ui->gView_fusion->winId();
    // 背景设为黑色，显得专业
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->gView_fusion->width(), ui->gView_fusion->height(),
                         winId_fusion, "visible", "", &winHandle_fusion);
    HDevWindowStack::Push(winHandle_fusion);
    m_isFirstFrame = true;

    //相机与界面显示控件绑定
    pApp->pNodeData->camera_1_para.winHandle_ori = winHandle_cam1_ori;
    pApp->pNodeData->camera_2_para.winHandle_ori = winHandle_cam2_ori;

    connect(pApp->m_workstationList[0], &Workstation::sigForwardToView,
                this, &frmView1::onUpdateRawImage);

    QLOG_INFO() << u8"界面控件初始化完成!";

    // 防止闪烁
    ui->gView_front_ori->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_front_pro->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_back_ori->setAttribute(Qt::WA_OpaquePaintEvent);

    //启动界面刷新
    startDispRefresh();
}

void frmView1::initForm() {
    ui->textBrowser->setWordWrapMode(QTextOption::NoWrap);
    ui->textBrowser->document()->setMaximumBlockCount(100);

    ui->textBrowser->setStyleSheet("#textBrowser { background-color: white; color: black; }");

    QString noBorderStyle = "border: none; background: transparent;";

    // 逐个设置，确保覆盖全局 QUI 皮肤
    ui->frame_main->setStyleSheet(QString("#frame_main { %1 }").arg(noBorderStyle));
    ui->frame_cam->setStyleSheet(QString("#frame_cam { %1 }").arg(noBorderStyle));
    ui->frame_curve->setStyleSheet(QString("#frame_curve { %1 }").arg(noBorderStyle));
    ui->frame_fusion->setStyleSheet(QString("#frame_fusion { %1 }").arg(noBorderStyle));
    ui->frame_info->setStyleSheet(QString("#frame_info { %1 }").arg(noBorderStyle));
    ui->frame_setValue->setStyleSheet(QString("#frame_setValue { %1 }").arg(noBorderStyle));
    ui->frame_get->setStyleSheet(QString("#frame_get { %1 }").arg(noBorderStyle));
    ui->frame_setWidth->setStyleSheet(QString("#frame_setWidth { %1 }").arg(noBorderStyle));
    ui->frame_width->setStyleSheet(QString("#frame_width { %1 }").arg(noBorderStyle));
    ui->frame_sta->setStyleSheet(QString("#frame_sta { %1 }").arg(noBorderStyle));
    ui->frame_steelnum->setStyleSheet(QString("#frame_steelnum { %1 }").arg(noBorderStyle));
    ui->frame_thickness->setStyleSheet(QString("#frame_thickness { %1 }").arg(noBorderStyle));
    ui->frame_search->setStyleSheet(QString("#frame_search { %1 }").arg(noBorderStyle));

    // 初始化视觉触发状态机
    m_isPlatePresent = false;
    m_emptyFrameCount = 0;

    adjustFontSize(ui->lineEdit);
    initCurveChart();
}

void frmView1::adjustFontSize(QLineEdit* lineEdit) {
    if (!lineEdit) return; // 安全检查

    QFont font = lineEdit->font();

    // 根据当前 lineEdit 的固定高度计算字号（0.6 是个经验值，防止字贴着边框）
    // 因为你已经在 UI 设计器里锁死了 Height，所以这里的 height() 获取到的是准确值
    int newSize = lineEdit->height() * 0.6;

    if (newSize > 0) {
        font.setPixelSize(newSize);
        lineEdit->setFont(font);
    }
}

void frmView1::onUpdateRawImage(const DualCameraChunk &chunk) {
    try {
        if (chunk.imgLeft.IsInitialized()) {
            HTuple widthL, heightL;
            HalconCpp::GetImageSize(chunk.imgLeft, &widthL, &heightL);
            HalconCpp::SetPart(winHandle_cam1_ori, 0, 0, heightL - 1, widthL - 1);
            HalconCpp::DispObj(chunk.imgLeft, winHandle_cam1_ori);
        }
        if (chunk.imgRight.IsInitialized()) {
            HTuple widthR, heightS;
            HalconCpp::GetImageSize(chunk.imgRight, &widthR, &heightS);
            HalconCpp::SetPart(winHandle_cam2_ori, 0, 0, heightS - 1, widthR - 1);
            HalconCpp::DispObj(chunk.imgRight, winHandle_cam2_ori);
        }
    } catch (const HalconCpp::HException& e) {
        QLOG_ERROR() << "UI刷新原始图像失败:" << e.ErrorMessage().Text();
    }
}

//用于数据刷新
void frmView1::refreshDataDisp(void) {
    MyApplication *pApp = (MyApplication *) qApp;
    pApp->pNodeData->isDispProcessing = false;
}

//启动触发循环
void frmView1::startDispRefresh() {
    MyApplication *pApp = (MyApplication *) qApp;
    for (int i = 0; i < pApp->m_workstationList.size(); ++i)
        pApp->m_workstationList[i]->StartTrigger();

    QLOG_INFO() << "触发进程启动，开始刷新界面...";
}

void frmView1::initCurveChart()
{
    m_currentFrameCount = 0;

    // 1. 基本背景颜色设置 (深灰黑色背景，适应你的工业UI皮肤)
    ui->customPlot_width->setBackground(QBrush(QColor(30, 30, 30)));
    ui->customPlot_width->axisRect()->setBackground(QBrush(QColor(20, 20, 20)));

    // 2. 添加一条折线 (Graph 0)
    ui->customPlot_width->addGraph();
    // 设置折线颜色为荧光绿，线宽为 2
    ui->customPlot_width->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    // 设置数据点显示为实心小圆圈
    ui->customPlot_width->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));

    // 3. X 轴和 Y 轴的标签和颜色
    ui->customPlot_width->xAxis->setLabelColor(Qt::white);
    ui->customPlot_width->xAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->xAxis->setBasePen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setSubTickPen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setLabel("图像帧数 (Frame)");

    ui->customPlot_width->yAxis->setLabelColor(Qt::white);
    ui->customPlot_width->yAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->yAxis->setBasePen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setSubTickPen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setLabel("宽度尺寸 (mm)");

    // 4. 设置默认的坐标轴范围
    ui->customPlot_width->xAxis->setRange(0, 15);     // 假设一张钢板默认拍15张
    ui->customPlot_width->yAxis->setRange(1000, 2500); // 宽度根据你实际板宽预设个大概范围

    // 5. 允许用户用鼠标拖拽和缩放图表 (超级好用的功能！)
    ui->customPlot_width->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void frmView1::addCurvePoint(double widthValue)
{
    // 1. 计数器加1，并将数据压入缓存
    m_currentFrameCount++;
    m_vecFrameIndex.append(m_currentFrameCount);
    m_vecWidthValue.append(widthValue);

    // 2. 将最新的缓存数据喂给图表
    ui->customPlot_width->graph(0)->setData(m_vecFrameIndex, m_vecWidthValue);

    // 3. 动态调整 X 轴范围 (如果拍的张数超过了当前界面，X轴自动往右滚动)
    if (m_currentFrameCount > ui->customPlot_width->xAxis->range().upper) {
        ui->customPlot_width->xAxis->setRange(0, m_currentFrameCount + 5);
    }

    // 4. 动态调整 Y 轴范围 (让曲线始终在画面中间上下轻微浮动展示)
    // false = 不缩放X轴, true = 放大Y轴适应数据
    // ui->customPlot_width->graph(0)->rescaleValueAxis(false, true);
    // 1. 先让图表紧贴数据自适应
    ui->customPlot_width->graph(0)->rescaleValueAxis(false, false);
    // 2. 然后把 Y 轴的上下限强行扩大 1.5 倍（上下各留 10% 的空白边距）
    ui->customPlot_width->yAxis->scaleRange(1.5);

    // 5. 立即重新绘制
    ui->customPlot_width->replot();
}

void frmView1::clearCurveChart()
{
    // 清空缓存
    m_currentFrameCount = 0;
    m_vecFrameIndex.clear();
    m_vecWidthValue.clear();

    // 清空图表上的数据点
    ui->customPlot_width->graph(0)->data()->clear();

    // 恢复默认 X 轴显示范围
    ui->customPlot_width->xAxis->setRange(0, 15);

    // 重绘以消除上一张板的线
    ui->customPlot_width->replot();
}

void frmView1::onMeasureReady(const WidthResult &res)
{
    // 这里的 lineEdit 就是你新 UI 里负责显示宽度的控件
    if (res.isValid) {// 只有测量成功了才画点 & 显示宽度
        ui->lineEdit->setText(QString::number(res.widthValue, 'f', 2));
    } else {
        ui->lineEdit->setText("0.00");
    }

    if (res.isValid) {
        // 条件A：算法看到了有效宽度
        // 如果系统之前记录的状态是“没板子”，说明这是新板子的“板头”刚刚进入视野！
        if (!m_isPlatePresent) {
            clearCurveChart();          // 瞬间清空上一张钢板的旧折线！
            resetFusion();     // 清空全景图
            m_isPlatePresent = true;    // 状态切换为：板子正在视野中
            // qInfo() << ">>> 视觉检测到新钢板到达，折线已清空！ <<<";
        }

        // 只要能看到有效宽度，就把点画上去
        addCurvePoint(res.widthValue);

        addFrameToFusion(res.dispImage);

        // 既然看到了板子，就把“空帧防抖计数器”清零
        m_emptyFrameCount = 0;

    } else {
        // 条件B：算法没看到有效宽度 (板子没来，或者板子走完了，或者被水汽挡住了)

        // 只有当系统认为当前有板子时，才开始累加空帧
        if (m_isPlatePresent) {
            m_emptyFrameCount++;

            // 如果连续 EMPTY_FRAME_LIMIT (3) 帧都没看到板子，说明板子的“板尾”彻底离开了
            if (m_emptyFrameCount >= EMPTY_FRAME_LIMIT) {
                m_isPlatePresent = false; // 状态彻底切换为：视野空闲，等待下一张板子
                // qInfo() << ">>> 视觉检测到钢板已完全离开视野。 <<<";
            }
        }
    }

    if (res.dispImage.IsInitialized()) {
        try {
            HTuple currentWin = this->winHandle_cam1_pro;
            HTuple imgW, imgH;
            HalconCpp::GetImageSize(res.dispImage, &imgW, &imgH);
            HalconCpp::SetPart(currentWin, 0, 0, imgH - 1, imgW - 1);
            HalconCpp::DispObj(res.dispImage, currentWin);

            if (res.isValid && res.renderLeftX != -1) {
                HalconCpp::SetLineWidth(currentWin, 2);

                HalconCpp::SetColor(currentWin, "green");
                HalconCpp::DispLine(currentWin, 0, res.renderLeftX, imgH - 1, res.renderLeftX);

                HalconCpp::SetColor(currentWin, "cyan");
                HalconCpp::DispLine(currentWin, 0, res.renderRightX, imgH - 1, res.renderRightX);

                HalconCpp::SetColor(currentWin, "red");
                HalconCpp::DispCross(currentWin, res.renderY, res.renderLeftX, 60, 0);
                HalconCpp::DispCross(currentWin, res.renderY, res.renderRightX, 60, 0);

                QString widthStr = QString("Physical Width: %1 mm").arg(res.widthValue, 0, 'f', 3);
                HalconCpp::DispText(currentWin, widthStr.toLocal8Bit().constData(),
                                    "window", 20, 20, "red", HTuple(), HTuple());

                QString yawStr = QString("Yaw Angle: %1 deg").arg(res.yawAngle, 0, 'f', 2);
                HalconCpp::DispText(currentWin, yawStr.toLocal8Bit().constData(),
                                    "window", 50, 20, "yellow", HTuple(), HTuple());
            } else {
                HalconCpp::DispText(currentWin, "NO PLATE DETECTED",
                                    "window", "center", "center", "red", HTuple(), HTuple());
            }

        } catch (HalconCpp::HException &e) {
            qWarning() << "[UI渲染报错] " << e.ErrorMessage().Text();
        }
    }
}

void frmView1::resetFusion() {
    m_hFusedImage.Clear(); // 清空旧图对象
    m_isFirstFrame = true;
}

void frmView1::addFrameToFusion(HalconCpp::HObject newFrame) {
    try {
        HalconCpp::HObject zoomedFrame;
        HalconCpp::ZoomImageFactor(newFrame, &zoomedFrame, 0.1, 0.1, "bilinear");

        if (m_isFirstFrame) {
            m_hFusedImage = zoomedFrame;
            m_isFirstFrame = false;
        } else {
            // 【核心算子】：拼接图像
            // 将新帧 newFrame 追加到 m_hFusedImage 后面（垂直方向拼接）
            // 参数 2 表示 2 帧，'vertical' 表示垂直堆叠
            HalconCpp::HObject concatenated;
            HalconCpp::ConcatObj(m_hFusedImage, zoomedFrame, &concatenated);
            // 变成一列（即垂直拼接）
            HalconCpp::TileImages(concatenated, &m_hFusedImage, 1, "vertical");
        }

        // 【核心要求】：逆时针旋转 90 度显示
        // 在 Halcon 中，正 90 度是逆时针
        HalconCpp::HObject imageRotated;
        HalconCpp::RotateImage(m_hFusedImage, &imageRotated, 90, "constant");

        // --- 渲染到界面 ---
        HTuple imgW, imgH;
        HalconCpp::GetImageSize(imageRotated, &imgW, &imgH);
        // 设置显示比例，自适应窗口大小
        HalconCpp::SetPart(winHandle_fusion, 0, 0, imgH - 1, imgW - 1);
        HalconCpp::DispObj(imageRotated, winHandle_fusion);

    } catch (HalconCpp::HException &e) {
        QLOG_ERROR() << "全景拼接/缩放失败: " << e.ErrorMessage().Text();
    }
}