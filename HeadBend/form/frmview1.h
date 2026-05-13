#ifndef FRMVIEW1_H
#define FRMVIEW1_H

#include <QWidget>
#include <QLineEdit> // 确保包含了 QLineEdit
#include "qcustomplot.h"
#include "MyApplication.h"

namespace Ui {
    class frmView1;
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
    HTuple winHandle_fusion; // 全景图窗口句柄

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

    // 接收测量结果的槽函数
    void onMeasureReady(const WidthResult &res);

public:
    void initForm();

private:
    // 【新增】：自适应字体大小的工具函数声明
    void adjustFontSize(QLineEdit* lineEdit);

    // --- 曲线图相关缓存数据 ---
    QVector<double> m_vecFrameIndex; // X轴数据：第几帧/第几次测量
    QVector<double> m_vecWidthValue; // Y轴数据：测量的宽度结果(mm)
    int m_currentFrameCount;         // 计数器：当前钢板拍了多少张

    // --- 纯视觉触发状态机变量（新增） ---
    bool m_isPlatePresent;       // 状态：当前钢板是否在视野内
    int m_emptyFrameCount;       // 计数器：连续没有检测到钢板的帧数
    const int EMPTY_FRAME_LIMIT = 3; // 防抖阈值：连续3帧没看到，才认为板子真走了

    // --- 曲线图操作函数 ---
    void initCurveChart();           // 初始化曲线图表样式

    // --- 全景拼接相关 ---
    HalconCpp::HObject m_hFusedImage; // 存储拼接后的完整大图
    bool m_isFirstFrame;            // 标记是否是新钢板的第一帧

    void initFusionView();          // 初始化全景窗口
    void resetFusion();             // 清空拼接大图
    void addFrameToFusion(HalconCpp::HObject newFrame); // 执行拼接、旋转与显示

private slots:
    void onUpdateRawImage(const DualCameraChunk &chunk);

    void addCurvePoint(double widthValue); // 槽：增加一个点并刷新图表
    void clearCurveChart();                // 槽：新钢板到来时清空图表

};

#endif // FRMVIEW1_H