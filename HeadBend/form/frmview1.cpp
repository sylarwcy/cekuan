#include "frmview1.h"
#include "ui_frmview1.h"
#include "MyApplication.h"
#include <algorithm>

frmView1::frmView1(QWidget *parent) : QWidget(parent), ui(new Ui::frmView1) {
    ui->setupUi(this);
    initForm();
    loadHistoryFromFile();
    QTimer::singleShot(1000, this, SLOT(varInit()));
}

frmView1::~frmView1() {
    delete ui;
    if (p_camera_front) { p_camera_front->StopGrabbing(); p_camera_front->CloseCamera(); delete p_camera_front; }
    if (p_camera_back) { p_camera_back->StopGrabbing(); p_camera_back->CloseCamera(); delete p_camera_back; }
}

void frmView1::resizeEvent(QResizeEvent *event) {
    static unsigned long tri_num;
    tri_num++;
    if (tri_num > 1) {
        SetWindowExtents(winHandle_cam1_ori, 0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height());
        SetWindowExtents(winHandle_cam2_ori, 0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height());
        SetWindowExtents(winHandle_cam1_pro, 0, 0, ui->gView_back_ori->width(), ui->gView_back_ori->height());
    }
}

void frmView1::logSlot(const QString &message, int level) { ui->textBrowser->append(qUtf8Printable(message)); }

void frmView1::varInit() {
    MyApplication *pApp = (MyApplication *) qApp;

    Hlong winId_front_ori = (Hlong) ui->gView_front_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height(), winId_front_ori, "visible", "", &winHandle_cam1_ori);

    Hlong winId_front_pro = (Hlong) ui->gView_front_pro->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height(), winId_front_pro, "visible", "", &winHandle_cam1_pro);

    Hlong winId_back_ori = (Hlong) ui->gView_back_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_back_ori->width(), ui->gView_back_ori->height(), winId_back_ori, "visible", "", &winHandle_cam2_ori);

    Hlong winId_fusion = (Hlong)ui->gView_fusion->winId();
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->gView_fusion->width(), ui->gView_fusion->height(), winId_fusion, "visible", "", &winHandle_fusion);
    m_isFirstFrame = true;

    pApp->pNodeData->camera_1_para.winHandle_ori = winHandle_cam1_ori;
    pApp->pNodeData->camera_2_para.winHandle_ori = winHandle_cam2_ori;

    connect(pApp->m_workstationList[0], &Workstation::sigForwardToView, this, &frmView1::onUpdateRawImage);

    ui->gView_front_ori->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_front_pro->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_back_ori->setAttribute(Qt::WA_OpaquePaintEvent);

    Hlong winId_front = (Hlong)ui->graphicsView_front->winId();
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->graphicsView_front->width(), ui->graphicsView_front->height(), winId_front, "visible", "", &winHandle_front);

    Hlong winId_back = (Hlong)ui->graphicsView_back->winId();
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->graphicsView_back->width(), ui->graphicsView_back->height(), winId_back, "visible", "", &winHandle_back);

    startDispRefresh();
}

void frmView1::initForm() {
    ui->textBrowser->setWordWrapMode(QTextOption::NoWrap);
    ui->textBrowser->document()->setMaximumBlockCount(100);
    ui->textBrowser->setStyleSheet("#textBrowser { background-color: white; color: black; }");

    QString noBorderStyle = "border: none; background: transparent;";
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
    ui->graphicsView_front->setStyleSheet(QString("#graphicsView_front { %1 }").arg(noBorderStyle));
    ui->graphicsView_back->setStyleSheet(QString("#graphicsView_back { %1 }").arg(noBorderStyle));

    ui->graphicsView_front->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->graphicsView_back->setAttribute(Qt::WA_OpaquePaintEvent);
    m_isPlatePresent = false;
    m_emptyFrameCount = 0;

    adjustFontSize(ui->lineEdit,76,40);
    adjustFontSize(ui->lineEdit_3,35,26);
    adjustFontSize(ui->lineEdit_4,35,26);
    adjustFontSize(ui->lineEdit_8,35,26);
    adjustFontSize(ui->lineEdit_9,35,26);
    adjustFontSize(ui->lineEdit_5,76,40); ui->lineEdit_5->setText("0.00");
    adjustFontSize(ui->lineEdit_6,76,40); ui->lineEdit_6->setText("0.00");
    adjustFontSize(ui->lineEdit_7,76,24); ui->lineEdit_7->setText("ABCDEFG");

    initCurveChart();
    initHistoryUI();
    loadHistoryFromFile();
}

void frmView1::adjustFontSize(QLineEdit* lineEdit, int h, int fontSize) {
    if (!lineEdit) return;
    lineEdit->setFixedHeight(h);
    if (lineEdit->minimumWidth() > 0) { lineEdit->setFixedWidth(lineEdit->minimumWidth()); }
    QString currentStyle = lineEdit->styleSheet();
    lineEdit->setStyleSheet(currentStyle + "QLineEdit { padding: 0px; margin: 0px; }");
    QFont font = lineEdit->font();
    font.setPixelSize(fontSize);
    font.setBold(true);
    lineEdit->setFont(font);
}

void frmView1::onUpdateRawImage(const DualCameraChunk &chunk) {
    try {
        if (chunk.imgLeft.IsInitialized()) {
            HTuple w, h; HalconCpp::GetImageSize(chunk.imgLeft, &w, &h);
            HalconCpp::SetPart(winHandle_cam1_ori, 0, 0, h - 1, w - 1);
            HalconCpp::DispObj(chunk.imgLeft, winHandle_cam1_ori);
        }
        if (chunk.imgRight.IsInitialized()) {
            HTuple w, h; HalconCpp::GetImageSize(chunk.imgRight, &w, &h);
            HalconCpp::SetPart(winHandle_cam2_ori, 0, 0, h - 1, w - 1);
            HalconCpp::DispObj(chunk.imgRight, winHandle_cam2_ori);
        }
    } catch (...) {}
}

void frmView1::refreshDataDisp(void) { ((MyApplication *) qApp)->pNodeData->isDispProcessing = false; }
void frmView1::startDispRefresh() {
    MyApplication *pApp = (MyApplication *) qApp;
    for (int i = 0; i < pApp->m_workstationList.size(); ++i) pApp->m_workstationList[i]->StartTrigger();
}

void frmView1::initCurveChart() {
    m_currentRowCount = 0;
    ui->customPlot_width->setBackground(QBrush(QColor(30, 30, 30)));
    ui->customPlot_width->axisRect()->setBackground(QBrush(QColor(20, 20, 20)));
    ui->customPlot_width->addGraph();

    QPen graphPen; graphPen.setColor(QColor(0, 255, 0)); graphPen.setWidth(2);
    graphPen.setJoinStyle(Qt::RoundJoin); graphPen.setCapStyle(Qt::RoundCap);
    ui->customPlot_width->graph(0)->setPen(graphPen);
    ui->customPlot_width->graph(0)->setBrush(QBrush(QColor(0, 255, 0, 40)));
    ui->customPlot_width->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->customPlot_width->setNotAntialiasedElements(QCP::aeNone);

    ui->customPlot_width->xAxis->setLabelColor(Qt::white); ui->customPlot_width->xAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->xAxis->setBasePen(QPen(Qt::white)); ui->customPlot_width->xAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setLabel("纵向物理行数 (Row)");
    ui->customPlot_width->yAxis->setLabelColor(Qt::white); ui->customPlot_width->yAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->yAxis->setBasePen(QPen(Qt::white)); ui->customPlot_width->yAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setLabel("宽度尺寸 (mm)");

    ui->customPlot_width->xAxis->setRange(0, 2000);
    ui->customPlot_width->yAxis->setRange(1000, 2500);
    ui->customPlot_width->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void frmView1::addCurvePoints(const QVector<double> &widths) {
    if (widths.isEmpty()) return;
    for (int i = 0; i < widths.size(); ++i) {
        m_currentRowCount++;
        m_vecFrameIndex.append(m_currentRowCount);
        m_vecWidthValue.append(widths[i]);
    }
    ui->customPlot_width->graph(0)->setData(m_vecFrameIndex, m_vecWidthValue);
    if (m_currentRowCount > ui->customPlot_width->xAxis->range().upper) {
        ui->customPlot_width->xAxis->setRange(m_currentRowCount - 2000, m_currentRowCount + 100);
    }
    ui->customPlot_width->graph(0)->rescaleValueAxis(false, false);
    ui->customPlot_width->yAxis->scaleRange(1.5);
    ui->customPlot_width->replot();
}

void frmView1::clearCurveChart() {
    m_currentRowCount = 0; m_vecFrameIndex.clear(); m_vecWidthValue.clear();
    ui->customPlot_width->graph(0)->data()->clear();
    ui->customPlot_width->xAxis->setRange(0, 2000);
    ui->customPlot_width->replot();
}

void frmView1::resetFusion() { m_hFusedImage.Clear(); m_isFirstFrame = true; }

void frmView1::addFrameToFusion(HalconCpp::HObject newFrame) {
    try {
        if (m_isFirstFrame) { m_hFusedImage = newFrame; m_isFirstFrame = false; }
        else { HalconCpp::HObject temp; HalconCpp::ConcatObj(m_hFusedImage, newFrame, &temp); m_hFusedImage = temp; }

        HalconCpp::HObject fusedImage, imageRotated;
        HalconCpp::TileImages(m_hFusedImage, &fusedImage, 1, "vertical");
        HalconCpp::RotateImage(fusedImage, &imageRotated, 90, "constant");

        HTuple imgW, imgH; HalconCpp::GetImageSize(imageRotated, &imgW, &imgH);
        int winW = ui->gView_fusion->width(), winH = ui->gView_fusion->height();
        if (winW <= 0 || winH <= 0) return;

        double winRatio = static_cast<double>(winH) / winW, imgRatio = static_cast<double>(imgH[0].D()) / imgW[0].D();
        double row1 = 0, col1 = 0, row2 = imgH[0].D() - 1.0, col2 = imgW[0].D() - 1.0;

        if (imgRatio > winRatio) {
            double partW = imgH[0].D() / winRatio; double diffW = partW - imgW[0].D();
            col1 = -diffW / 2.0; col2 = imgW[0].D() - 1.0 + diffW / 2.0;
        } else {
            double partH = imgW[0].D() * winRatio; double diffH = partH - imgH[0].D();
            row1 = -diffH / 2.0; row2 = imgH[0].D() - 1.0 + diffH / 2.0;
        }

        HalconCpp::SetPart(winHandle_fusion, (Hlong)std::floor(row1), (Hlong)std::floor(col1), (Hlong)std::ceil(row2), (Hlong)std::ceil(col2));
        HalconCpp::SetColor(winHandle_fusion, "white");
        // 💡 关键：外扩 20 像素填充，死死封住黑边
        HalconCpp::DispRectangle1(winHandle_fusion, row1 - 20.0, col1 - 20.0, row2 + 20.0, col2 + 20.0);
        HalconCpp::DispObj(imageRotated, winHandle_fusion);
    } catch (...) {}
}

// =========================================================================================
// 🚀 终极完美重构：精确局部留白，彻底解决全景大图纵向背景冗余导致的侧边白边问题
// =========================================================================================
void frmView1::onMeasureReady(const WidthResult &res)
{
    // 💡【核心控制】：在这里随意修改你想在头尾特写中显示的绝对行数
    int SHOW_ROWS = 1500;

    // 静态变量：使用数组精确记录整条钢板每一帧的真实边缘坐标，用于局部精准裁剪
    static QVector<int> s_leftEdges;
    static QVector<int> s_rightEdges;
    static int s_preBufferRows = 0;
    static int s_headStartRow = 0;

    auto displayImageProportional = [](HalconCpp::HObject img, HalconCpp::HTuple winHandle, int winW, int winH, QString bgColor) {
        if (!img.IsInitialized() || winW <= 0 || winH <= 0) return;
        try {
            HalconCpp::HTuple imgW, imgH; HalconCpp::GetImageSize(img, &imgW, &imgH);
            double winRatio = static_cast<double>(winH) / winW, imgRatio = static_cast<double>(imgH[0].D()) / imgW[0].D();
            double row1 = 0, col1 = 0, row2 = imgH[0].D() - 1.0, col2 = imgW[0].D() - 1.0;

            if (imgRatio > winRatio) {
                double partW = imgH[0].D() / winRatio; double diffW = partW - imgW[0].D();
                col1 = -diffW / 2.0; col2 = imgW[0].D() - 1.0 + diffW / 2.0;
            } else {
                double partH = imgW[0].D() * winRatio; double diffH = partH - imgH[0].D();
                row1 = -diffH / 2.0; row2 = imgH[0].D() - 1.0 + diffH / 2.0;
            }

            HalconCpp::SetPart(winHandle, (Hlong)std::floor(row1), (Hlong)std::floor(col1), (Hlong)std::ceil(row2), (Hlong)std::ceil(col2));
            HalconCpp::SetColor(winHandle, bgColor.toLocal8Bit().constData());
            // 💡 关键：同样外扩 20 像素，防止白边缩进露出黑色
            HalconCpp::DispRectangle1(winHandle, row1 - 20.0, col1 - 20.0, row2 + 20.0, col2 + 20.0);
            HalconCpp::DispObj(img, winHandle);
        } catch (...) { }
    };

    if (res.isValid) {
        ui->lineEdit->setText(QString::number(res.widthValue, 'f', 2));
    } else {
        ui->lineEdit->setText("0.00");
    }

    // 🌟 局部结算函数：钢板完全离厂或被强行截断时触发
    auto finishCurrentPlate = [&]() {
        m_isPlatePresent = false;

        int globalMinLeft = 99999, globalMaxRight = -1;
        int localHeadMinLeft = 99999, localHeadMaxRight = -1;
        int localTailMinLeft = 99999, localTailMaxRight = -1;

        int totalFrames = s_leftEdges.size();
        int framesToLook = std::min(totalFrames, static_cast<int>(std::ceil(SHOW_ROWS / 380.0)));

        for (int i = 0; i < totalFrames; ++i) {
            if (s_leftEdges[i] != -1 && s_leftEdges[i] < globalMinLeft) globalMinLeft = s_leftEdges[i];
            if (s_rightEdges[i] != -1 && s_rightEdges[i] > globalMaxRight) globalMaxRight = s_rightEdges[i];

            if (i < framesToLook) {
                if (s_leftEdges[i] != -1 && s_leftEdges[i] < localHeadMinLeft) localHeadMinLeft = s_leftEdges[i];
                if (s_rightEdges[i] != -1 && s_rightEdges[i] > localHeadMaxRight) localHeadMaxRight = s_rightEdges[i];
            }

            if (i >= totalFrames - framesToLook) {
                if (s_leftEdges[i] != -1 && s_leftEdges[i] < localTailMinLeft) localTailMinLeft = s_leftEdges[i];
                if (s_rightEdges[i] != -1 && s_rightEdges[i] > localTailMaxRight) localTailMaxRight = s_rightEdges[i];
            }
        }

        if (localHeadMinLeft == 99999) localHeadMinLeft = globalMinLeft;
        if (localHeadMaxRight == -1) localHeadMaxRight = globalMaxRight;
        if (localTailMinLeft == 99999) localTailMinLeft = globalMinLeft;
        if (localTailMaxRight == -1) localTailMaxRight = globalMaxRight;

        if (m_validFrameCount > 0) {
            double avgWidth = m_sumWidth / m_validFrameCount;
            double totalLength = m_totalRows * 1.0;

            if (totalLength >= 1000.0 && totalLength >= avgWidth) {
                ui->lineEdit_4->setText(QString::number(totalLength, 'f', 2));
                ui->lineEdit_3->setText(QString::number(avgWidth, 'f', 2));
                ui->lineEdit_8->setText(QString::number(m_maxWidth, 'f', 2));
                ui->lineEdit_9->setText(QString::number(m_minWidth, 'f', 2));

                QString plateID = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
                addHistoryRecord(plateID, totalLength, 0.0, 0.0, avgWidth, m_maxWidth, m_minWidth);

                // 🌟 同步更新 1：【头部窗口终极刷新】
                if (m_hFusedImage.IsInitialized() && localHeadMinLeft != 99999 && localHeadMaxRight != -1) {
                    try {
                        HalconCpp::HObject fusedTemp, croppedFront, rotatedFront;
                        HalconCpp::TileImages(m_hFusedImage, &fusedTemp, 1, "vertical");
                        HTuple tW, tH; HalconCpp::GetImageSize(fusedTemp, &tW, &tH);

                        int leftX = std::max(0, localHeadMinLeft - 100);
                        int rightX = std::min(static_cast<int>(tW[0].I() - 1), localHeadMaxRight + 100);
                        int cropW = rightX - leftX + 1;

                        if (cropW > 0) {
                            int rowsToCrop = std::min(static_cast<int>(tH[0].I() - s_headStartRow), SHOW_ROWS);
                            if (rowsToCrop > 0) {
                                HalconCpp::CropPart(fusedTemp, &croppedFront, s_headStartRow, leftX, cropW, rowsToCrop);
                                HalconCpp::RotateImage(croppedFront, &rotatedFront, 90, "constant");
                                displayImageProportional(rotatedFront, winHandle_front, ui->graphicsView_front->width(), ui->graphicsView_front->height(), "white");
                                m_headDisplayed = true;
                            }
                        }
                    } catch (...) {}
                }

                // 🌟 同步更新 2：【尾部窗口终极刷新】
                if (m_hFusedImage.IsInitialized() && localTailMinLeft != 99999 && localTailMaxRight != -1) {
                    try {
                        HalconCpp::HObject fusedTemp, croppedBack, rotatedBack;
                        HalconCpp::TileImages(m_hFusedImage, &fusedTemp, 1, "vertical");
                        HTuple tW, tH; HalconCpp::GetImageSize(fusedTemp, &tW, &tH);

                        int leftX = std::max(0, localTailMinLeft - 100);
                        int rightX = std::min(static_cast<int>(tW[0].I() - 1), localTailMaxRight + 100);
                        int cropW = rightX - leftX + 1;

                        if (cropW > 0) {
                            int physicalEndRow = tH[0].I() - (m_emptyFrameCount * 380);
                            int tailEndRow = std::min(static_cast<int>(tH[0].I()), physicalEndRow + 100);

                            int startRow = std::max(0, tailEndRow - SHOW_ROWS);
                            int tailRows = tailEndRow - startRow;

                            HalconCpp::CropPart(fusedTemp, &croppedBack, startRow, leftX, cropW, tailRows);
                            HalconCpp::RotateImage(croppedBack, &rotatedBack, 90, "constant");
                            displayImageProportional(rotatedBack, winHandle_back, ui->graphicsView_back->width(), ui->graphicsView_back->height(), "white");
                        }
                    } catch (...) {}
                }

                // 🌟 同步更新 3：【整板全景窗口终极刷新】(💡核心修复区)
                if (m_hFusedImage.IsInitialized() && globalMinLeft != 99999 && globalMaxRight != -1) {
                    try {
                        HalconCpp::HObject fusedFull, croppedFused, imageRotated;
                        HalconCpp::TileImages(m_hFusedImage, &fusedFull, 1, "vertical");
                        HTuple fW, fH; HalconCpp::GetImageSize(fusedFull, &fW, &fH);

                        // 横向定位：左右各留 100 像素
                        int leftX = std::max(0, globalMinLeft - 100);
                        int rightX = std::min(static_cast<int>(fW[0].I() - 1), globalMaxRight + 100);
                        int cropW = rightX - leftX + 1;

                        // 💡 修复纵向定位：头尾各留 100 像素！彻底干掉多余的皮带背景
                        int topY = std::max(0, s_preBufferRows - 100);
                        int physicalEndRow = fH[0].I() - (m_emptyFrameCount * 380);
                        int bottomY = std::min(static_cast<int>(fH[0].I() - 1), physicalEndRow + 100);
                        int cropH = bottomY - topY + 1;

                        if (cropW > 0 && cropH > 0) {
                            // 使用完整的 x,y 坐标和 w,h 宽高进行终极裁切
                            HalconCpp::CropPart(fusedFull, &croppedFused, topY, leftX, cropW, cropH);
                            HalconCpp::RotateImage(croppedFused, &imageRotated, 90, "constant");
                            displayImageProportional(imageRotated, winHandle_fusion, ui->gView_fusion->width(), ui->gView_fusion->height(), "white");
                        }
                    } catch (...) {}
                }
            } else {
                clearCurveChart();
                resetFusion();
            }
        }
    };

    if (res.isValid && m_isPlatePresent && m_emptyFrameCount > 0) {
        finishCurrentPlate();
        m_emptyFrameCount = 0;
    }

    if (res.isValid) {
        s_leftEdges.append(res.renderLeftX);
        s_rightEdges.append(res.renderRightX);

        if (!m_isPlatePresent) {
            clearCurveChart();
            resetFusion();
            m_isPlatePresent = true;
            m_headDisplayed = false;
            m_sumWidth = 0.0;
            m_validFrameCount = 0;
            m_totalRows = 0;
            m_maxWidth = 0.0;
            m_minWidth = 99999.0;

            s_leftEdges.clear();
            s_leftEdges.append(res.renderLeftX);
            s_rightEdges.clear();
            s_rightEdges.append(res.renderRightX);

            s_preBufferRows = 0;
            for (int i = 0; i < m_preBufferList.size(); ++i) {
                HalconCpp::HTuple w, h; HalconCpp::GetImageSize(m_preBufferList.at(i), &w, &h);
                s_preBufferRows += h[0].I();
                addFrameToFusion(m_preBufferList.at(i));
            }
            m_preBufferList.clear();

            s_headStartRow = std::max(0, s_preBufferRows - 100);
        }

        m_sumWidth += res.widthValue;
        m_validFrameCount++;

        if (res.widthValue > m_maxWidth) m_maxWidth = res.widthValue;
        if (res.widthValue < m_minWidth) m_minWidth = res.widthValue;

        if (res.dispImage.IsInitialized()) {
            m_hLastValidImage = res.dispImage;
            HalconCpp::HTuple w, h; HalconCpp::GetImageSize(res.dispImage, &w, &h);
            m_totalRows += h[0].I();
            addFrameToFusion(res.dispImage);
        }

        // 【运行途中头部的实时确认】
        if (!m_headDisplayed && (s_preBufferRows + m_totalRows >= s_headStartRow + SHOW_ROWS) && m_hFusedImage.IsInitialized()) {
            try {
                int localHeadMinLeft = 99999, localHeadMaxRight = -1;
                for (int i = 0; i < s_leftEdges.size(); ++i) {
                    if (s_leftEdges[i] != -1 && s_leftEdges[i] < localHeadMinLeft) localHeadMinLeft = s_leftEdges[i];
                    if (s_rightEdges[i] != -1 && s_rightEdges[i] > localHeadMaxRight) localHeadMaxRight = s_rightEdges[i];
                }

                HalconCpp::HObject fusedTemp, croppedFront, rotatedFront;
                HalconCpp::TileImages(m_hFusedImage, &fusedTemp, 1, "vertical");
                HTuple tW, tH; HalconCpp::GetImageSize(fusedTemp, &tW, &tH);

                int leftX = std::max(0, localHeadMinLeft - 100);
                int rightX = std::min(static_cast<int>(tW[0].I() - 1), localHeadMaxRight + 100);
                int cropW = rightX - leftX + 1;

                if (cropW > 0) {
                    int rowsToCrop = std::min(static_cast<int>(tH[0].I() - s_headStartRow), SHOW_ROWS);
                    if (rowsToCrop > 0) {
                        HalconCpp::CropPart(fusedTemp, &croppedFront, s_headStartRow, leftX, cropW, rowsToCrop);
                        HalconCpp::RotateImage(croppedFront, &rotatedFront, 90, "constant");
                        displayImageProportional(rotatedFront, winHandle_front, ui->graphicsView_front->width(), ui->graphicsView_front->height(), "black");
                        m_headDisplayed = true;
                    }
                }
            } catch (...) {}
        }

        // 💡 修复：恢复高密度逐行曲线绘制！
        addCurvePoints(res.rowWidths);
        m_emptyFrameCount = 0;

    } else {
        if (m_isPlatePresent) {
            m_emptyFrameCount++;
            if (res.dispImage.IsInitialized() && m_emptyFrameCount <= EMPTY_FRAME_LIMIT) {
                HalconCpp::HTuple w, h; HalconCpp::GetImageSize(res.dispImage, &w, &h);
                m_totalRows += h[0].I();
                addFrameToFusion(res.dispImage);
            }
            if (m_emptyFrameCount >= EMPTY_FRAME_LIMIT) {
                finishCurrentPlate();
            }
        } else {
            if (res.dispImage.IsInitialized()) {
                m_preBufferList.append(res.dispImage);
                while (m_preBufferList.size() > HEAD_FRAME_COUNT) {
                    m_preBufferList.removeFirst();
                }
            }
        }
    }

    if (res.dispImage.IsInitialized()) {
        try {
            HalconCpp::HTuple currentWin = this->winHandle_cam1_pro;
            HalconCpp::HTuple imgW, imgH; HalconCpp::GetImageSize(res.dispImage, &imgW, &imgH);
            HalconCpp::SetPart(currentWin, 0, 0, imgH - 1, imgW - 1);
            HalconCpp::DispObj(res.dispImage, currentWin);

            if (res.isValid && res.renderLeftX != -1) {
                HalconCpp::SetLineWidth(currentWin, 2);
                HalconCpp::SetColor(currentWin, "green"); HalconCpp::DispLine(currentWin, 0, res.renderLeftX, imgH - 1, res.renderLeftX);
                HalconCpp::SetColor(currentWin, "cyan"); HalconCpp::DispLine(currentWin, 0, res.renderRightX, imgH - 1, res.renderRightX);
                HalconCpp::SetColor(currentWin, "red"); HalconCpp::DispCross(currentWin, res.renderY, res.renderLeftX, 60, 0);
                HalconCpp::DispCross(currentWin, res.renderY, res.renderRightX, 60, 0);
            } else {
                HalconCpp::DispText(currentWin, "NO PLATE DETECTED", "window", "center", "center", "red", HalconCpp::HTuple(), HalconCpp::HTuple());
            }
        } catch (...) {}
    }
}

void frmView1::initHistoryUI() {
    m_tableModelHistory = new QStandardItemModel(this);
    QStringList headers = {"板号", "长度(mm)", "厚度(mm)", "设定宽度(mm)", "平均宽度(mm)", "最大宽度(mm)", "最小宽度(mm)"};
    m_tableModelHistory->setHorizontalHeaderLabels(headers);
    ui->tableView_history->setModel(m_tableModelHistory);
    ui->tableView_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_history->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QFont headerFont = ui->tableView_history->horizontalHeader()->font();
    headerFont.setPointSize(14); headerFont.setBold(true);
    ui->tableView_history->horizontalHeader()->setFont(headerFont);
    ui->tableView_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QFont tableFont = ui->tableView_history->font();
    tableFont.setPointSize(16); ui->tableView_history->setFont(tableFont);
    QFont verticalFont = ui->tableView_history->verticalHeader()->font();
    verticalFont.setPointSize(14); ui->tableView_history->verticalHeader()->setFont(verticalFont);
}

void frmView1::saveHistoryToFile() {
    // 1. 使用 QStandardPaths 获取 AppData 目录，保证有写入权限
    QString folderPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(folderPath);
    if (!dir.exists()) {
        dir.mkpath("."); // 2. 如果目录不存在，自动创建
    }

    QString filePath = folderPath + "/history_data.dat";
    QFile file(filePath);

    // 3. 打开文件并增加错误排查
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "错误: 无法保存历史数据到" << filePath << " 原因:" << file.errorString();
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_0);
    out << m_historyList;
    file.close();
    qDebug() << "历史数据已成功保存至:" << filePath;
}

void frmView1::loadHistoryFromFile() {
    // 保持路径一致性
    QString folderPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = folderPath + "/history_data.dat";
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_0);
        in >> m_historyList;
        file.close();
        qDebug() << "历史数据已成功加载:" << filePath;
    } else {
        qDebug() << "未找到历史数据文件，将创建新文件:" << filePath;
    }
    updateHistoryTable();
}

void frmView1::addHistoryRecord(const QString& plateID, double length, double thickness, double targetW, double avgW, double maxW, double minW) {
    PlateRecord newRecord{plateID, length, thickness, targetW, avgW, maxW, minW};
    m_historyList.prepend(newRecord);
    while (m_historyList.size() > 5) m_historyList.removeLast();
    saveHistoryToFile(); updateHistoryTable();
}

void frmView1::updateHistoryTable() {
    m_tableModelHistory->setRowCount(0);
    for (int i = 0; i < m_historyList.size(); ++i) {
        const PlateRecord &rec = m_historyList.at(i);
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(rec.plateID) << new QStandardItem(QString::number(rec.length, 'f', 1))
                 << new QStandardItem(QString::number(rec.thickness, 'f', 2)) << new QStandardItem(QString::number(rec.targetWidth, 'f', 1))
                 << new QStandardItem(QString::number(rec.avgWidth, 'f', 2)) << new QStandardItem(QString::number(rec.maxWidth, 'f', 2))
                 << new QStandardItem(QString::number(rec.minWidth, 'f', 2));
        for(auto item : rowItems) item->setTextAlignment(Qt::AlignCenter);
        m_tableModelHistory->appendRow(rowItems);
    }
}