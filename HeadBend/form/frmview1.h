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
    void on_btn_openCalib_clicked(); // 主界面“设定”按钮的响应槽

public:
    void initForm();

private:
    void initDatabase();
    void saveRecordToDb(const PlateRecord& record);
    void loadHistoryFromDb();

    void adjustFontSize(QLineEdit* lineEdit, int h, int fontSize);

    // bool m_isInCalibrationMode{false};
    // int m_calibPassCount{0};
    // bool m_isCalibPaused{false};          // 是否临时暂停接收（用于防反向路过污染）
    // PlateCalibrationManager m_calibManager;
    DlgCalibration* m_pCalibDlg{nullptr};

    // --- 曲线图相关缓存数据 ---
    QVector<double> m_vecFrameIndex;
    QVector<double> m_vecWidthValue;
    int m_currentRowCount;

    // --- 纯视觉触发状态机变量 ---
    bool m_isPlatePresent;
    int m_emptyFrameCount;
    // 将缓冲限制放大到 5 帧，留足空间给长尖细尾进行全量行数累计打捞
    const int EMPTY_FRAME_LIMIT = 5;

    // 彻底丢掉 HEAD_FRAME_COUNT 上限，允许完整容纳整个车头尖尖数据链
    QList<WidthResult> m_preBufferList;

    void initCurveChart();

    // --- 全景拼接相关 ---
    HalconCpp::HObject m_hFusedImage;
    bool m_isFirstFrame;

    void initFusionView();
    void resetFusion();
    void addFrameToFusion(HalconCpp::HObject newFrame);

    // --- 本地历史数据相关 ---
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

    // void on_btn_startCalib_clicked();      // 开启自标定
    // void on_btn_pauseCalib_clicked();      // 暂停/恢复接收开关
    // void on_btn_removeLastPass_clicked();  // 撤销/删除上一趟
    // void on_btn_executeCalib_clicked();    // 计算并落盘
    // void on_btn_cancelCalib_clicked();     // 退出模式

private:
    // 🌟【签名修正】：将原void更正为QVector<double>，与.cpp完全对齐，消除C2556报错
    QVector<double> updatePostProcessCurve(int multiplier);

    double m_sumWidth = 0.0;
    int m_validFrameCount = 0;
    int m_totalRows = 0;
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0;

    QVector<double> m_globalContourRows;
    QVector<double> m_globalContourColsL;
    QVector<double> m_globalContourColsR;

    QVector<double> m_globalPhysicalWidths;

    int m_currentGlobalY{0};

    QVector<double> m_realtimeWidths;
    QVector<double> m_correctedGlobalWidths;

    // 🌟 后台异步服务线程句柄组件
    QThread* m_pMqttThread{nullptr};
    WorkerMQTT* m_pMqttWorker{nullptr};

signals:
    void sig_postPlateReportToWeb(const PlateMqttReportData& data); // 邮寄大数据信号
};

#endif // FRMVIEW1_H