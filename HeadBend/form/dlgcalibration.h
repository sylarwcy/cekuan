// ==================== [ dlgcalibration.h ] ====================
#ifndef DLGCALIBRATION_H
#define DLGCALIBRATION_H

#include <QDialog>
#include <QCoreApplication>
#include <QString>
#include <QVector>
#include "workStationDataStructure.h"
#include "PlateCalibrationManager.h"

namespace Ui { class DlgCalibration; }

// 🌟 新增：用于存储多点厚度标定历史数据的结构体
struct ThicknessCalibPoint {
    double deltaT; // 相对基准厚度(2mm)的厚度差 (x 轴)
    double ratioY; // 宽度缩放比例因子 (y 轴)
};

class DlgCalibration : public QDialog {
    Q_OBJECT
public:
    explicit DlgCalibration(QWidget *parent = nullptr);
    ~DlgCalibration();

    bool isActive() const { return m_isInCalibrationMode; }
    bool isPaused() const { return m_isCalibPaused; }
    int getMultiplier() const;
    void finishPass();

    int getCurrentStep() const { return m_currentStep; }
    void onPlateCalibFinished(double trueAvg);

public slots:
    void slot_onBaselineCalibrated(double value);

    signals:
        void sig_multiplierChanged(int value);
    void sig_speedChanged(double speed_m_s);
    void sig_exposureTimeChanged(int exposure_us);

private slots:
    void on_btn_startCalib_clicked(); void on_btn_pauseCalib_clicked();
    void on_btn_removeLastPass_clicked(); void on_btn_executeCalib_clicked();
    void on_btn_cancelCalib_clicked(); void slot_speedEditingFinished();
    void slot_spinSegmentChanged(int value);
    void slot_exposureEditingFinished();

    void slot_calibWidthEditingFinished();
    void slot_calibThicknessEditingFinished();

    void on_btn_autoMasterPixel_clicked();
    void on_btn_autoSlavePixel_clicked();
    void on_btn_autoBaseline_clicked();
    void on_btn_autoThicknessComp_clicked();

protected:
    void showEvent(QShowEvent *event) override;
private:
    Ui::DlgCalibration *ui;
    bool m_isInCalibrationMode{false};
    bool m_isCalibPaused{false};
    PlateCalibrationManager m_calibManager;

    int m_currentStep{0};

    // 🌟 新增：厚度标定多点拟合池
    QVector<ThicknessCalibPoint> m_thicknessPoints;
};
#endif