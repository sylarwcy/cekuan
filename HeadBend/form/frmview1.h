#ifndef FRMVIEW1_H
#define FRMVIEW1_H

#include <QWidget>
#include <QLineEdit> // 确保包含了 QLineEdit
#include "qcustomplot.h"
#include "MyApplication.h"

namespace Ui {
    class frmView1;
}

struct PlateRecord {
    QString plateID{"001"};       // 板号
    double length{0};         // 长度
    double thickness{0};      // 厚度
    double targetWidth{0};    // 设定宽度
    double avgWidth{0};       // 平均宽度
    double maxWidth{0};       // 最大宽度
    double minWidth{0};       // 最小宽度
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

    QList<HalconCpp::HObject> m_preBufferList;
    const int HEAD_FRAME_COUNT = 3;  // 【补头配置】：提前缓存 3 帧作为板头

    // --- 曲线图操作函数 ---
    void initCurveChart();           // 初始化曲线图表样式

    // --- 全景拼接相关 ---
    HalconCpp::HObject m_hFusedImage; // 存储拼接后的完整大图
    bool m_isFirstFrame;            // 标记是否是新钢板的第一帧

    void initFusionView();          // 初始化全景窗口
    void resetFusion();             // 清空拼接大图
    void addFrameToFusion(HalconCpp::HObject newFrame); // 执行拼接、旋转与显示

    // --- 本地历史数据相关 ---
    QList<PlateRecord> m_historyList;        // 内存中缓存的最近5条记录
    QStandardItemModel *m_tableModelHistory; // 用于界面显示的表格模型

    void initHistoryUI();                    // 初始化表格
    void loadHistoryFromFile();              // 软件启动时加载二进制文件
    void saveHistoryToFile();                // 每次更新后保存到二进制文件
    void updateHistoryTable();               // 刷新界面表格

    // 添加一条新记录
    void addHistoryRecord(const QString& plateID, double length, double thickness,
                          double targetW, double avgW, double maxW, double minW);

private slots:
    void onUpdateRawImage(const DualCameraChunk &chunk);

    void addCurvePoint(double widthValue); // 槽：增加一个点并刷新图表
    void clearCurveChart();                // 槽：新钢板到来时清空图表
private:
    // ... 原本的变量 ...

    // --- 新增：测宽数据统计专用变量 ---
    double m_sumWidth = 0.0;      // 累加总宽度
    int m_validFrameCount = 0;    // 有效检测帧数
    int m_totalRows = 0;          // 总行数(用于算长度)
    double m_maxWidth = 0.0;      // 最大宽度
    double m_minWidth = 99999.0;  // 最小宽度
private:
    // ... 原有变量 ...
    HalconCpp::HTuple winHandle_front; // 头部显示窗口
    HalconCpp::HTuple winHandle_back;  // 尾部显示窗口
    HalconCpp::HObject m_hLastValidImage; // 临时记录最后一帧有效图，用于出尾显示
};

#endif // FRMVIEW1_H