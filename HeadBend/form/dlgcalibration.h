#ifndef DLGCALIBRATION_H
#define DLGCALIBRATION_H

#include <QDialog>
#include <QCoreApplication>
#include <QString>
#include "workStationDataStructure.h"
#include "PlateCalibrationManager.h"

namespace Ui { class DlgCalibration; }

class DlgCalibration : public QDialog {
    Q_OBJECT
public:
    explicit DlgCalibration(QWidget *parent = nullptr);
    ~DlgCalibration();

    bool isActive() const { return m_isInCalibrationMode; }
    bool isPaused() const { return m_isCalibPaused; }
    int getMultiplier() const;
    void finishPass();

    // 🌟 暴露给主控查询，并接收最终战果
    int getCurrentStep() const { return m_currentStep; }
    void onPlateCalibFinished(double trueAvg);

public slots:
    void slot_onBaselineCalibrated(double value);

    signals:
        void sig_multiplierChanged(int value);
    void sig_speedChanged(double speed_m_s);

private slots:
    void on_btn_startCalib_clicked(); void on_btn_pauseCalib_clicked();
    void on_btn_removeLastPass_clicked(); void on_btn_executeCalib_clicked();
    void on_btn_cancelCalib_clicked(); void slot_speedEditingFinished();
    void slot_spinSegmentChanged(int value);

    // 三大标定触发网关
    void on_btn_autoMasterPixel_clicked();
    void on_btn_autoSlavePixel_clicked();
    void on_btn_autoBaseline_clicked();

private:
    Ui::DlgCalibration *ui;
    bool m_isInCalibrationMode{false};
    bool m_isCalibPaused{false};
    PlateCalibrationManager m_calibManager;

    // 标记状态位 (0:正常, 1:主, 2:副, 3:双目重叠)
    int m_currentStep{0};
};
#endif