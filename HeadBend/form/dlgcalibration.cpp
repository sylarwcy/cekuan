// ==================== [ dlgcalibration.cpp ] ====================
#include "dlgcalibration.h"
#include "MyApplication.h"
#include "ui_dlgcalibration.h"

DlgCalibration::DlgCalibration(QWidget *parent) : QDialog(parent), ui(new Ui::DlgCalibration) {
    ui->setupUi(this);

    // 让弹窗变为独立的可自由拖动、带有关闭按钮的标准独立小页面
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("标准样坯在线自标定管理系统");

    // 依据你更新的布局调整弹窗初始大小，防止内容拥挤
    this->resize(750, 480);
    this->setMinimumSize(750, 480);

    ui->btn_pauseCalib->setEnabled(false);
    ui->btn_executeCalib->setEnabled(false);
    ui->btn_removeLastPass->setEnabled(false);

    // 🌟 安全的显式信号连接：避免自动命名规范导致的重复触发Bug
    connect(ui->spinSegment, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DlgCalibration::slot_spinSegmentChanged);

    // 绑定到 DlgCalibration::DlgCalibration 构造函数的末尾
    connect(ui->lineEdit, &QLineEdit::editingFinished,
            this, &DlgCalibration::slot_speedEditingFinished);
}

DlgCalibration::~DlgCalibration() {
    delete ui;
}

int DlgCalibration::getMultiplier() const {
    // 🌟 实现对外接口：无条件返回当前 UI 上的真实倍率数值
    return ui->spinSegment->value();
}

void DlgCalibration::slot_spinSegmentChanged(int value) {
    // 🌟 信号路由中转：直接向下发传递
    emit sig_multiplierChanged(value);
}

void DlgCalibration::feedFrame(const WidthResult& res) {
    if (m_isInCalibrationMode && !m_isCalibPaused) {
        m_calibManager.feedCalibrationFrame(res);
    }
}

// ==================== [ dlgcalibration.cpp ] ====================
void DlgCalibration::finishPass() {
    double trueW = ui->lineEdit_calibWidth->text().toDouble();
    if(trueW <= 0) trueW = 1500.0;

    // 🌟 动态获取单视场金标准：向全局唯一的相机驱动索取横向物理尺寸
    MyApplication *pApp = (MyApplication *) qApp;
    double mmPerPixel = 0.09473; // 安全兜底防呆值
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) {
        mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    }

    // 将动态获取到的 mmPerPixel 代入最小二乘和虚宽剥离公式中
    bool success = m_calibManager.finishCurrentPass(trueW, mmPerPixel);
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

// ==================== [ dlgcalibration.cpp ] ====================
void DlgCalibration::on_btn_executeCalib_clicked() {
    // 🌟 路径与名称 100% 刚性对齐
    QString appDir = QCoreApplication::applicationDirPath();
    QString mPath = appDir + "/Camera_Master_1DLUT.hdict";
    QString sPath = appDir + "/Camera_Slave_1DLUT.hdict";

    // 动态获取相机标定常数，保证重写后的 Coef_c 线性项与镜头绝对 1:1 物理对齐
    MyApplication *pApp = (MyApplication *) qApp;
    double mmPerPixel = 0.09473;
    if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerCamera) {
        mmPerPixel = pApp->m_workstationList[0]->m_pWorkerCamera->m_mmPerPixelX;
    }

    bool success = m_calibManager.finalizeGlobalCalibration(mmPerPixel, mPath, sPath);
    if (success) {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: green;");
        ui->lbl_calibStatus->setText(QString("🎉 自标定大获成功！融合 %1 趟点云，参数已重写存盘。").arg(m_calibManager.getPassesCount()));

        // =======================================================================
        // 🌟【核心解算闭环】：通过神经总线跨线程安全通知算法工作线程热装载新字典。
        // 使用 Qt::QueuedConnection 投递，算法线程会在处理完当前图像行的瞬间顺畅重读，
        // 绝不影响拉流，且立刻生效！
        // =======================================================================
        if (pApp && !pApp->m_workstationList.isEmpty() && pApp->m_workstationList[0]->m_pWorkerImageProcess) {
            QMetaObject::invokeMethod(pApp->m_workstationList[0]->m_pWorkerImageProcess,
                                      "slot_reloadCalibration",
                                      Qt::QueuedConnection);
        }

    } else {
        ui->lbl_calibStatus->setStyleSheet("font-weight: bold; color: red;");
        ui->lbl_calibStatus->setText("标定解算失败，请检查物理磁盘文件读写权限。");
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
    this->close();
}

void DlgCalibration::slot_speedEditingFinished() {
    // 1. 获取工人填写的 mm/s 速度值（例如输入 380）
    double speed_mm_s = ui->lineEdit->text().toDouble();
    if (speed_mm_s <= 0.0) return;

    // 2. 自动换算为相机底层需要的 m/s 单位（380 mm/s -> 0.38 m/s）
    double speed_m_s = speed_mm_s / 1000.0;

    // 3. 将换算后的标准速度跨窗体发射出去
    emit sig_speedChanged(speed_m_s);

    ui->lbl_calibStatus->setText(QString("辊道速度已更新为: %1 mm/s，正在动态调整相机物理行频...").arg(speed_mm_s));
}