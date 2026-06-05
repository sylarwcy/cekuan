//
// Created by sylar on 26-6-5.
//

#include "dlgcalibration.h"
#include "ui_dlgcalibration.h"

DlgCalibration::DlgCalibration(QWidget *parent) : QDialog(parent), ui(new Ui::DlgCalibration) {
    ui->setupUi(this);

    // 🌟 核心高阶配置：让弹窗变为独立的可自由拖动、带有关闭按钮的标准独立小页面
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("标准样坯在线自标定管理系统");

    ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);
}

DlgCalibration::~DlgCalibration() {
    delete ui;
}

void DlgCalibration::feedFrame(const WidthResult& res) {
    if (m_isInCalibrationMode && !m_isCalibPaused) {
        m_calibManager.feedCalibrationFrame(res);
    }
}

void DlgCalibration::finishPass() {
    double trueW = ui->lineEdit_calibWidth->text().toDouble();
    if(trueW <= 0) trueW = 1500.0;

    bool success = m_calibManager.finishCurrentPass(trueW, 0.09473);
    if (success) {
        ui->lbl_calibStatus->setText(QString("已录入第 %1 趟有效正向数据 (当前总点数: %2)")
                                     .arg(m_calibManager.getPassesCount())
                                     .arg(m_calibManager.getGlobalPointsCount()));
        ui->btn_executeCalib->setEnabled(true);
        ui->btn_removeLastPass->setEnabled(true);
    }
}

void DlgCalibration::on_btn_startCalib_clicked() {
    m_isInCalibrationMode = true;
    m_isCalibPaused = false;
    m_calibManager.resetGlobalCalibration();

    ui->btn_startCalib->setEnabled(false);
    ui->btn_pauseCalib->setEnabled(true);
    ui->btn_pauseCalib->setText("暂停");
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);

    ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;");
    ui->lbl_calibStatus->setText("自标定激活：请让样坯向前通过相机（推荐左中右走3遍）...");
}

void DlgCalibration::on_btn_pauseCalib_clicked() {
    if (!m_isInCalibrationMode) return;
    m_isCalibPaused = !m_isCalibPaused;
    if (m_isCalibPaused) {
        ui->btn_pauseCalib->setText("恢复");
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: gray;");
        ui->lbl_calibStatus->setText("已挂起接收！请让样坯反向退回起点，退出后点击恢复。");
    } else {
        ui->btn_pauseCalib->setText("暂停");
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;");
        ui->lbl_calibStatus->setText(QString("已恢复接收！当前就绪。已累计录入 %1 趟数据。").arg(m_calibManager.getPassesCount()));
    }
}

void DlgCalibration::on_btn_removeLastPass_clicked() {
    bool ok = m_calibManager.removeLastPass();
    if (ok) {
        int currentPasses = m_calibManager.getPassesCount();
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ff5500;");
        ui->lbl_calibStatus->setText(QString("已成功撤销最后一趟异常数据！当前池内剩余有效趟数: %1").arg(currentPasses));
        if (currentPasses == 0) {
            ui->btn_executeCalib->setEnabled(false);
            ui->btn_removeLastPass->setEnabled(false);
        }
    }
}

void DlgCalibration::on_btn_executeCalib_clicked() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString mPath = appDir + "/master_dict.hdict";
    QString sPath = appDir + "/slave_dict.hdict";

    bool success = m_calibManager.finalizeGlobalCalibration(0.09473, mPath, sPath);
    if (success) {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: green;");
        ui->lbl_calibStatus->setText(QString("🎉 自标定大获成功！融合 %1 趟点云，参数已重写。").arg(m_calibManager.getPassesCount()));
    } else {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: red;");
        ui->lbl_calibStatus->setText("标定解算失败，请检查文件权限。");
    }
    m_isInCalibrationMode = false;
    ui->btn_startCalib->setEnabled(true);
    ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);
}

void DlgCalibration::on_btn_cancelCalib_clicked() {
    m_isInCalibrationMode = false;
    m_isCalibPaused = false;
    m_calibManager.resetGlobalCalibration();
    this->close(); // 关闭当前可拖动小页面
}