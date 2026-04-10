//
// Created by wcy on 25-8-1.
//

#ifndef HALCONDET_H
#define HALCONDET_H

#include <Det/HalconDetInfer.h>
#include "opencv2/opencv.hpp"

struct DetectionResult {
    cv::Rect_<int> boundingBox;
    float confidence;
    QString label;

    DetectionResult(cv::Rect_<int> rect, float conf, QString lab)
        : boundingBox(std::move(rect)), confidence(conf), label(std::move(lab)) {}
};

class DetRes {

private:
    std::vector<DetectionResult> originalResults;

public:
    // 添加检测结果
    void addDetection(cv::Rect_<int> rect, float confidence, QString label) {
        originalResults.emplace_back(std::move(rect), confidence,label);
    }

    void addDetection(DetectionResult det) {
        originalResults.emplace_back(det);
    }

    // 批量添加检测结果
    void addDetections(const std::vector<cv::Rect_<int>>& rects,
                      const std::vector<float>& confidences, QString label) {
        if (rects.size() != confidences.size())
            return;

        for (size_t i = 0; i < rects.size(); ++i) {
            originalResults.emplace_back(rects[i], confidences[i],label);
        }
    }

    // 获取原始检测结果
    std::vector<DetectionResult> getOriginalResults() {
        return originalResults;
    }

    // 获取指定类别的检测结果
    std::vector<DetectionResult> getResultsByLabel(const QString& label) const {
        std::vector<DetectionResult> filtered;
        std::copy_if(originalResults.begin(), originalResults.end(),
                     std::back_inserter(filtered),
                     [&label](const DetectionResult& result) {
                         return result.label == label;
                     });
        return filtered;
    }

    // 获取指定类别的检测框
    std::vector<cv::Rect_<int>> getRectsByLabel(const QString& label) const {
        std::vector<cv::Rect_<int>> rects;
        for (const auto& result : originalResults) {
            if (result.label == label) {
                rects.push_back(result.boundingBox);
            }
        }
        return rects;
    }

    // 获取指定类别的置信度
    std::vector<float> getConfsByLabel(const QString& label) const {
        std::vector<float> confs;
        for (const auto& result : originalResults) {
            if (result.label == label) {
                confs.push_back(result.confidence);
            }
        }
        return confs;
    }

    // 获取原始检测框
    std::vector<cv::Rect_<int>> getOriginalRects() const{
        std::vector<cv::Rect_<int>> rects;
        rects.reserve(originalResults.size());
        for (const auto& result : originalResults) {
            rects.push_back(result.boundingBox);
        }
        return rects;
    }

    // 获取原始置信度
    std::vector<float> getOriginalConfs() const {
        std::vector<float> confs;
        confs.reserve(originalResults.size());
        for (const auto& result : originalResults) {
            confs.push_back(result.confidence);
        }
        return confs;
    }

    // 获取所有不重复的类别标签
    std::vector<QString> getUniqueLabels() const {
        std::vector<QString> labels;
        for (const auto& result : originalResults) {
            if (std::find(labels.begin(), labels.end(), result.label) == labels.end()) {
                labels.push_back(result.label);
            }
        }
        return labels;
    }

    // 检查是否包含指定类别
    bool hasLabel(const QString& label) const {
        return std::any_of(originalResults.begin(), originalResults.end(),
                          [&label](const DetectionResult& result) {
                              return result.label == label;
                          });
    }

    // 获取指定类别的检测数量
    int getCountByLabel(const QString& label) const {
        return static_cast<int>(std::count_if(originalResults.begin(), originalResults.end(),
                                             [&label](const DetectionResult& result) {
                                                 return result.label == label;
                                             }));
    }

    // 生成简化的检测结果信息
    std::string toSimpleString() const {
        std::stringstream ss;

        if (originalResults.empty()) {
            ss << "No detections.";
            return ss.str();
        }

        ss << "Detections: " << originalResults.size()
           << " objects found. First box: ("
           << originalResults[0].boundingBox.x << ", "
           << originalResults[0].boundingBox.y << ", "
           << originalResults[0].boundingBox.width << ", "
           << originalResults[0].boundingBox.height << ")";

        return ss.str();
    }
    // 清除所有结果
    void clear() {
        originalResults.clear();
    }

};

class HalconDet {
public:
    HalconDet() = default;
    //目标检测模型；数据集参数
    void Init(std::string modelPath, std::string preprocessParamPath, const HTuple& hv_DLDeviceHandle);
    DetRes Infer(const HObject &img);

    HTuple hv_PreprocessParam, hv_DLModelHandle;
    HTuple hv_DLDeviceHandles, hv_DLDevice;
    HTuple hv_imgWidth, hv_imgHeight;
};

#endif //HALCONDET_H
