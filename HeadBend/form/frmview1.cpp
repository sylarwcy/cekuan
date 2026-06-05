#include "frmview1.h"
#include "ui_frmview1.h"
#include "MyApplication.h"
#include <algorithm>
#include <QSpinBox>
#include <QLabel>
#include <QStandardPaths>
#include <QDir>
#include <cmath>

frmView1::frmView1(QWidget *parent) : QWidget(parent), ui(new Ui::frmView1) {
    ui->setupUi(this);
    initForm();
    DatabaseManager::instance().initDatabase();
    m_historyList = DatabaseManager::instance().loadHistoryFromDb(5);
    updateHistoryTable();

    // 初始化后台网络物理隔离子线程服务
    m_pMqttThread = new QThread(this);
    m_pMqttWorker = new WorkerMQTT();
    m_pMqttWorker->init(WorkStation_DATA());
    m_pMqttWorker->moveToThread(m_pMqttThread);

    connect(this, &frmView1::sig_postPlateReportToWeb, m_pMqttWorker, &WorkerMQTT::slot_uploadAndSendReport, Qt::QueuedConnection);
    connect(m_pMqttThread, &QThread::started, m_pMqttWorker, &WorkerMQTT::startConnectMqtt);
    connect(m_pMqttThread, &QThread::finished, m_pMqttWorker, &QObject::deleteLater);

    m_pMqttThread->start();

    QTimer::singleShot(1000, this, SLOT(varInit()));
}

frmView1::~frmView1() {
    if (m_pMqttThread) {
        m_pMqttThread->quit();
        m_pMqttThread->wait(2000);
    }
    delete ui;
    if (p_camera_front) { p_camera_front->StopGrabbing(); p_camera_front->CloseCamera(); delete p_camera_front; }
    if (p_camera_back) { p_camera_back->StopGrabbing(); p_camera_back->CloseCamera(); delete p_camera_back; }
}

void frmView1::loadHistoryFromDb() {
    if (!QSqlDatabase::database().isOpen()) return;
    QSqlQuery query;
    query.prepare("SELECT plateID, length, thickness, targetWidth, avgWidth, maxWidth, minWidth "
        "FROM PlateRecord ORDER BY id DESC LIMIT 5");
    if (!query.exec()) return;

    m_historyList.clear();
    while (query.next()) {
        PlateRecord rec;
        rec.plateID = query.value(0).toString();
        rec.length = query.value(1).toDouble();
        rec.thickness = query.value(2).toDouble();
        rec.targetWidth = query.value(3).toDouble();
        rec.avgWidth = query.value(4).toDouble();
        rec.maxWidth = query.value(5).toDouble();
        rec.minWidth = query.value(6).toDouble();
        m_historyList.append(rec);
    }
    updateHistoryTable();
}

void frmView1::initDatabase() {
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = db.addDatabase("QSQLITE");
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/history_data.db");
    }
    if (!db.open()) return;

    QSqlQuery query;
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS PlateRecord (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            plateID TEXT,
            length REAL,
            thickness REAL,
            targetWidth REAL,
            avgWidth REAL,
            maxWidth REAL,
            minWidth REAL,
            measureTime DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    query.exec(createTableSql);
}

void frmView1::resizeEvent(QResizeEvent *event) {
    static unsigned long tri_num;
    tri_num++;
    if (tri_num > 1) {
        SetWindowExtents(winHandle_cam1_ori, 0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height());
        SetWindowExtents(winHandle_cam2_ori, 0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height());
        SetWindowExtents(winHandle_cam1_pro, 0, 0, ui->gView_back_ori->width(), ui->gView_back_ori->height());
        if (winHandle_lunkuo.Length() > 0) {
            SetWindowExtents(winHandle_lunkuo, 0, 0, ui->gView_lunkuo->width(), ui->gView_lunkuo->height());
        }
    }
}

void frmView1::logSlot(const QString &message, int level) { ui->textBrowser->append(qUtf8Printable(message)); }

void frmView1::varInit() {
    MyApplication *pApp = (MyApplication *) qApp;

    Hlong winId_front_ori = (Hlong) ui->gView_front_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height(), winId_front_ori, "visible", "",
               &winHandle_cam1_ori);

    Hlong winId_front_pro = (Hlong) ui->gView_front_pro->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height(), winId_front_pro, "visible", "",
               &winHandle_cam1_pro);

    Hlong winId_back_ori = (Hlong) ui->gView_back_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_back_ori->width(), ui->gView_back_ori->height(), winId_back_ori, "visible", "",
               &winHandle_cam2_ori);

    Hlong winId_fusion = (Hlong) ui->gView_fusion->winId();
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->gView_fusion->width(), ui->gView_fusion->height(), winId_fusion, "visible", "",
                          &winHandle_fusion);

    Hlong winId_lunkuo = (Hlong) ui->gView_lunkuo->winId();
    HalconCpp::SetWindowAttr("background_color", "black");
    HalconCpp::OpenWindow(0, 0, ui->gView_lunkuo->width(), ui->gView_lunkuo->height(), winId_lunkuo, "visible", "",
                          &winHandle_lunkuo);

    m_isFirstFrame = true;
    pApp->pNodeData->camera_1_para.winHandle_ori = winHandle_cam1_ori;
    pApp->pNodeData->camera_2_para.winHandle_ori = winHandle_cam2_ori;

    connect(pApp->m_workstationList[0], &Workstation::sigForwardToView, this, &frmView1::onUpdateRawImage);

    ui->gView_front_ori->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_front_pro->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_back_ori->setAttribute(Qt::WA_OpaquePaintEvent);

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

    m_isPlatePresent = false;
    m_emptyFrameCount = 0;

    adjustFontSize(ui->lineEdit, 76, 40);
    adjustFontSize(ui->lineEdit_3, 35, 26);
    adjustFontSize(ui->lineEdit_4, 35, 26);
    adjustFontSize(ui->lineEdit_8, 35, 26);
    adjustFontSize(ui->lineEdit_9, 35, 26);
    adjustFontSize(ui->lineEdit_5, 76, 26);
    ui->lineEdit_5->setText("0.00");
    adjustFontSize(ui->lineEdit_6, 76, 26);
    ui->lineEdit_6->setText("0.00");
    adjustFontSize(ui->lineEdit_7, 76, 24);
    ui->lineEdit_7->setText("ABCD");

    initCurveChart();
    initHistoryUI();

    QHBoxLayout *frameSetLayout = ui->frame_setValue->findChild<QHBoxLayout *>("frame_set");
    if (frameSetLayout) {
        QWidget *segWidget = new QWidget(ui->frame_setValue);
        QHBoxLayout *segLay = new QHBoxLayout(segWidget);
        segLay->setContentsMargins(0, 0, 0, 0);

        QLabel *lblSeg = new QLabel("曲线\n分段倍率:", segWidget);
        lblSeg->setStyleSheet("font-family: 'Alibaba PuHuiTi'; font-size: 12pt; color: black;");
        lblSeg->setAlignment(Qt::AlignCenter);

        QSpinBox *spinSeg = new QSpinBox(segWidget);
        spinSeg->setObjectName("spinSegment");
        spinSeg->setRange(1, 5);
        spinSeg->setValue(3);
        spinSeg->setFixedSize(131, 35);
        spinSeg->setAlignment(Qt::AlignCenter);
        spinSeg->setStyleSheet(
            "QSpinBox { font-family: 'Alibaba PuHuiTi'; font-size: 14pt; font-weight: bold; background: white; color: black; border: none; }");

        connect(spinSeg, QOverload<int>::of(&QSpinBox::valueChanged), this, &frmView1::onMultiplierChanged);

        segLay->addWidget(lblSeg);
        segLay->addWidget(spinSeg);
        frameSetLayout->addWidget(segWidget);
    }
}

void frmView1::initCurveChart() {
    m_currentRowCount = 0;
    ui->customPlot_width->setBackground(QBrush(QColor(30, 30, 30)));
    ui->customPlot_width->axisRect()->setBackground(QBrush(QColor(20, 20, 20)));

    ui->customPlot_width->addGraph();
    QPen greenPen;
    greenPen.setColor(QColor(0, 255, 0));
    greenPen.setWidth(1.5);
    ui->customPlot_width->graph(0)->setPen(greenPen);
    ui->customPlot_width->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor(0, 255, 0), 4));

    ui->customPlot_width->addGraph();
    QPen whitePen;
    whitePen.setColor(Qt::white);
    whitePen.setWidth(2);
    ui->customPlot_width->graph(1)->setPen(whitePen);
    ui->customPlot_width->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::red, Qt::red, 6));

    ui->customPlot_width->setNotAntialiasedElements(QCP::aeNone);
    ui->customPlot_width->xAxis->ticker()->setTickCount(20);
    ui->customPlot_width->yAxis->ticker()->setTickCount(15);

    ui->customPlot_width->xAxis->grid()->setSubGridVisible(true);
    ui->customPlot_width->yAxis->grid()->setSubGridVisible(true);
    QPen subGridPen(QColor(255, 255, 255, 30), 1, Qt::DotLine);
    ui->customPlot_width->xAxis->grid()->setSubGridPen(subGridPen);
    ui->customPlot_width->yAxis->grid()->setSubGridPen(subGridPen);

    ui->customPlot_width->xAxis->setLabelColor(Qt::white);
    ui->customPlot_width->xAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->xAxis->setBasePen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->xAxis->setLabel("测量位置点");

    ui->customPlot_width->yAxis->setLabelColor(Qt::white);
    ui->customPlot_width->yAxis->setTickLabelColor(Qt::white);
    ui->customPlot_width->yAxis->setBasePen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setTickPen(QPen(Qt::white));
    ui->customPlot_width->yAxis->setLabel("钢板真实宽度 (mm)");
}

void frmView1::adjustFontSize(QLineEdit *lineEdit, int h, int fontSize) {
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
            HTuple w, h;
            HalconCpp::GetImageSize(chunk.imgLeft, &w, &h);
            HalconCpp::SetPart(winHandle_cam1_ori, 0, 0, h - 1, w - 1);
            HalconCpp::DispObj(chunk.imgLeft, winHandle_cam1_ori);
        }
        if (chunk.imgRight.IsInitialized()) {
            HTuple w, h;
            HalconCpp::GetImageSize(chunk.imgRight, &w, &h);
            HalconCpp::SetPart(winHandle_cam2_ori, 0, 0, h - 1, w - 1);
            HalconCpp::DispObj(chunk.imgRight, winHandle_cam2_ori);
        }
    } catch (...) {
    }
}

void frmView1::refreshDataDisp(void) { ((MyApplication *) qApp)->pNodeData->isDispProcessing = false; }

void frmView1::startDispRefresh() {
    MyApplication *pApp = (MyApplication *) qApp;
    for (int i = 0; i < pApp->m_workstationList.size(); ++i) pApp->m_workstationList[i]->StartTrigger();
}

void frmView1::addCurvePoints(const QVector<double> &widths) {
    if (widths.isEmpty()) return;
    int multiplier = 1;
    QSpinBox *spin = this->findChild<QSpinBox *>("spinSegment");
    if (spin) multiplier = spin->value();

    for (int i = 0; i < widths.size(); ++i) {
        m_currentRowCount++;
        m_realtimeWidths.append(widths[i]);
        m_vecFrameIndex.append(m_currentRowCount * multiplier);
        m_vecWidthValue.append(widths[i]);
    }
    ui->customPlot_width->graph(0)->setData(m_vecFrameIndex, m_vecWidthValue);

    if (m_currentRowCount * multiplier > ui->customPlot_width->xAxis->range().upper) {
        ui->customPlot_width->xAxis->setRange(0, m_currentRowCount * multiplier + 5);
    }
    ui->customPlot_width->graph(0)->rescaleValueAxis(false, false);
    ui->customPlot_width->yAxis->scaleRange(5);
    ui->customPlot_width->replot();
}

void frmView1::clearCurveChart() {
    m_currentRowCount = 0;
    m_vecFrameIndex.clear();
    m_vecWidthValue.clear();
    m_realtimeWidths.clear();
    m_correctedGlobalWidths.clear();
    m_globalPhysicalWidths.clear();

    ui->customPlot_width->graph(0)->data()->clear();
    if (ui->customPlot_width->graphCount() > 1) {
        ui->customPlot_width->graph(1)->data()->clear();
    }
    ui->customPlot_width->xAxis->setRange(0, 30);
    ui->customPlot_width->replot();
}

void frmView1::onMultiplierChanged(int value) {
    if (m_isPlatePresent) {
        m_vecFrameIndex.clear();
        for (int i = 0; i < m_realtimeWidths.size(); ++i) {
            m_vecFrameIndex.append((i + 1) * value);
        }
        ui->customPlot_width->graph(0)->setData(m_vecFrameIndex, m_realtimeWidths);
        ui->customPlot_width->xAxis->setRange(0, m_realtimeWidths.size() * value + 5);
        ui->customPlot_width->replot();
    } else {
        if (!m_correctedGlobalWidths.isEmpty()) {
            updatePostProcessCurve(value);
        }
    }
}

QVector<double> frmView1::updatePostProcessCurve(int multiplier) {
    QVector<double> whiteY;
    if (m_correctedGlobalWidths.isEmpty() || m_realtimeWidths.isEmpty()) return whiteY;

    int frames = m_realtimeWidths.size();
    int totalSegments = frames * multiplier;
    int n = m_correctedGlobalWidths.size();

    double dynamicMin = 999999.0;
    double dynamicMax = 0.0;
    QVector<double> validWidthsForTrim;

    for (double w: m_correctedGlobalWidths) {
        if (w > 50.0) {
            validWidthsForTrim.append(w);
        }
    }

    if (!validWidthsForTrim.isEmpty()) {
        std::sort(validWidthsForTrim.begin(), validWidthsForTrim.end());
        int trimCount = validWidthsForTrim.size() * 0.02;
        if (trimCount < 0) trimCount = 0;
        if (trimCount * 2 < validWidthsForTrim.size()) {
            dynamicMin = validWidthsForTrim[trimCount];
            dynamicMax = validWidthsForTrim[validWidthsForTrim.size() - 1 - trimCount];
        }
    }

    QVector<double> whiteX;
    if (totalSegments > 0 && n > 0) {
        int pointsPerSeg = std::max(1, n / totalSegments);
        for (int s = 0; s < totalSegments; ++s) {
            int startIdx = s * pointsPerSeg;
            if (startIdx >= n) break;
            int endIdx = (s == totalSegments - 1) ? n : (s + 1) * pointsPerSeg;
            if (endIdx > n) endIdx = n;
            int segLen = endIdx - startIdx;
            if (segLen <= 0) break;

            QVector<double> segW;
            for (int i = startIdx; i < endIdx; ++i) { segW.append(m_correctedGlobalWidths[i]); }
            std::sort(segW.begin(), segW.end());
            double medianW = segW[segLen / 2];

            if (medianW < dynamicMin) medianW = dynamicMin;
            if (medianW > dynamicMax) medianW = dynamicMax;

            whiteX.append(s + 1);
            whiteY.append(medianW);
        }
    }

    ui->customPlot_width->graph(0)->data()->clear();
    ui->customPlot_width->graph(1)->setData(whiteX, whiteY);
    ui->customPlot_width->xAxis->setRange(0, totalSegments + 1);

    if (!whiteY.isEmpty()) {
        double minY = *std::min_element(whiteY.begin(), whiteY.end());
        double maxY = *std::max_element(whiteY.begin(), whiteY.end());
        double pad = (maxY - minY) * 0.5;
        if (pad < 5) pad = 5;
        ui->customPlot_width->yAxis->setRange(minY - pad, maxY + pad);
    }
    ui->customPlot_width->replot();
    return whiteY;
}

void frmView1::resetFusion() {
    m_hFusedImage.Clear();
    m_isFirstFrame = true;
}

void frmView1::addFrameToFusion(HalconCpp::HObject newFrame) {
    try {
        if (m_isFirstFrame) {
            m_hFusedImage = newFrame;
            m_isFirstFrame = false;
        } else {
            HalconCpp::HObject temp;
            HalconCpp::ConcatObj(m_hFusedImage, newFrame, &temp);
            m_hFusedImage = temp;
        }

        HalconCpp::HObject fusedImage, imageRotated;
        HalconCpp::TileImages(m_hFusedImage, &fusedImage, 1, "vertical");
        HalconCpp::RotateImage(fusedImage, &imageRotated, 90, "constant");

        HTuple imgW, imgH;
        HalconCpp::GetImageSize(imageRotated, &imgW, &imgH);
        int winW = ui->gView_fusion->width(), winH = ui->gView_fusion->height();
        if (winW <= 0 || winH <= 0) return;

        double winRatio = static_cast<double>(winH) / winW, imgRatio = static_cast<double>(imgH[0].D()) / imgW[0].D();
        double row1 = 0, col1 = 0, row2 = imgH[0].D() - 1.0, col2 = imgW[0].D() - 1.0;

        if (imgRatio > winRatio) {
            double partW = imgH[0].D() / winRatio;
            double diffW = partW - imgW[0].D();
            col1 = -diffW / 2.0;
            col2 = imgW[0].D() - 1.0 + diffW / 2.0;
        } else {
            double partH = imgW[0].D() * winRatio;
            double diffH = partH - imgH[0].D();
            row1 = -diffH / 2.0;
            row2 = imgH[0].D() - 1.0 + diffH / 2.0;
        }

        HalconCpp::SetPart(winHandle_fusion, (Hlong) std::floor(row1), (Hlong) std::floor(col1),
                           (Hlong) std::ceil(row2), (Hlong) std::ceil(col2));
        HalconCpp::SetColor(winHandle_fusion, "white");
        HalconCpp::DispRectangle1(winHandle_fusion, row1 - 20.0, col1 - 20.0, row2 + 20.0, col2 + 20.0);
        HalconCpp::DispObj(imageRotated, winHandle_fusion);

        HalconCpp::SetPart(winHandle_lunkuo, (Hlong) std::floor(row1), (Hlong) std::floor(col1),
                           (Hlong) std::ceil(row2), (Hlong) std::ceil(col2));
        HalconCpp::SetColor(winHandle_lunkuo, "white");
        HalconCpp::DispRectangle1(winHandle_lunkuo, row1 - 20.0, col1 - 20.0, row2 + 20.0, col2 + 20.0);

        if (m_globalContourRows.size() > 0) {
            HTuple fW, fH;
            HalconCpp::GetImageSize(fusedImage, &fW, &fH);
            int origWidth = fW[0].I();
            HTuple rotRows, rotCols;

            for (int i = 0; i < m_globalContourRows.size(); ++i) {
                rotRows.Append(origWidth - 1 - m_globalContourColsL[i]);
                rotCols.Append(m_globalContourRows[i]);
            }
            for (int i = m_globalContourRows.size() - 1; i >= 0; --i) {
                rotRows.Append(origWidth - 1 - m_globalContourColsR[i]);
                rotCols.Append(m_globalContourRows[i]);
            }
            if (rotRows.Length() > 0) {
                rotRows.Append(rotRows[0]);
                rotCols.Append(rotCols[0]);
            }

            HalconCpp::HObject closedPlateXld;
            HalconCpp::GenContourPolygonXld(&closedPlateXld, rotRows, rotCols);

            HalconCpp::SetLineWidth(winHandle_lunkuo, 4);
            HalconCpp::SetColor(winHandle_lunkuo, "red");
            HalconCpp::DispObj(closedPlateXld, winHandle_lunkuo);
        }
    } catch (...) {
    }
}

void frmView1::onMeasureReady(const WidthResult &res)
{
    static QVector<int> s_leftEdges;
    static QVector<int> s_rightEdges;
    static int s_preBufferRows = 0;

    auto displayImageProportional = [](HalconCpp::HObject img, HalconCpp::HTuple winHandle, int winW, int winH, QString bgColor, bool showImg) {
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
            HalconCpp::DispRectangle1(winHandle, row1 - 20.0, col1 - 20.0, row2 + 20.0, col2 + 20.0);
            if (showImg) { HalconCpp::DispObj(img, winHandle); }
        } catch (...) { }
    };

    if (res.isValid) { ui->lineEdit->setText(QString::number(res.widthValue, 'f', 2)); }
    else { ui->lineEdit->setText("0.00"); }

    auto finishCurrentPlate = [&]() {
        m_isPlatePresent = false;
        int globalMinLeft = 99999, globalMaxRight = -1;

        for (int i = 0; i < s_leftEdges.size(); ++i) {
            if (s_leftEdges[i] != -1 && s_leftEdges[i] < globalMinLeft) globalMinLeft = s_leftEdges[i];
            if (s_rightEdges[i] != -1 && s_rightEdges[i] > globalMaxRight) globalMaxRight = s_rightEdges[i];
        }

        if (m_validFrameCount > 0) {

            // =================================================================
            // 🌟【高级接口映射】：如果小弹窗存在，并且处于自标定激活状态下
            // =================================================================
            if (m_pCalibDlg && m_pCalibDlg->isVisible() && m_pCalibDlg->isActive()) {
                if (m_pCalibDlg->isPaused()) {
                    // 如果在弹窗里点了暂停，此反向数据包不被录入池中
                } else {
                    m_pCalibDlg->finishPass(); // 间接通知小窗当场清算当前趟偏航角
                }
                clearCurveChart(); resetFusion(); m_emptyFrameCount = 0; m_totalRows = 0; m_validFrameCount = 0;
                return; // 强行拦截常规数据库存储
            }

            int n = m_globalContourRows.size();
            double trueAvg = 0, trueMax = 0, trueMin = 999999.0;
            double cosTheta = 1.0; double sinTheta = 0.0;
            m_correctedGlobalWidths.clear();

            if (n > 0) {
                double sum_y = 0, sum_x = 0, sum_y2 = 0, sum_xy = 0;
                for (int i = 0; i < n; ++i) {
                    double y = m_globalContourRows[i]; double x = (m_globalContourColsL[i] + m_globalContourColsR[i]) / 2.0;
                    sum_y += y; sum_x += x; sum_y2 += y * y; sum_xy += x * y;
                }
                double denom = n * sum_y2 - sum_y * sum_y;
                double k = (std::abs(denom) > 1e-6) ? (n * sum_xy - sum_x * sum_y) / denom : 0.0;
                cosTheta = 1.0 / std::sqrt(1.0 + k * k); sinTheta = std::abs(k) * cosTheta;

                int validCount = 0; QVector<double> validWidthsForStats;
                for (int i = 0; i < n; ++i) {
                    if (i >= m_globalPhysicalWidths.size()) break;
                    double w_true = m_globalPhysicalWidths[i] * cosTheta;
                    m_correctedGlobalWidths.append(w_true);
                    if (w_true > 50.0) { trueAvg += w_true; validWidthsForStats.append(w_true); validCount++; }
                }

                if (validCount > 0) {
                    trueAvg /= validCount; std::sort(validWidthsForStats.begin(), validWidthsForStats.end());
                    int trimCount = validWidthsForStats.size() * 0.02;
                    trueMin = validWidthsForStats[trimCount]; trueMax = validWidthsForStats[validWidthsForStats.size() - 1 - trimCount];
                } else { trueAvg = m_sumWidth / m_validFrameCount; }
            } else { trueAvg = m_sumWidth / m_validFrameCount; trueMax = m_maxWidth; trueMin = m_minWidth; }

            double projectedLength = m_totalRows * 0.09473;
            double totalLength = (projectedLength - trueAvg * sinTheta) / cosTheta;

            if (totalLength >= 50.0) {
                ui->lineEdit_4->setText(QString::number(totalLength, 'f', 2));
                ui->lineEdit_3->setText(QString::number(trueAvg, 'f', 2));
                ui->lineEdit_8->setText(QString::number(trueMax, 'f', 2));
                ui->lineEdit_9->setText(QString::number(trueMin, 'f', 2));

                QString plateID = QDateTime::currentDateTime().toString("MMdd_HHmmss");
                addHistoryRecord(plateID, totalLength, 0.0, 0.0, trueAvg, trueMax, trueMin);

                QVector<double> finalCurveData = updatePostProcessCurve(3);

                if (m_hFusedImage.IsInitialized() && globalMinLeft != 99999 && globalMaxRight != -1) {
                    try {
                        HalconCpp::HObject fFull, croppedFused, imageRotated;
                        HalconCpp::TileImages(m_hFusedImage, &fFull, 1, "vertical");

                        HalconCpp::HTuple fW, fH; HalconCpp::GetImageSize(fFull, &fW, &fH);
                        int leftX = std::max(0, globalMinLeft - 100);
                        int rightX = std::min(static_cast<int>(fW[0].I() - 1), globalMaxRight + 100);
                        int cropW = rightX - leftX + 1;
                        int topY = std::max(0, s_preBufferRows - 380);

                        int physicalEndRow = fH[0].I() - 380;
                        if (!m_globalContourRows.isEmpty()) {
                            physicalEndRow = static_cast<int>(*std::max_element(m_globalContourRows.begin(), m_globalContourRows.end()));
                        }
                        int bottomY = std::min(static_cast<int>(fH[0].I() - 1), physicalEndRow + 100);
                        int cropH = bottomY - topY + 1;

                        if (cropW > 0 && cropH > 0) {
                            HalconCpp::CropPart(fFull, &croppedFused, topY, leftX, cropW, cropH);
                            HalconCpp::RotateImage(croppedFused, &imageRotated, 90, "constant");

                            displayImageProportional(imageRotated, winHandle_fusion, ui->gView_fusion->width(), ui->gView_fusion->height(), "white", true);
                            displayImageProportional(imageRotated, winHandle_lunkuo, ui->gView_lunkuo->width(), ui->gView_lunkuo->height(), "white", false);

                            HalconCpp::HObject ho_webContourImg;
                            HalconCpp::GenEmptyObj(&ho_webContourImg);

                            if (m_globalContourRows.size() > 0) {
                                HalconCpp::HTuple origWidth, dummyH; HalconCpp::GetImageSize(croppedFused, &origWidth, &dummyH);
                                HTuple rotRows, rotCols;

                                for(int i = 0; i < m_globalContourRows.size(); ++i) {
                                    rotRows.Append(origWidth[0].I() - 1 - (m_globalContourColsL[i] - leftX));
                                    rotCols.Append(m_globalContourRows[i] - topY);
                                }
                                for(int i = m_globalContourRows.size() - 1; i >= 0; --i) {
                                    rotRows.Append(origWidth[0].I() - 1 - (m_globalContourColsR[i] - leftX));
                                    rotCols.Append(m_globalContourRows[i] - topY);
                                }
                                if (rotRows.Length() > 0) { rotRows.Append(rotRows[0]); rotCols.Append(rotCols[0]); }

                                HalconCpp::HObject localXld;
                                HalconCpp::GenContourPolygonXld(&localXld, rotRows, rotCols);
                                HalconCpp::SetLineWidth(winHandle_lunkuo, 4); HalconCpp::SetColor(winHandle_lunkuo, "red");
                                HalconCpp::DispObj(localXld, winHandle_lunkuo);

                                HalconCpp::PaintXld(croppedFused, localXld, &ho_webContourImg, 255);
                            } else {
                                ho_webContourImg = croppedFused;
                            }

                            PlateMqttReportData webReport;
                            webReport.plateID = plateID;
                            webReport.length = totalLength;
                            webReport.thickness = 0.00;
                            webReport.targetWidth = 0.00;
                            webReport.avgWidth = trueAvg;
                            webReport.maxWidth = trueMax;
                            webReport.minWidth = trueMin;
                            webReport.ho_fusedImage = croppedFused;
                            webReport.ho_contourImage = ho_webContourImg;
                            webReport.curveValues = finalCurveData;

                            emit sig_postPlateReportToWeb(webReport);
                        }
                    } catch (...) {}
                }
            } else { clearCurveChart(); resetFusion(); }
        }
    };

    if (res.isValid && m_isPlatePresent && m_emptyFrameCount > 0) { finishCurrentPlate(); m_emptyFrameCount = 0; }

    if (res.isValid) {
        s_leftEdges.append(res.renderLeftX); s_rightEdges.append(res.renderRightX);
        if (!m_isPlatePresent) {
            clearCurveChart(); resetFusion();
            m_isPlatePresent = true; m_sumWidth = 0.0; m_validFrameCount = 0; m_totalRows = 0;
            m_maxWidth = 0.0; m_minWidth = 99999.0;
            m_globalContourRows.clear(); m_globalContourColsL.clear(); m_globalContourColsR.clear(); m_globalPhysicalWidths.clear();
            s_leftEdges.clear(); s_leftEdges.append(res.renderLeftX); s_rightEdges.clear(); s_rightEdges.append(res.renderRightX);

            s_preBufferRows = 0;
            for (const auto& cachedRes : m_preBufferList) {
                int pRows = cachedRes.contourRows.size();
                if (pRows > 0 && cachedRes.dispImage.IsInitialized()) {
                    HalconCpp::HTuple w, h; HalconCpp::GetImageSize(cachedRes.dispImage, &w, &h);
                    s_preBufferRows += h[0].I(); addFrameToFusion(cachedRes.dispImage);
                    int halfLen = (pRows - 1) / 2;
                    for(int j = 0; j < halfLen; ++j) {
                        m_globalContourRows.append(cachedRes.contourRows[j] + m_totalRows);
                        m_globalContourColsL.append(cachedRes.contourColsLeft[j]);
                        m_globalContourColsR.append(cachedRes.contourColsLeft[pRows - 2 - j]);
                    }
                    for(int j = 0; j < cachedRes.rowWidths.size(); ++j) m_globalPhysicalWidths.append(cachedRes.rowWidths[j]);
                    m_totalRows += h[0].I(); m_validFrameCount++; m_sumWidth += cachedRes.widthValue;
                }
            }
            m_preBufferList.clear(); m_currentGlobalY = m_totalRows;
        }

        m_sumWidth += res.widthValue; m_validFrameCount++;
        if (res.widthValue > m_maxWidth) m_maxWidth = res.widthValue;
        if (res.widthValue < m_minWidth) m_minWidth = res.widthValue;

        int pRows = res.contourRows.size();
        if (pRows > 0) {
            int halfLen = (pRows - 1) / 2;
            for(int i = 0; i < halfLen; ++i) {
                m_globalContourRows.append(res.contourRows[i] + m_currentGlobalY);
                m_globalContourColsL.append(res.contourColsLeft[i]);
                m_globalContourColsR.append(res.contourColsLeft[pRows - 2 - i]);
            }
        }
        for(int i = 0; i < res.rowWidths.size(); ++i) m_globalPhysicalWidths.append(res.rowWidths[i]);

        if (res.dispImage.IsInitialized()) {
            HalconCpp::HTuple w, h; HalconCpp::GetImageSize(res.dispImage, &w, &h);
            m_totalRows += h[0].I(); m_currentGlobalY += h[0].I(); addFrameToFusion(res.dispImage);
        }
        addCurvePoints(QVector<double>() << res.widthValue);

        // =================================================================
        // 🌟【高级接口映射】：改由间接调取弹窗方法把采集点云推进自标定系统
        // =================================================================
        if (m_pCalibDlg && m_pCalibDlg->isVisible() && m_pCalibDlg->isActive() && !m_pCalibDlg->isPaused()) {
            m_pCalibDlg->feedFrame(res);
        }
        m_emptyFrameCount = 0;
    } else {
        if (m_isPlatePresent) {
            m_emptyFrameCount++;
            if (m_emptyFrameCount <= EMPTY_FRAME_LIMIT) {
                int pRows = res.contourRows.size();
                if (pRows > 0) {
                    int halfLen = (pRows - 1) / 2;
                    for(int i = 0; i < halfLen; ++i) {
                        m_globalContourRows.append(res.contourRows[i] + m_currentGlobalY);
                        m_globalContourColsL.append(res.contourColsLeft[i]);
                        m_globalContourColsR.append(res.contourColsLeft[pRows - 2 - i]);
                    }
                    for(int i = 0; i < res.rowWidths.size(); ++i) m_globalPhysicalWidths.append(res.rowWidths[i]);
                    if (res.dispImage.IsInitialized()) {
                        HalconCpp::HTuple w, h; HalconCpp::GetImageSize(res.dispImage, &w, &h);
                        m_totalRows += h[0].I(); m_currentGlobalY += h[0].I();
                    }
                } else {
                    if (res.dispImage.IsInitialized()) {
                        HalconCpp::HTuple w, h; HalconCpp::GetImageSize(res.dispImage, &w, &h);
                        m_currentGlobalY += h[0].I();
                    }
                }
                if (res.dispImage.IsInitialized()) addFrameToFusion(res.dispImage);

                // 过渡尾部尖尖点云的间接标定分流推进
                if (m_pCalibDlg && m_pCalibDlg->isVisible() && m_pCalibDlg->isActive() && !m_pCalibDlg->isPaused()) {
                    m_pCalibDlg->feedFrame(res);
                }
            }
            if (m_emptyFrameCount >= EMPTY_FRAME_LIMIT) finishCurrentPlate();
        } else {
            m_preBufferList.append(res);
            while (m_preBufferList.size() > 3) m_preBufferList.removeFirst();
        }
    }

    if (res.dispImage.IsInitialized()) {
        try {
            HalconCpp::HTuple currentWin = this->winHandle_cam1_pro;
            HalconCpp::HTuple imgW, imgH; HalconCpp::GetImageSize(res.dispImage, &imgW, &imgH);
            HalconCpp::SetPart(currentWin, 0, 0, imgH - 1, imgW - 1); HalconCpp::DispObj(res.dispImage, currentWin);
            if (res.isValid && res.contourRows.size() > 0) {
                HalconCpp::HTuple hRows, hCols;
                for (int i = 0; i < res.contourRows.size(); ++i) { hRows.Append(res.contourRows[i]); hCols.Append(res.contourColsLeft[i]); }
                HalconCpp::HObject closedPolygonXld; HalconCpp::GenContourPolygonXld(&closedPolygonXld, hRows, hCols);
                HalconCpp::SetLineWidth(currentWin, 4); HalconCpp::SetColor(currentWin, "red"); HalconCpp::DispObj(closedPolygonXld, currentWin);
            }
        } catch (...) {}
    }
}

// 🌟 新增：点击主窗体新摆放的唤醒按钮，弹出可自由拖动的标定小页面
void frmView1::on_btn_openCalib_clicked() {
    if (!m_pCalibDlg) {
        m_pCalibDlg = new DlgCalibration(this);
    }
    m_pCalibDlg->show();
    m_pCalibDlg->raise();
    m_pCalibDlg->activateWindow();
}

void frmView1::initHistoryUI() {
    m_tableModelHistory = new QStandardItemModel(this);
    QStringList headers = {"板号", "长度(mm)", "厚度(mm)", "设定宽度(mm)", "平均宽度(mm)", "最大宽度(mm)", "最小宽度(mm)"};
    m_tableModelHistory->setHorizontalHeaderLabels(headers);
    ui->tableView_history->setModel(m_tableModelHistory); ui->tableView_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_history->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFont headerFont = ui->tableView_history->horizontalHeader()->font(); headerFont.setPointSize(14); headerFont.setBold(true);
    ui->tableView_history->horizontalHeader()->setFont(headerFont); ui->tableView_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QFont tableFont = ui->tableView_history->font(); tableFont.setPointSize(16); ui->tableView_history->setFont(tableFont);
    QFont verticalFont = ui->tableView_history->verticalHeader()->font(); verticalFont.setPointSize(14); ui->tableView_history->verticalHeader()->setFont(verticalFont);
}

void frmView1::addHistoryRecord(const QString &plateID, double length, double thickness, double targetW, double avgW, double maxW, double minW) {
    PlateRecord newRecord{plateID, length, thickness, targetW, avgW, maxW, minW};
    m_historyList.prepend(newRecord);
    while (m_historyList.size() > 5) { m_historyList.removeLast(); }
    DatabaseManager::instance().saveRecordToDb(newRecord);
    updateHistoryTable();
}

void frmView1::saveRecordToDb(const PlateRecord &record) {
    if (!QSqlDatabase::database().isOpen()) return;
    QSqlQuery query;
    query.prepare("INSERT INTO PlateRecord (plateID, length, thickness, targetWidth, avgWidth, maxWidth, minWidth) VALUES (:plateID, :length, :thickness, :targetWidth, :avgWidth, :maxWidth, :minWidth)");
    query.bindValue(":plateID", record.plateID); query.bindValue(":length", record.length); query.bindValue(":thickness", record.thickness);
    query.bindValue(":targetWidth", record.targetWidth); query.bindValue(":avgWidth", record.avgWidth); query.bindValue(":maxWidth", record.maxWidth); query.bindValue(":minValue", record.minWidth);
    if (!query.exec()) { qDebug() << "写入历史记录失败:" << query.lastError().text(); }
}

void frmView1::updateHistoryTable() {
    m_tableModelHistory->setRowCount(0);
    for (int i = 0; i < m_historyList.size(); ++i) {
        const PlateRecord &rec = m_historyList.at(i); QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(rec.plateID) << new QStandardItem(QString::number(rec.length, 'f', 1))
                << new QStandardItem(QString::number(rec.thickness, 'f', 2)) << new QStandardItem(QString::number(rec.targetWidth, 'f', 1))
                << new QStandardItem(QString::number(rec.avgWidth, 'f', 2)) << new QStandardItem(QString::number(rec.maxWidth, 'f', 2))
                << new QStandardItem(QString::number(rec.minWidth, 'f', 2));
        for (auto item: rowItems) item->setTextAlignment(Qt::AlignCenter);
        m_tableModelHistory->appendRow(rowItems);
    }
}