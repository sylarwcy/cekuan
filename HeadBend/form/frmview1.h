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

namespace Ui {
    class frmView1;
}

struct PlateRecord {
    QString plateID{"001"};
    double length{0};
    double thickness{0};
    double targetWidth{0};
    double avgWidth{0};
    double maxWidth{0};
    double minWidth{0};
};

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

public:
    void initForm();

private:
    void initDatabase();
    void saveRecordToDb(const PlateRecord& record);
    void loadHistoryFromDb();   // 从数据库加载最近5条记录

    void adjustFontSize(QLineEdit* lineEdit, int h, int fontSize);

    // --- 曲线图相关缓存数据 ---
    QVector<double> m_vecFrameIndex;
    QVector<double> m_vecWidthValue;
    int m_currentRowCount;

    // --- 纯视觉触发状态机变量 ---
    bool m_isPlatePresent;
    int m_emptyFrameCount;
    const int EMPTY_FRAME_LIMIT = 1;

    QList<HalconCpp::HObject> m_preBufferList;
    const int HEAD_FRAME_COUNT = 3;

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

private:
    void updatePostProcessCurve(int multiplier);

    double m_sumWidth = 0.0;
    int m_validFrameCount = 0;
    int m_totalRows = 0;
    double m_maxWidth = 0.0;
    double m_minWidth = 99999.0;

    QVector<double> m_globalContourRows;
    QVector<double> m_globalContourColsL;
    QVector<double> m_globalContourColsR;

    // 物理宽度缓存
    QVector<double> m_globalPhysicalWidths;

    int m_currentGlobalY{0};

    // 缓存绘制图表用的实时值与纠偏值
    QVector<double> m_realtimeWidths;
    QVector<double> m_correctedGlobalWidths;
};

#endif // FRMVIEW1_H