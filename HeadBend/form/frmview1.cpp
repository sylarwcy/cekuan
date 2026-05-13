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

    initHistoryUI();
    loadHistoryFromFile();
}

void frmView1::adjustFontSize(QLineEdit* lineEdit) {
    if (!lineEdit) return; // 安全检查

    QFont font = lineEdit->font();

    // 根据当前 lineEdit 的固定高度计算字号（0.6 是个经验值，防止字贴着边框）
    // 因为你已经在 UI 设计器里锁死了 Height，所以这里的 height() 获取到的是准确值
    int newSize = lineEdit->height() * 1;

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
        // --- 条件 A：算法识别到了钢板 ---

        if (!m_isPlatePresent) {
            // 【上升沿：新钢板刚刚到达！】
            clearCurveChart();
            resetFusion();
            m_isPlatePresent = true;

            // 💡【核心动作：补头】把缓存里的“老图”作为板头先拼进去！
            for (int i = 0; i < m_preBufferList.size(); ++i) {
                addFrameToFusion(m_preBufferList.at(i));
            }
            m_preBufferList.clear(); // 拼完立刻清空兜里的图
        }

        // 正常画折线点
        addCurvePoint(res.widthValue);

        // 正常拼入当前帧
        if (res.dispImage.IsInitialized()) {
            addFrameToFusion(res.dispImage);
        }

        // 重置空帧计数器
        m_emptyFrameCount = 0;

    } else {
        // --- 条件 B：算法没看到钢板 ---

        if (m_isPlatePresent) {
            // 【当前状态是有钢板的，但这一帧瞎了（可能是有干扰，或者是板尾正在离开）】
            m_emptyFrameCount++;

            // 💡【核心动作：补尾】只要还没达到结束阈值，就把这帧空背景也拼进去当板尾！
            if (res.dispImage.IsInitialized() && m_emptyFrameCount <= EMPTY_FRAME_LIMIT) {
                addFrameToFusion(res.dispImage);
            }

            // 如果连续多帧没看到板子，说明板尾彻底走完了
            if (m_emptyFrameCount >= EMPTY_FRAME_LIMIT) {
                m_isPlatePresent = false; // 状态彻底复位

                // ... 此处保留你原有的：计算最大最小值、平均值，并存入本地前 5 条历史记录的代码 ...
                // int pointCount = m_vecWidthValue.size();
                // if (pointCount > 0) {
                //     ...
                // }
            }

        } else {
            // 【当前视野里完全是空的，正在等待下一张新钢板】
            // 💡【核心动作：蓄力板头】虽然是空画面，但它可能是下一张钢板的头！存起来！
            if (res.dispImage.IsInitialized()) {
                m_preBufferList.append(res.dispImage);

                // 维持队列长度，踢出最老的图，永远只保留最近的 HEAD_FRAME_COUNT 张
                while (m_preBufferList.size() > HEAD_FRAME_COUNT) {
                    m_preBufferList.removeFirst();
                }
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
        HalconCpp::ZoomImageFactor(newFrame, &zoomedFrame, 1, 1, "bilinear");

        if (m_isFirstFrame) {
            m_hFusedImage = zoomedFrame;
            m_isFirstFrame = false;
        } else {
            HalconCpp::HObject tempTuple;
            // 【关键改变】：ConcatObj 拼接的是所有原始小图的集合
            HalconCpp::ConcatObj(m_hFusedImage, zoomedFrame, &tempTuple);
            m_hFusedImage = tempTuple;
        }
        // 3. 对纯净的“图库集合”执行平铺
        // 因为所有图尺寸一致，TileImages 会严丝合缝地把它们贴在一起！
        HalconCpp::HObject fusedImage;
        HalconCpp::TileImages(m_hFusedImage, &fusedImage, 1, "vertical");

        // 4. 逆时针旋转 90 度
        HalconCpp::HObject imageRotated;
        HalconCpp::RotateImage(fusedImage, &imageRotated, 90, "constant");

        // --- 5. 渲染到界面 ---
        HTuple imgW, imgH;
        HalconCpp::GetImageSize(imageRotated, &imgW, &imgH);

        HalconCpp::SetPart(winHandle_fusion, 0, 0, imgH - 1, imgW - 1);
        HalconCpp::DispObj(imageRotated, winHandle_fusion);

    } catch (HalconCpp::HException &e) {
        QLOG_ERROR() << "全景拼接失败: " << e.ErrorMessage().Text();
    }
}

void frmView1::initHistoryUI()
{
    // 初始化标准表格模型
    m_tableModelHistory = new QStandardItemModel(this);
    QStringList headers = {"板号", "长度(mm)", "厚度(mm)", "设定宽度(mm)", "平均宽度(mm)", "最大宽度(mm)", "最小宽度(mm)"};
    m_tableModelHistory->setHorizontalHeaderLabels(headers);

    ui->tableView_history->setModel(m_tableModelHistory);
    ui->tableView_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_history->setEditTriggers(QAbstractItemView::NoEditTriggers); // 只读

    QFont headerFont = ui->tableView_history->horizontalHeader()->font();
    headerFont.setPointSize(14); // 这里填你想要的字号，比如 14 或 16
    headerFont.setBold(true);    // 建议加粗，让表头更醒目
    ui->tableView_history->horizontalHeader()->setFont(headerFont);

    // 让列宽自动拉伸平分
    ui->tableView_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void frmView1::loadHistoryFromFile()
{
    // 文件保存在程序运行目录下
    QString filePath = QApplication::applicationDirPath() + "/history_data.dat";
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_0); // 确保版本兼容性
        in >> m_historyList; // 一行代码直接把二进制文件还原成 QList
        file.close();
    }

    // 加载完成后刷新一下界面
    updateHistoryTable();
}

void frmView1::saveHistoryToFile()
{
    QString filePath = QApplication::applicationDirPath() + "/history_data.dat";
    QFile file(filePath);

    // 以只写模式打开，覆盖旧文件
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_0);
        out << m_historyList; // 一行代码把 QList 变成二进制写入硬盘
        file.close();
    }
}

void frmView1::addHistoryRecord(const QString& plateID, double length, double thickness,
                                double targetW, double avgW, double maxW, double minW)
{
    PlateRecord newRecord;
    newRecord.plateID = plateID;
    newRecord.length = length;
    newRecord.thickness = thickness;
    newRecord.targetWidth = targetW;
    newRecord.avgWidth = avgW;
    newRecord.maxWidth = maxW;
    newRecord.minWidth = minW;

    // 1. 将新记录插入到列表最前面（最新测的排在第一行）
    m_historyList.prepend(newRecord);

    // 2. 如果超过 5 条，就把最后面的老数据踢掉
    while (m_historyList.size() > 5) {
        m_historyList.removeLast();
    }

    // 3. 立即保存到本地文件
    saveHistoryToFile();

    // 4. 刷新界面表格
    updateHistoryTable();
}

void frmView1::updateHistoryTable()
{
    // 清空现有表格内容（不清空表头）
    m_tableModelHistory->setRowCount(0);

    // 遍历内存中的那 5 条数据，填入表格
    for (int i = 0; i < m_historyList.size(); ++i) {
        const PlateRecord &rec = m_historyList.at(i);

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(rec.plateID);
        rowItems << new QStandardItem(QString::number(rec.length, 'f', 1));
        rowItems << new QStandardItem(QString::number(rec.thickness, 'f', 2));
        rowItems << new QStandardItem(QString::number(rec.targetWidth, 'f', 1));
        rowItems << new QStandardItem(QString::number(rec.avgWidth, 'f', 2));
        rowItems << new QStandardItem(QString::number(rec.maxWidth, 'f', 2));
        rowItems << new QStandardItem(QString::number(rec.minWidth, 'f', 2));

        // 让文字居中显示更美观
        for(auto item : rowItems) {
            item->setTextAlignment(Qt::AlignCenter);
        }

        m_tableModelHistory->appendRow(rowItems);
    }
}