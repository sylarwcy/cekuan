// StackerTableImgPro.h
//
// Created by sylar on 25-10-24.
//

#ifndef STACKERTABLEIMGPRO_H
#define STACKERTABLEIMGPRO_H

#include "ImageProcessor.h"
#include <QMap>

class StackerTableImgPro : public ImageProcessor {
    Q_OBJECT

public:
    explicit StackerTableImgPro(QObject *parent = nullptr);

    // 实现基类的图像处理接口
    void processImage(const HObject &img, const DetRes &detRes, const std::vector<ocrInfo> &ocrRes) override;

    void AllocationBox(const DetRes &detRes, std::vector<cv::Rect_<int> > &plate_boxes);

    // 初始化
    void init(
        const HTuple &hv_x_pre, const HTuple &hv_y_pre,
        const HTuple &hv_x_post, const HTuple &hv_y_post,
        const std::vector<int> &region,
        const int &max_age, const int &min_hits, const double &iouThreshold) override;

    void AffineImg(const HObject &src, HObject &affine);

    void SaveImage(const HObject &img, const cv::Rect_<int> &box, const QString &plateNumber);

    void SendImage(const QString &plateNumber);

    void SendImage(const HObject &img);

    void SendInfo();

    void cleanup() override;

private:
    bool m_initialized = false;

    // 推钢机状态枚举
    enum PusherState {
        PUSHING, // 正转推进
        RETRACTING, // 反转收回
        PAUSED, // 暂停
    };

    std::map<PusherState, QString> colorMap = {
        {PUSHING, "1"},
        {RETRACTING, "-1"},
        {PAUSED, "0"}
    };

    PusherState m_pusherStatus;
    PusherState m_lastPusherStatus; // 上一帧状态

    // 识别状态
    bool m_hasStartedRecognition = false; // 是否已开始识别
    bool m_hasReported = false; // 是否已上报

    // 统计变量
    int m_tableDetectionCount = 0; // table检测次数
    int m_totalDetectionFrames = 0; // 总检测帧数
    // int m_recognizingFrameCount = 0;//

    // OCR收集相关
    int m_maxOcrCollectionFrames = 10; // 最大收集帧数
    int m_ocrStopThreshold = 10; // 停止收集的条件，交叉像素阈值

    // 板号统计
    QMap<QString, int> m_plateNumberCounter; // 板号出现次数统计
    QMap<QString, int> m_plateValidCounter;  // 板号判定为可信的次数统计

    bool statusChanged;
    int m_pushThreshold = 300; // 推到位阈值
    // bool initPositionFlag_LOG{false};
    bool m_hasUpdatedTemplate = false;

    HObject m_startImg, m_InitPositionImg, m_InitPositionRegion;
    HTuple hv_ModelID;

    int logPrintCount{0};
    int initPositionCount{0};

    bool shouldStopOCR;

    int m_retractFrameCount; // 记录RETRACTING状态持续的帧数
    bool m_skipNextFrames; // 是否需要跳过接下来的帧
    int m_framesToSkip; // 需要跳过的帧数

    QQueue<QString> recentPlates;
    const int MAX_RECENT_PLATES = 5;

    void updateDetectionStatistics(const HObject &img, const cv::Rect_<int> &pusherBox); // 更新检测统计
    void collectOcrResult(const HObject &img, const std::vector<ocrInfo> &ocrRes, const cv::Rect_<int> &pusherBox);

    // 收集OCR结果
    bool shouldStopOcrCollection(const std::vector<ocrInfo> &ocrRes, const cv::Rect_<int> &pusherBox); // 判断是否停止OCR收集
    void processPushCompletion(); // 处理推到位情况
    void UpdatePusherStatus(const HObject &img);

    cv::Rect_<int> GetBox(const DetRes &detRes);

    bool IsAtInitPosition(const HObject &img);

    bool HasPlate(const HObject &img, const cv::Rect_<int> &pusherBox);

    void updateRecentPlatesNumber(const QString &newPlate);

    int ModifyPlateNumber(QString &plate_number, bool isValid);
};

#endif //STACKERTABLEIMGPRO_H
