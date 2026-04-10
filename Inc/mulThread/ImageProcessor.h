//
// Created by sylar on 25-10-24.
//

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "halcon_inc.h"
#include <QObject>
#include <QString>
#include "imgAI.h"
#include "SharedMemoryDataStructure.h"


class UploadInfo {
public:
    int has_steel_plate{0}; // 辊道上是否有钢板 <1-有|0-无>
    int gundao_X_left{0}; // 辊道上，钢板沿辊道方向从左数第1块钢板<左边>坐标
    QString ID{""}; // 辊道上，钢板的板号
    QString img_name{""}; // 翻板台上，钢板的图片名
    int recog_finished_flag{0}; // 辊道上，该块钢板是否识别完成 <1-完成|0-未完成>
    int delete_flag{0};  //删除反向回流的板子
    double steel_length{0}; //板坯长度
    int has_cam_broken{0}; // 相机是否故障 <1-有|0-无>
    int conf{1}; //板号值置信度
};

class ImageProcessor : public QObject {
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr) : QObject(parent) {
    }

    virtual ~ImageProcessor() = default;

    // 图像处理接口，子类必须实现
    virtual void processImage(const HObject &img, const DetRes &detRes, const std::vector<ocrInfo> &ocrRes) {
    }

    // 可选的初始化接口
    virtual void init(const QString &det_model_path, const QString &det_param_path,
                      const QString &ocr_det_path, const QString &ocr_recog_path,
                      const HTuple &hv_x_pre, const HTuple &hv_y_pre,
                      const HTuple &hv_x_post, const HTuple &hv_y_post,
                      const int &max_age, const int &min_hits, const double &iouThreshold) {
    }

    virtual void init(
        const HTuple &hv_x_pre, const HTuple &hv_y_pre,
        const HTuple &hv_x_post, const HTuple &hv_y_post,
        const std::vector<int> &region,
        const int &max_age, const int &min_hits, const double &iouThreshold) {
    }

    // 可选的清理接口
    virtual void cleanup() {
    }

    virtual void AffineImg(const HObject &src, HObject &affine) {
    }

    // void FindDevice(const QString &deviceType);
    // const HTuple FindDevice(const QString &deviceType);

    DetRes detInfer(const HObject &image) {
        return ModelManager::getInstance().detInfer(image);
    }

    std::vector<ocrInfo> ocrInfer(const HObject &image, const QString &type) {
        return ModelManager::getInstance().ocrInfer(image, type);
    }

    QString className;
    int m_serialNumber; //工位序号
    QString ocrBufferPath, detBufferPath, remotePath; //本地存图路径和远程存图路径
    cv::Rect_<int> m_region;
    int frameNumber;
    UploadInfo info;
    From_PLC_DATA plcData;
    // std::unordered_map<int, PlateObject> hotPlateMap, stackerPlateMap; //钢板vector容器
    HalconPreprocessing m_ins_PreProcessing;
    // SORTTracker *m_tracker = nullptr;
    HTuple winHandle_pro;
    int img_width, img_height, left_half_width, left_one_third_width;
};


#endif //IMAGEPROCESSOR_H
