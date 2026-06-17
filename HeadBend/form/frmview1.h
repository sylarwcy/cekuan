// ==================== [ frmview1.h ] ====================
#ifndef FRMVIEW1_H
#define FRMVIEW1_H

#include <QWidget>
#include <QLineEdit>
#include "qcustomplot.h"
#include "MyApplication.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <databasemanager.h>
#include <PlateCalibrationManager.h>
#include "dlgcalibration.h"

namespace Ui {
    class frmView1;
}

inline QDataStream &operator<<(QDataStream &out, const PlateRecord &record) {
    out << record.plateID << record.length << record.thickness
        << record.targetWidth << record.avgWidth << record.maxWidth << record.minWidth;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, PlateRecord &record) {
    in >> record.plateID >> record.length >> record.thickness
       >> record.targetWidth >> record.avgWidth >> record.maxWidth >> record.minWidth;
    return in;
}

class frmView1 : public QWidget
{
    Q_OBJECT

public:
    explicit frmView1(QWidget *parent = nullptr);
    ~frmView1();

private:
    Ui::frmView1 *ui;

public:
    HTuple winHandle_cam1_ori;
    HTuple winHandle_cam1_pro;
    HTuple winHandle_cam2_ori;
    HTuple winHandle_fusion;
    HTuple winHandle_lunkuo;

    MyCameraGigE *p_camera_front;
    MyCameraGigE *p_camera_back;

    QStandardItemModel *model_from_plc;
    QStandardItemModel *model_to_plc;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    void varInit();
    void logSlot(const QString &message, int level);
    void refreshDataDisp(void);
    void startDispRefresh();
    void onMeasureReady(const WidthResult &res);
    void on_btn_openCalib_clicked();

public:
    void initForm();

private:
    void initDatabase();
    void saveRecordToDb(const PlateRecord& record);
    void loadHistoryFromDb();
    void adjustFontSize(QLineEdit* lineEdit, int h, int fontSize);

    DlgCalibration* m_pCalibDlg{nullptr};

    QVector<double> m_vecFrameIndex;
    QVector<double> m_vecWidthValue;
    int m_currentRowCount;

    bool m_isPlatePresent;
    int m_emptyFrameCount;
    const int EMPTY_FRAME_LIMIT = 5;

    QList<WidthResult> m_preBufferList;

    void initCurveChart();

    HalconCpp::HObject m_hFusedImage;
    bool m_isFirstFrame;

    void initFusionView();
    void resetFusion();
    void addFrameToFusion(HalconCpp::HObject newFrame);

    QList<PlateRecord> m_historyList;
    QStandardItemModel *m_tableModelHistory;

    void initHistoryUI();
    void updateHistoryTable();
    void addHistoryRecord(const QString& plateID, double length, double thickness,
                          double targetW, double avgW, double maxW, double minW);

private slots:
    void onUpdateRawImage(const DualCameraChunk &chunk);
    void addCurvePoints(const QVector<double> &widths);
    void clearCurveChart();
    void onMultiplierChanged(int value);

private:
    QVector<double> updatePostProcessCurve(int multiplier);

    // --- 算法重构抽离的核心私有控制流 ---
    void displayImageProportional(HalconCpp::HObject img, HalconCpp::HTuple winHandle, int winW, int winH, QString bgColor, bool showImg);
    void handleValidFrame(const WidthResult &res);
    void handleInvalidFrame(const WidthResult &res);
    void drawCurrentFrame(const WidthResult &res);
    void calculatePlateStats(double &trueAvg, double &trueMax, double &trueMin, double &cosTheta, double &sinTheta, int currentStep);
    void renderAndSavePlateImages(int globalMinLeft, int globalMaxRight, double trueAvg, double trueMax, double trueMin, double totalLength, int currentStep);
    void finishCurrentPlate();

    double m_sumWidth = 0.0;
    int m_validFrameCount = 0;
    int m_totalRows = 0;
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0;

    QVector<double> m_globalContourRows;
    QVector<double> m_globalContourColsL;
    QVector<double> m_globalContourColsR;
    QVector<double> m_globalPhysicalWidths;

    // --- 取代原版静态变量，彻底解耦清理 ---
    QVector<int> m_leftEdges;
    QVector<int> m_rightEdges;
    int m_preBufferRows{0};

    int m_currentGlobalY{0};

    QVector<double> m_realtimeWidths;
    QVector<double> m_correctedGlobalWidths;

    QThread* m_pMqttThread{nullptr};
    WorkerMQTT* m_pMqttWorker{nullptr};

signals:
    void sig_postPlateReportToWeb(const PlateMqttReportData& data);
};

#endif // FRMVIEW1_H