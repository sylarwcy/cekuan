// ==================== [ WorkerImageProcess.cpp ] ====================
#include "WorkerImageProcess.h"
#include "appconfig.h"
#include "HalconCpp.h"
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>
#include "ini/settings.h"
#include <QCollator>
#include <algorithm>

HalconCpp::HTuple winHandle_pro;
WorkerImageProcess::WorkerImageProcess(QObject *parent) : QObject(parent), m_algo(nullptr) {
    qRegisterMetaType<WidthResult>("WidthResult");
    m_pOfflineTimer = new QTimer(this);
    connect(m_pOfflineTimer, &QTimer::timeout, this, &WorkerImageProcess::onOfflineTimerTimeout);
}

WorkerImageProcess::~WorkerImageProcess() { if (m_algo) delete m_algo; }

void WorkerImageProcess::init(const WorkStation_DATA &paramData, double mmPerPixel) {
    m_mm_per_row = mmPerPixel; m_paramData = paramData;
    if (!m_algo) m_algo = new DualLineScanWidthImgPro();

    QString appDir = QCoreApplication::applicationDirPath();
    IniSettings setting_ini("setting.ini");

    QString strUse1D = setting_ini.getValue("workstation1", "Use1DMeasure");
    bool use1DMode = (!strUse1D.isEmpty() && (strUse1D.toLower() == "false" || strUse1D == "0")) ? false : true;
    m_algo->setEdgeExtractionMode(use1DMode);

    // 🌟 1. 初始化厚度补偿物理参数
    m_thicknessK = setting_ini.getValue("Calibration", "ThicknessK").toDouble();
    m_baseThickness = setting_ini.getValue("Calibration", "BaseThickness").toDouble();
    if (m_baseThickness <= 0) m_baseThickness = 2.0;

    QString strOfflineMode = setting_ini.getValue("workstation1", "OfflineMode");
    m_bIsOfflineMode = (strOfflineMode.toLower() == "true" || strOfflineMode == "1");
    m_offlineDir = setting_ini.getValue("workstation1", "OfflineTestPath");
    m_offlineIntervalMs = setting_ini.getValue("workstation1", "OfflineIntervalMs").toInt();
    if (m_offlineIntervalMs <= 0) m_offlineIntervalMs = 1000;

    if (m_algo->initAlgorithm(appDir + "/Camera_Master_1DLUT.hdict", appDir + "/Camera_Slave_1DLUT.hdict", mmPerPixel)) {
        qInfo() << "[系统通知] 算法大脑启动成功。厚度补偿引擎就绪(K=" << m_thicknessK << ")";
    }
}

// 🌟 厚度热更新通道
void WorkerImageProcess::slot_updateCurrentThickness(double t) { m_currentThickness = t; }
void WorkerImageProcess::slot_updateThicknessK(double k, double baseT) { m_thicknessK = k; m_baseThickness = baseT; }

void WorkerImageProcess::startOfflineTest() {
    if (!m_bIsOfflineMode || m_offlineDir.isEmpty()) return;
    m_offlineIndex = 0; m_offlineFiles.clear(); QDir dir(m_offlineDir);
    QStringList filters; filters << "*.jpg" << "*.bmp" << "*.png";
    m_offlineFiles = dir.entryList(filters, QDir::Files);
    if (m_offlineFiles.isEmpty()) return;

    QCollator collator; collator.setNumericMode(true);
    std::sort(m_offlineFiles.begin(), m_offlineFiles.end(), collator);
    m_pOfflineTimer->start(m_offlineIntervalMs);
}

void WorkerImageProcess::onOfflineTimerTimeout() {
    if (m_offlineIndex >= m_offlineFiles.size()) {
        m_pOfflineTimer->stop(); WidthResult emptyRes; emptyRes.isValid = false;
        if (m_isPlateActive) {
            m_isPlateActive = false;
            if (m_validFrameCount > 0) emit sigPlateFinished(m_sumWidth / m_validFrameCount, m_totalRows * m_mm_per_row, m_maxWidth, m_minWidth);
        }
        emit sigMeasureReady(emptyRes); return;
    }

    HalconCpp::HObject dispImage;
    try { HalconCpp::ReadImage(&dispImage, (m_offlineDir + "/" + m_offlineFiles[m_offlineIndex]).toLocal8Bit().constData()); } catch (...) { m_offlineIndex++; return; }

    if (m_algo) {
        WidthResult res = m_algo->processOfflineDispImage(dispImage, m_offlineIndex);
        res.frameID = m_offlineIndex;

        if (res.isValid) {
            // =========================================================
            // 🌟 核心：极其优雅的厚度线性透视拦截器 (1 - K * Delta_T)
            // =========================================================
            double compFactor = 1.0 - m_thicknessK * (m_currentThickness - m_baseThickness);
            res.widthValue *= compFactor;
            for (int i = 0; i < res.rowWidths.size(); ++i) res.rowWidths[i] *= compFactor;

            if (!m_isPlateActive) { m_isPlateActive = true; m_sumWidth = 0.0; m_validFrameCount = 0; m_totalRows = 0; m_maxWidth = 0.0; m_minWidth = 99999.0; }
            m_sumWidth += res.widthValue; m_validFrameCount++;
            if (res.widthValue > m_maxWidth) m_maxWidth = res.widthValue;
            if (res.widthValue < m_minWidth) m_minWidth = res.widthValue;
            HalconCpp::HTuple w, h; HalconCpp::GetImageSize(dispImage, &w, &h); m_totalRows += h[0].I();
        } else {
            if (m_isPlateActive) { m_isPlateActive = false; if (m_validFrameCount > 0) emit sigPlateFinished(m_sumWidth / m_validFrameCount, m_totalRows * m_mm_per_row, m_maxWidth, m_minWidth); }
        }
        emit sigMeasureReady(res);
    }
    m_offlineIndex++;
}

void WorkerImageProcess::imgProcessMeasure(const DualCameraChunk &chunk) {
    if (m_isProcessing) return; m_isProcessing = true;

    if (m_algo && (chunk.hasLeft || chunk.hasRight)) {
        WidthResult res = m_algo->processFrame(chunk.imgLeft, chunk.imgRight, chunk.frameID);
        res.frameID = chunk.frameID;

        if (res.isValid) {
            // =========================================================
            // 🌟 在线生产时，同样接受实时厚度的透明拦截
            // =========================================================
            double compFactor = 1.0 - m_thicknessK * (m_currentThickness - m_baseThickness);
            res.widthValue *= compFactor;
            for (int i = 0; i < res.rowWidths.size(); ++i) res.rowWidths[i] *= compFactor;

            if (!m_isPlateActive) { m_isPlateActive = true; m_sumWidth = 0.0; m_validFrameCount = 0; m_totalRows = 0; m_maxWidth = 0.0; m_minWidth = 99999.0; m_algo->resetBuffers(); }
            m_sumWidth += res.widthValue; m_validFrameCount++;
            if (res.widthValue > m_maxWidth) m_maxWidth = res.widthValue;
            if (res.widthValue < m_minWidth) m_minWidth = res.widthValue;

            HalconCpp::HTuple w, h; HalconCpp::GetImageSize(chunk.imgLeft, &w, &h); m_totalRows += h[0].I();
            if (res.dispImage.IsInitialized() && !m_pOfflineTimer->isActive()) {
                try {
                    QString dirPath = QCoreApplication::applicationDirPath() + "/DebugImages/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH"); QDir().mkpath(dirPath);
                    HalconCpp::WriteImage(res.dispImage, "jpeg", 0, QString("%1/Frame_Spliced_%2.jpg").arg(dirPath).arg(res.frameID).toLocal8Bit().constData());
                } catch (...) {}
            }
        } else {
            if (m_isPlateActive) { m_isPlateActive = false; if (m_validFrameCount > 0) emit sigPlateFinished(m_sumWidth / m_validFrameCount, m_totalRows * m_mm_per_row, m_maxWidth, m_minWidth); }
        }
        emit sigMeasureReady(res);
    }
    m_isProcessing = false;
}

void WorkerImageProcess::slot_reloadCalibration() {
    QString appDir = QCoreApplication::applicationDirPath();
    if (m_algo) m_algo->initAlgorithm(appDir + "/Camera_Master_1DLUT.hdict", appDir + "/Camera_Slave_1DLUT.hdict", m_mm_per_row);
}
void WorkerImageProcess::slot_setCalibMode(int mode) { if (m_algo) m_algo->setCalibMode(mode); }
void WorkerImageProcess::slot_hotUpdateMasterPixel(double newC) { if (m_algo) { m_algo->updatePixelResolution(true, newC); m_algo->saveCurrentDictsToDisk(); } }
void WorkerImageProcess::slot_hotUpdateSlavePixel(double newC) { if (m_algo) { m_algo->updatePixelResolution(false, newC); m_algo->saveCurrentDictsToDisk(); } }
void WorkerImageProcess::slot_hotUpdateBaseline(double newOffset) {
    if (m_algo) { m_algo->updateBaselineOffset(newOffset); m_algo->saveCurrentDictsToDisk(); emit sig_baselineCalibrated(newOffset); }
}