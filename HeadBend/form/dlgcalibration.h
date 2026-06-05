//
// Created by sylar on 26-6-5.
//
#ifndef DLGCALIBRATION_H
#define DLGCALIBRATION_H

#include <QDialog>
#include <QCoreApplication>
#include <QVector>
#include <QString>
#include "workStationDataStructure.h"
#include "PlateCalibrationManager.h"

namespace Ui {
    class DlgCalibration;
}

class DlgCalibration : public QDialog {
    Q_OBJECT
public:
    explicit DlgCalibration(QWidget *parent = nullptr);
    ~DlgCalibration();

    // 暴露给主窗体的状态查询接口
    bool isActive() const { return m_isInCalibrationMode; }
    bool isPaused() const { return m_isCalibPaused; }

    // 供主窗体在高频收到相机行数据时，异步喂入点云
    void feedFrame(const WidthResult& res);

    // 供主窗体在一趟车尾完全滑出时，触发单趟偏航计算
    void finishPass();

private slots:
    void on_btn_startCalib_clicked();
    void on_btn_pauseCalib_clicked();
    void on_btn_removeLastPass_clicked();
    void on_btn_executeCalib_clicked();
    void on_btn_cancelCalib_clicked();

private:
    Ui::DlgCalibration *ui;
    bool m_isInCalibrationMode{false};
    bool m_isCalibPaused{false};
    PlateCalibrationManager m_calibManager;
};

#endif // DLGCALIBRATION_H