#include "dlgcalibration.h"
#include "MyApplication.h"
#include "ui_dlgcalibration.h"
#include <QMessageBox>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>

DlgCalibration::DlgCalibration(QWidget *parent) : QDialog(parent), ui(new Ui::DlgCalibration) {
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("标准样坯在线自标定与空间重叠区域对齐中心");

    this->resize(720, 490);
    this->setMinimumSize(720, 490);
    this->setMaximumSize(720, 490);

    ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);

    connect(ui->spinSegment, QOverload<int>::of(&QSpinBox::valueChanged), this, &DlgCalibration::slot_spinSegmentChanged);
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &DlgCalibration::slot_speedEditingFinished);
}

DlgCalibration::~DlgCalibration() {
    delete ui;
}

int DlgCalibration::getMultiplier() const {
    return ui->spinSegment->value();
}

void DlgCalibration::slot_spinSegmentChanged(int value) {
    emit sig_multiplierChanged(value);
}

void DlgCalibration::slot_speedEditingFinished() {
    double speed_mm_s = ui->lineEdit->text().toDouble();
    if (speed_mm_s <= 0.0) return;
    double speed_m_s = speed_mm_s / 1000.0;
    emit sig_speedChanged(speed_m_s);
    ui->lbl_calibStatus->setText(QString("辊道速度已更新为: %1 mm/s，正在自适应微调相机物理行频...").arg(speed_mm_s));
}

// 🌟 指挥指令下发
void DlgCalibration::on_btn_autoMasterPixel_clicked() {
    m_currentStep = 1;
    MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerImageProcess) {
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 1));
        ui->lbl_calibStatus->setText("步骤1激态：底层管道已被强制替换为主相机，请走板...");
    }
}

void DlgCalibration::on_btn_autoSlavePixel_clicked() {
    m_currentStep = 2;
    MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerImageProcess) {
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 2));
        ui->lbl_calibStatus->setText("步骤2激态：底层管道已被强制替换为副相机，请走板...");
    }
}

void DlgCalibration::on_btn_autoBaseline_clicked() {
    m_currentStep = 3;
    MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerImageProcess) {
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 3));
        ui->lbl_calibStatus->setText("步骤3激态：等待样坯在双目交界处通过以解算轴距...");
    }
}

// 🌟 核心回调：接收从金牌生产线层层大网过滤后反馈回来的“绝对无损大身真理均值”
void DlgCalibration::onPlateCalibFinished(double trueAvg) {
    double realW = ui->lineEdit_calibWidth->text().toDouble();
    MyApplication *pApp = (MyApplication *) qApp;

    if (m_currentStep == 1) {
        // trueAvg 此时实际上是主相机的“像素跨度均值”
        double newCm = realW / trueAvg;
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateMasterPixel", Qt::QueuedConnection, Q_ARG(double, newCm));
        ui->lbl_masterPixelRes->setText(QString("%1 mm/px").arg(QString::number(newCm, 'f', 6)));
        QMessageBox::information(this, "成功", "主相机当量标定成功并落盘！");
    }
    else if (m_currentStep == 2) {
        // trueAvg 此时实际上是副相机的“像素跨度均值”
        double newCs = realW / trueAvg;
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateSlavePixel", Qt::QueuedConnection, Q_ARG(double, newCs));
        ui->lbl_slavePixelRes->setText(QString("%1 mm/px").arg(QString::number(newCs, 'f', 6)));
        QMessageBox::information(this, "成功", "副相机当量标定成功并落盘！");
    }
    else if (m_currentStep == 3) {
        // trueAvg 此时直接就是双目物理轴距偏移量的精细均值！
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateBaseline", Qt::QueuedConnection, Q_ARG(double, trueAvg));
    }

    // 收尾工作：解除欺骗，恢复生产状态机
    m_currentStep = 0;
    QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 0));
    ui->lbl_calibStatus->setText("当前状态：正常测量生产模式");
}

void DlgCalibration::slot_onBaselineCalibrated(double value) {
    ui->lbl_baselineRes->setText(QString("双目重叠轴距: %1 mm").arg(QString::number(value, 'f', 2)));
    QMessageBox::information(this, "大功告成", QString("🎉 双目轴距 [%1 mm] 对齐成功已落盘！").arg(QString::number(value, 'f', 2)));
}
// ... 下方的 finishPass、按钮响应等完全不变 ...

void DlgCalibration::finishPass() {
    double trueW = ui->lineEdit_calibWidth->text().toDouble();
    if(trueW <= 0) trueW = 1500.0;
    MyApplication *pApp = (MyApplication *) qApp;
    double mmPerPixel = 0.09473;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) {
        mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    }
    bool success = m_calibManager.finishCurrentPass(trueW, mmPerPixel);
    if (success) {
        ui->lbl_calibStatus->setText(QString("已录入第 %1 趟有效正向数据 (当前总点数: %2)").arg(m_calibManager.getPassesCount()).arg(m_calibManager.getGlobalPointsCount()));
        ui->btn_executeCalib->setEnabled(true); ui->btn_removeLastPass->setEnabled(true);
    }
}

void DlgCalibration::on_btn_startCalib_clicked() {
    m_isInCalibrationMode = true; m_isCalibPaused = false; m_calibManager.resetGlobalCalibration();
    ui->btn_startCalib->setEnabled(false); ui->btn_pauseCalib->setEnabled(true); ui->btn_pauseCalib->setText("暂停");
    ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false);
    ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;");
    ui->lbl_calibStatus->setText("自标定激活：请让样坯向前通过相机（推荐左中右走3慢）...");
}

void DlgCalibration::on_btn_pauseCalib_clicked() {
    if (!m_isInCalibrationMode) return;
    m_isCalibPaused = !m_isCalibPaused;
    if (m_isCalibPaused) {
        ui->btn_pauseCalib->setText("恢复"); ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: gray;");
        ui->lbl_calibStatus->setText("已挂起接收！请让样坯反向退回起点，退出后点击恢复。");
    } else {
        ui->btn_pauseCalib->setText("暂停"); ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;");
        ui->lbl_calibStatus->setText(QString("已恢复接收！当前就绪。已累计录入 %1 趟数据。").arg(m_calibManager.getPassesCount()));
    }
}

void DlgCalibration::on_btn_removeLastPass_clicked() {
    bool ok = m_calibManager.removeLastPass();
    if (ok) {
        int currentPasses = m_calibManager.getPassesCount();
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ff5500;");
        ui->lbl_calibStatus->setText(QString("已成功撤销最后一趟异常数据！当前池内剩余有效趟数: %1").arg(currentPasses));
        if (currentPasses == 0) { ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false); }
    }
}

void DlgCalibration::on_btn_executeCalib_clicked() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString mPath = appDir + "/Camera_Master_1DLUT.hdict";
    QString sPath = appDir + "/Camera_Slave_1DLUT.hdict";
    MyApplication *pApp = (MyApplication *) qApp;
    double mmPerPixel = 0.09473;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) {
        mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    }
    bool success = m_calibManager.finalizeGlobalCalibration(mmPerPixel, mPath, sPath);
    if (success) {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: green;");
        ui->lbl_calibStatus->setText(QString("🎉 自标定大获成功！融合 %1 趟点云，参数已重写。").arg(m_calibManager.getPassesCount()));
        if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerImageProcess) {
            QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_reloadCalibration", Qt::QueuedConnection);
        }
    } else {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: red;"); ui->lbl_calibStatus->setText("标定解算失败，请检查文件权限。");
    }
    m_isInCalibrationMode = false; ui->btn_startCalib->setEnabled(true); ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false);
}

void DlgCalibration::on_btn_cancelCalib_clicked() {
    m_isInCalibrationMode = false; m_isCalibPaused = false; m_calibManager.resetGlobalCalibration(); this->close();
}