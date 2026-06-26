// ==================== [ dlgcalibration.cpp ] ====================
#include "dlgcalibration.h"
#include "MyApplication.h"
#include "ui_dlgcalibration.h"
#include <QMessageBox>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QSettings>

DlgCalibration::DlgCalibration(QWidget *parent) : QDialog(parent), ui(new Ui::DlgCalibration) {
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("标准样坯在线自标定与空间重叠区域对齐中心");
    this->resize(850, 540); this->setMinimumSize(850, 540); this->setMaximumSize(850, 540);

    QList<QPushButton*> allButtons = this->findChildren<QPushButton*>();
    for (QPushButton* btn : allButtons) { btn->setAutoDefault(false); btn->setDefault(false); }

    ui->btn_pauseCalib->setEnabled(false); ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false);

    connect(ui->spinSegment, QOverload<int>::of(&QSpinBox::valueChanged), this, &DlgCalibration::slot_spinSegmentChanged);
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &DlgCalibration::slot_speedEditingFinished);
    connect(ui->lineEdit_exposure, &QLineEdit::editingFinished, this, &DlgCalibration::slot_exposureEditingFinished);
    connect(ui->lineEdit_calibWidth, &QLineEdit::editingFinished, this, &DlgCalibration::slot_calibWidthEditingFinished);
    connect(ui->lineEdit_calibThickness, &QLineEdit::editingFinished, this, &DlgCalibration::slot_calibThicknessEditingFinished);

    QSettings settings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat);
    ui->lineEdit_exposure->setText(QString::number(settings.value("CameraFront/front_expTime", 5000).toInt()));
    ui->lineEdit_calibWidth->setText(QString::number(settings.value("Calibration/StandardWidth", 149.5).toDouble(), 'f', 1));
    ui->lineEdit_calibThickness->setText(QString::number(settings.value("Calibration/StandardThickness", 2.0).toDouble(), 'f', 1));
    ui->spinSegment->setValue(settings.value("Calibration/CurveMultiplier", 3).toInt());

    double savedK = settings.value("Calibration/ThicknessK", 0.0).toDouble();
    if (savedK > 0) ui->lbl_thicknessK->setText(QString("当前系数 K：%1").arg(QString::number(savedK, 'f', 7)));

    // 每次打开弹窗，清空历史标定池，准备迎接新一轮的多点拟合
    m_thicknessPoints.clear();
}

DlgCalibration::~DlgCalibration() { delete ui; }

int DlgCalibration::getMultiplier() const { return ui->spinSegment->value(); }

void DlgCalibration::slot_spinSegmentChanged(int value) {
    emit sig_multiplierChanged(value);
    QSettings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat).setValue("Calibration/CurveMultiplier", value);
    ui->lbl_calibStatus->setText(QString("曲线分段倍率已更新为: %1 倍").arg(value));
}

void DlgCalibration::slot_calibWidthEditingFinished() {
    double width = ui->lineEdit_calibWidth->text().toDouble();
    if (width <= 0) return;
    QSettings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat).setValue("Calibration/StandardWidth", width);
    ui->lbl_calibStatus->setText(QString("标准样坯物理宽度已设定为: %1 mm").arg(width));
}

void DlgCalibration::slot_calibThicknessEditingFinished() {
    double thickness = ui->lineEdit_calibThickness->text().toDouble();
    QSettings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat).setValue("Calibration/StandardThickness", thickness);
    ui->lbl_calibStatus->setText(QString("标准样坯/生产基准厚度已设定为: %1 mm").arg(thickness));
}

void DlgCalibration::slot_speedEditingFinished() {
    double speed_mm_s = ui->lineEdit->text().toDouble();
    if (speed_mm_s <= 0.0) return;
    emit sig_speedChanged(speed_mm_s / 1000.0);
    ui->lbl_calibStatus->setText(QString("辊道速度已更新为: %1 mm/s").arg(speed_mm_s));
}

void DlgCalibration::slot_exposureEditingFinished() {
    int exp_us = ui->lineEdit_exposure->text().toInt();
    if (exp_us < 10) return;
    QSettings settings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat);
    settings.setValue("CameraFront/front_expTime", exp_us); settings.setValue("CameraBack/back_expTime", exp_us);
    emit sig_exposureTimeChanged(exp_us);
    ui->lbl_calibStatus->setText(QString("曝光时间已下发: %1 us，行频已锁定在95%极速").arg(exp_us));
}

void DlgCalibration::on_btn_autoMasterPixel_clicked() {
    m_currentStep = 1; MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty()) QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 1));
    ui->lbl_calibStatus->setText("步骤1激态：底层管道已被强制替换为主相机，请走板...");
}

void DlgCalibration::on_btn_autoSlavePixel_clicked() {
    m_currentStep = 2; MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty()) QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 2));
    ui->lbl_calibStatus->setText("步骤2激态：底层管道已被强制替换为副相机，请走板...");
}

void DlgCalibration::on_btn_autoBaseline_clicked() {
    m_currentStep = 3; MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty()) QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 3));
    ui->lbl_calibStatus->setText("步骤3激态：等待样坯在双目交界处通过以解算轴距...");
}

void DlgCalibration::on_btn_autoThicknessComp_clicked() {
    m_currentStep = 4; MyApplication *pApp = (MyApplication *) qApp;
    if (pApp && !pApp->m_workstationList.isEmpty()) QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 0));
    ui->lbl_calibStatus->setText("步骤4激态：请放入非2mm的厚钢板进行扫描以反推K系数...");
}

void DlgCalibration::onPlateCalibFinished(double trueAvg) {
    double realW = ui->lineEdit_calibWidth->text().toDouble();
    double realT = ui->lineEdit_calibThickness->text().toDouble();
    MyApplication *pApp = (MyApplication *) qApp;

    if (m_currentStep == 1) {
        double newCm = realW / trueAvg;
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateMasterPixel", Qt::QueuedConnection, Q_ARG(double, newCm));
        ui->lbl_masterPixelRes->setText(QString("%1 mm/px").arg(QString::number(newCm, 'f', 6)));
    }
    else if (m_currentStep == 2) {
        double newCs = realW / trueAvg;
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateSlavePixel", Qt::QueuedConnection, Q_ARG(double, newCs));
        ui->lbl_slavePixelRes->setText(QString("%1 mm/px").arg(QString::number(newCs, 'f', 6)));
    }
    else if (m_currentStep == 3) {
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_hotUpdateBaseline", Qt::QueuedConnection, Q_ARG(double, trueAvg));
    }
    else if (m_currentStep == 4) {
        // ==========================================================
        // 🌟 核心：多点最小二乘法 (Least Squares Method) 线性拟合
        // 公式：W_true = W_meas * (1 - K * delta_T)  =>  1 - W_true/W_meas = K * delta_T
        // 设 y = 1 - W_true/W_meas, x = delta_T
        // 最小二乘解为：K = Sum(x * y) / Sum(x * x)
        // ==========================================================
        double baseT = 2.0;
        if (std::abs(realT - baseT) < 5.0) {
            QMessageBox::warning(this, "参数错误", "用来反推系数的钢板，厚度必须与基准(2mm)有明显差异(建议>10mm)！");
            m_currentStep = 0; return;
        }

        // 提取本次数据的 (x, y)
        double currentX = realT - baseT;
        double currentY = 1.0 - (realW / trueAvg);

        // 压入历史拟合池
        m_thicknessPoints.append({currentX, currentY});

        // 遍历池中所有钢板数据，进行最小二乘拟合
        double sum_xy = 0.0;
        double sum_xx = 0.0;
        for (int i = 0; i < m_thicknessPoints.size(); ++i) {
            sum_xy += m_thicknessPoints[i].deltaT * m_thicknessPoints[i].ratioY;
            sum_xx += m_thicknessPoints[i].deltaT * m_thicknessPoints[i].deltaT;
        }

        double K = sum_xy / sum_xx; // 求出全局最优解 K

        QSettings settings(QCoreApplication::applicationDirPath() + "/setting.ini", QSettings::IniFormat);
        settings.setValue("Calibration/ThicknessK", K);
        settings.setValue("Calibration/BaseThickness", baseT);

        // 热更新给底层
        QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_updateThicknessK", Qt::QueuedConnection, Q_ARG(double, K), Q_ARG(double, baseT));

        ui->lbl_thicknessK->setText(QString("当前系数 K：%1").arg(QString::number(K, 'f', 7)));

        QMessageBox::information(this, "大功告成", QString("🎉 厚度标定成功！\n\n当前已融合 [%1] 块不同厚度钢板的数据。\n测得的最后一块均宽：%2 mm\n最后一块真实宽度：%3 mm\n\n系统拟合的最优物理比例 K = %4\n\n*提示：您可以继续修改厚度并放入新钢板点击[步骤4]以进一步提升精度。关闭此窗口将清空当前拟合池。")
                                                .arg(m_thicknessPoints.size()).arg(trueAvg).arg(realW).arg(K));
    }

    m_currentStep = 0;
    QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_setCalibMode", Qt::QueuedConnection, Q_ARG(int, 0));
    ui->lbl_calibStatus->setText("当前状态：正常测量生产模式");
}

void DlgCalibration::slot_onBaselineCalibrated(double value) {
    ui->lbl_baselineRes->setText(QString("双目重叠轴距: %1 mm").arg(QString::number(value, 'f', 2)));
    QMessageBox::information(this, "大功告成", QString("🎉 双目轴距 [%1 mm] 对齐成功已落盘！").arg(QString::number(value, 'f', 2)));
}

void DlgCalibration::finishPass() {
    double trueW = ui->lineEdit_calibWidth->text().toDouble();
    if(trueW <= 0) trueW = 1500.0; MyApplication *pApp = (MyApplication *) qApp; double mmPerPixel = 0.09473;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    if (m_calibManager.finishCurrentPass(trueW, mmPerPixel)) {
        ui->lbl_calibStatus->setText(QString("已录入第 %1 趟有效正向数据 (当前总点数: %2)").arg(m_calibManager.getPassesCount()).arg(m_calibManager.getGlobalPointsCount()));
        ui->btn_executeCalib->setEnabled(true); ui->btn_removeLastPass->setEnabled(true);
    }
}

void DlgCalibration::on_btn_startCalib_clicked() {
    m_isInCalibrationMode = true; m_isCalibPaused = false; m_calibManager.resetGlobalCalibration();
    ui->btn_startCalib->setEnabled(false); ui->btn_pauseCalib->setEnabled(true); ui->btn_pauseCalib->setText("暂停");
    ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false);
    ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;"); ui->lbl_calibStatus->setText("自标定激活：请让样坯向前通过相机（推荐左中右走3慢）...");
}

void DlgCalibration::on_btn_pauseCalib_clicked() {
    if (!m_isInCalibrationMode) return;
    m_isCalibPaused = !m_isCalibPaused;
    if (m_isCalibPaused) {
        ui->btn_pauseCalib->setText("恢复"); ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: gray;"); ui->lbl_calibStatus->setText("已挂起接收！请让样坯反向退回起点，退出后点击恢复。");
    } else {
        ui->btn_pauseCalib->setText("暂停"); ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ffaa00;"); ui->lbl_calibStatus->setText(QString("已恢复接收！当前就绪。已累计录入 %1 趟数据。").arg(m_calibManager.getPassesCount()));
    }
}

void DlgCalibration::on_btn_removeLastPass_clicked() {
    if (m_calibManager.removeLastPass()) {
        int currentPasses = m_calibManager.getPassesCount();
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #ff5500;"); ui->lbl_calibStatus->setText(QString("已成功撤销最后一趟异常数据！当前池内剩余有效趟数: %1").arg(currentPasses));
        if (currentPasses == 0) { ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false); }
    }
}

void DlgCalibration::on_btn_executeCalib_clicked() {
    QString appDir = QCoreApplication::applicationDirPath(); MyApplication *pApp = (MyApplication *) qApp; double mmPerPixel = 0.09473;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    if (m_calibManager.finalizeGlobalCalibration(mmPerPixel, appDir + "/Camera_Master_1DLUT.hdict", appDir + "/Camera_Slave_1DLUT.hdict")) {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: green;"); ui->lbl_calibStatus->setText(QString("🎉 自标定大获成功！融合 %1 趟点云，参数已重写。").arg(m_calibManager.getPassesCount()));
        if (pApp && !pApp->m_workstationList.isEmpty()) QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess, "slot_reloadCalibration", Qt::QueuedConnection);
    } else {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: red;"); ui->lbl_calibStatus->setText("标定解算失败，请检查文件权限。");
    }
    m_isInCalibrationMode = false; ui->btn_startCalib->setEnabled(true); ui->btn_pauseCalib->setEnabled(false); ui->btn_executeCalib->setEnabled(false); ui->btn_removeLastPass->setEnabled(false);
}

void DlgCalibration::on_btn_cancelCalib_clicked() {
    m_isInCalibrationMode = false; m_isCalibPaused = false; m_calibManager.resetGlobalCalibration();
    m_thicknessPoints.clear(); // 🌟 退出时清空厚度拟合池
    this->close();
}
// 将这段代码追加到 dlgcalibration.cpp 的最底下即可：

// =======================================================================
// 🌟 终极防呆机制：只要标定弹窗浮现在屏幕上，立刻执行物理清场！
// 无论工人上次是怎么关掉窗口的（点按钮、点红叉、甚至任务管理器杀进程重启），
// 都能保证每次进来看到的都是一个干干净净的全新拟合池。
// =======================================================================
void DlgCalibration::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    // 1. 强制清空厚度拟合池
    m_thicknessPoints.clear();

    // 2. 强制复位内部状态机
    m_currentStep = 0;
    m_isInCalibrationMode = false;
    m_isCalibPaused = false;

    // 3. 恢复 UI 按钮默认状态
    ui->btn_startCalib->setEnabled(true);
    ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);

    // 4. 给操作员安心的文字提示
    ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: #00ff00;");
    ui->lbl_calibStatus->setText("当前状态：拟合池已清空，准备好进行全新标定...");
}