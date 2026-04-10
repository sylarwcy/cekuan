//
// Created by wcy on 25-8-1.
//

#ifndef HALCONOCR_H
#define HALCONOCR_H

#include <OCR/HalconOCRInfer.h>
#include <vector>
#include <opencv2/opencv.hpp>

struct ocrInfo {
    // int charLength = 0; //字符位数
    QString word = ""; //字符串内容
    cv::Point point, srcImgPoint; //中心点
    HObject img;
    double length1, length2, phi, minConf{1.1}; //长轴、短轴、倾斜角度
    bool isValid{false}; //是否为合格的字符
    // std::vector<std::pair<std::string, double>> charInfoVec; //存储每个单字符的文本和置信度

    cv::Rect_<int> GetRectSrc() const {
        cv::Rect_<int> rect{0, 0, 0, 0};
        rect.x = srcImgPoint.x - length1;// * 1.5
        rect.y = srcImgPoint.y - length2;// * 2
        rect.width = length1 * 2;// * 3
        rect.height = length2 * 2;//* 4
        return rect;
    }

    cv::Rect_<int> GetRect() const {
        cv::Rect_<int> rect{0, 0, 0, 0};
        rect.x = point.x - length1;
        rect.y = point.y - length2;
        rect.width = length1 * 2;
        rect.height = length2 * 2;
        return rect;
    }
};

class HalconOCR {
    HTuple hv_DLDevice, hv_DeepOcrHandle, hv_Exception, hv_DeepOcrHandle_crop; //, hv_RecognitionImageWidthDefault
    std::string det1ModelPath, det2ModelPath, rec2ModelPath;
    std::vector<std::pair<std::string, cv::Point2d> > res;

    void SetOCRModelParam(const HTuple &hv_DLDeviceHandles);

public:
    HalconOCR() = default;

    //目标检测模型；数据集参数
    void Init(std::string det1ModelPath, std::string det2ModelPath, std::string rec2ModelPath, const HTuple &hv_DLDeviceHandles);

    void RenderBox(const HObject &img, HObject *renderImg, std::vector<ocrInfo> &res);

    std::vector<ocrInfo> Infer(const HObject &img, const QString &type);

    void Infer_StackerTable(const HObject &img, std::vector<ocrInfo> &res);

    void Infer_HotCharging(const HObject &img, std::vector<ocrInfo> &res);

    std::vector<ocrInfo> ParseOCRResults(const HTuple &ocrResult);

    void SaveImg(const HObject &image, const std::vector<QString> &ocrRes, const QString &ocrBufferPath);
};


#endif //HALCONOCR_H
