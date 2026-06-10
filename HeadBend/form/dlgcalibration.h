// ==================== [ dlgcalibration.h ] ====================
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

    // 🌟 公共获取接口：供主界面结算钢板时直接获取当前小页面设定的倍率值
    int getMultiplier() const;

    // 供主窗体在高频收到相机行数据时，异步喂入点云
    void feedFrame(const WidthResult& res);

    // 供主窗体在一趟车尾完全滑出时，触发单趟偏航计算
    void finishPass();

    signals:
        // 🌟 自定义信号：当小页面的 SpinBox 被调节时，通知主界面曲线进行即时重绘刷新
        void sig_multiplierChanged(int value);
        void sig_speedChanged(double speed_m_s); // 🌟 新增：向外发射换算成 m/s 的速度信号

private slots:
    void on_btn_startCalib_clicked();
    void on_btn_pauseCalib_clicked();
    void on_btn_removeLastPass_clicked();
    void on_btn_executeCalib_clicked();
    void on_btn_cancelCalib_clicked();
    void slot_speedEditingFinished();        // 🌟 新增：响应输入框输入完成的槽函数

    // 🌟 自定义槽函数：用于安全承接 SpinBox 数值改变信号，规避双击触发Bug
    void slot_spinSegmentChanged(int value);

private:
    Ui::DlgCalibration *ui;
    bool m_isInCalibrationMode{false};
    bool m_isCalibPaused{false};
    PlateCalibrationManager m_calibManager;
};

#endif // DLGCALIBRATION_H