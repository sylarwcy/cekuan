//
// Created by sylar on 25-12-2.
//

#ifndef IMGPROCESSTOOL_H
#define IMGPROCESSTOOL_H
#include <imgAI.h>
// #include <HObject.h>
// #include <core/types.hpp>


namespace ImgProcessTool {
    cv::Point GetRectCenter(const cv::Rect_<int>& rect);
    bool IsPointInRegion(const cv::Rect_<int>& region, const cv::Point& point);
    void SortRectsByX(std::vector<cv::Rect_<int>>& boxes); //x,从小到大排序
    void SortRectsByY(std::vector<cv::Rect_<int>>& boxes); //y,从大到小排序
    bool isFormatValid(const QString &str);
    void PreProcessOCRRes(std::vector<ocrInfo>& ocrRes);
    ocrInfo FilterOCRResult(const ocrInfo &inputOcrRes);
    std::vector<ocrInfo> FilterOCRResult(const std::vector<ocrInfo> &ocrRes);
    double calculateIOU(const cv::Rect_<int>& rect1, const cv::Rect_<int>& rect2);
    void filterOverlappingBoxes(DetRes &detRes);
    const HTuple FindDevice(const QString &deviceType);
}



#endif //IMGPROCESSTOOL_H
