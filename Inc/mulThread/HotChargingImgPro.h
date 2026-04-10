//
// Created by sylar on 25-10-24.
//

#ifndef HOTCHARGINGIMGPRO_H
#define HOTCHARGINGIMGPRO_H

#include "ImageProcessor.h"

class HotChargingImgPro : public ImageProcessor {
    Q_OBJECT

public:
    explicit HotChargingImgPro(QObject *parent = nullptr);

    void processImage(const HObject &img, const DetRes &detRes, const std::vector<ocrInfo> &ocrRes) override;

    // 初始化
    void init(
        const HTuple &hv_x_pre, const HTuple &hv_y_pre,
        const HTuple &hv_x_post, const HTuple &hv_y_post,
        const std::vector<int> &region,
        const int &max_age, const int &min_hits, const double &iouThreshold) override;

    void AffineImg(const HObject &src, HObject &affine);

    // 坐标转换
    int coordinateXTrans(int imageX);

    // 发送信息
    void SendInfo();

    // 保存和发送图片
    void SaveImage(const HObject &img, const cv::Rect_<int> &box, const QString &plateNumber);

    void SendImage(const QString &plateNumber);

    void SendImage(const HObject &img);

    // 获取辊道流向
    void GetGundaoMachineState();

    // 更新钢板位置信息
    void UpdatePlatePositions(const std::vector<cv::Rect_<int> > &gundao_boxes);

    // 更新OCR统计
    void UpdateOCRStatistics(const HObject &img, const std::vector<ocrInfo> &ocrRes);

    // 检查并处理上报
    void CheckAndReport(const HObject &img, const std::vector<cv::Rect_<int> > &gundao_boxes,
                        const std::vector<ocrInfo> &ocrRes);

    void cleanup() override;

    void CleanBufferImg(QString plateNumber, QMap<QString, int> ocr_count);

    bool shouldReportPlate(const QString &plateNumber);

    void updateRecentPlatesNumber(const QString &newPlate);

private:
    bool m_initialized = false;

    QQueue<QString> recentPlates;
    const int MAX_RECENT_PLATES = 5;

    // 物理坐标转换参数
    int imageXMax = 1420;
    int physicalLeft = 2400;
    int physicalRight = 0;
    double ratioX;

    // 钢板数量管理
    int gundao_last_count = 0; // 上一帧辊道钢板数量

    // 辊道和翻板台各自的OCR统计
    // QMap<QString, int> gundao_ocr_count;   // 辊道：板号 -> 出现次数

    // 上报状态标记
    bool gundao_reported = false;

    QString lastGundao_ID{""}; //改成缓存5个号

    const int REPORT_THRESHOLD = 3; // 上报阈值

    int width{2000}, height{900};

    int gundao_flow_direction{1}, gundao_current_direction{0}; //流向默认是正向

    // 钢板信息结构
    struct PlateInfo {
        cv::Rect rect; // 钢板位置
        QString plate_number; // 板号（如果有识别到）
        bool reported; // 是否已上报过
        // bool isFullView;                  // 是否全视野
        // int ocr_count;                    // OCR识别次数
        QMap<QString, int> gundao_ocr_count; // 统计板号出现次数
        // int gundao_X_left; //头部位置
        double conf{1}; //置信度
        int total_ocr_count{0};     // 这块钢板被OCR识别的总次数
        int valid_ocr_count{0};     // 这块钢板OCR识别判定为可信的总次数
        double steel_length{0}; // 板坯长度
        bool is_reverse_plate; // 是否为反向移动中的钢板

        PlateInfo() : reported(false), is_reverse_plate(false) {
        }

        // 重置函数
        void reset() {
            plate_number = "";
            reported = false;
            gundao_ocr_count.clear();
            is_reverse_plate = false;
            steel_length = 0;
            conf = 1.0;
            total_ocr_count = 0;
            valid_ocr_count = 0;
        }

        // 添加 swap 成员函数
        void swap(PlateInfo &other) {
            std::swap(rect, other.rect);
            std::swap(plate_number, other.plate_number);
            std::swap(reported, other.reported);
            std::swap(gundao_ocr_count, other.gundao_ocr_count);
            std::swap(is_reverse_plate, other.is_reverse_plate);
            std::swap(steel_length, other.steel_length);
            // std::swap(gundao_X_left, other.gundao_X_left);
            std::swap(conf, other.conf);
            std::swap(total_ocr_count, other.total_ocr_count);
            std::swap(valid_ocr_count, other.valid_ocr_count);
        }
    };

    // 钢板列表（按从左到右顺序）
    std::vector<PlateInfo> plates;

    // 当前画面中的钢板数量（用于检测变化）
    int last_plate_count = 0;

    // 上一次最左侧钢板的位置（用于检测左边钢板移出）
    cv::Rect last_leftmost_plate;

    // 上一次最右侧钢板的位置（用于检测右边钢板移出）
    cv::Rect last_rightmost_plate;

    // 定义区间信息结构
    struct PixelInterval {
        double left_px; // 区间左边界像素坐标
        double right_px; // 区间右边界像素坐标
        double meters_per_px; // 每像素的物理距离（米/像素）
    };

    // 预先计算好的区间信息
    const std::vector<PixelInterval> INTERVALS = {
        {0.0, 310.0, 0.8 / 510.0}, // 区间1: 0-310 (使用310-820区间的比例)
        {310.0, 820.0, 0.8 / 510.0}, // 区间2: 310-820 (0.8米)
        {820.0, 1300.0, 0.8 / 480.0}, // 区间3: 820-1300 (0.8米)
        {1300.0, 1765.0, 0.8 / 465.0}, // 区间4: 1300-1765 (0.8米)
        {1765.0, 2000.0, 0.8 / 465.0} // 区间5: 1765-2000 (使用1300-1765区间的比例)
    };

    double calculateDistance(double left_px, double right_px);

    // 辅助函数
    void UpdatePlatesForForwardMovement(const std::vector<cv::Rect_<int> > &sorted_boxes);

    void UpdatePlatesForReverseMovement(const std::vector<cv::Rect_<int> > &sorted_boxes);

    int ModifyPlateNumber(QString &plate_number, bool isValid);

    void MatchOCRToPlates(const HObject &img, const std::vector<ocrInfo> &ocrRes);

    void ProcessForwardLogic(const HObject &img);

    void ProcessReverseLogic();

    bool IsLeftPlateLeaving(const std::vector<cv::Rect_<int> > &sorted_boxes);

    bool IsRightPlateLeaving(const std::vector<cv::Rect_<int> > &sorted_boxes);


    HObject m_bgImg;
};


#endif //HOTCHARGINGIMGPRO_H
