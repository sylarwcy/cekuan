//
// Created by sylar on 25-12-2.
//

#include "ImgProcessTool.h"
#include "QsLog.h"
#include "halconcpp/HalconCpp.h"
#include "halconcpp/HDevThread.h"

cv::Point ImgProcessTool::GetRectCenter(const cv::Rect_<int> &rect) {
    return {rect.x + rect.width / 2, rect.y + rect.height / 2};
}

bool ImgProcessTool::IsPointInRegion(const cv::Rect_<int> &region, const cv::Point &point) {
    HTuple hv_res_f;
    // TestRegionPoint(region, point.y, point.x, &hv_res_f);
    if (region.contains(point))
        return true;

    return false;
}

void ImgProcessTool::SortRectsByX(std::vector<cv::Rect_<int> > &boxes) {
    std::sort(boxes.begin(), boxes.end(),
              [](const cv::Rect_<int> &a, const cv::Rect_<int> &b) {
                  return a.x < b.x; // 按x坐标从小到大排序
              });
}

void ImgProcessTool::SortRectsByY(std::vector<cv::Rect_<int> > &boxes) {
    std::sort(boxes.begin(), boxes.end(),
              [](const cv::Rect_<int> &a, const cv::Rect_<int> &b) {
                  return a.y > b.y; // 按y坐标从大到小排序
              });
}

// 计算两个矩形的IOU
double ImgProcessTool::calculateIOU(const cv::Rect_<int> &rect1, const cv::Rect_<int> &rect2) {
    // 计算交集矩形的坐标
    int interLeft = std::max(rect1.x, rect2.x);
    int interTop = std::max(rect1.y, rect2.y);
    int interRight = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
    int interBottom = std::min(rect1.y + rect1.height, rect2.y + rect2.height);

    // 检查是否有交集
    if (interRight <= interLeft || interBottom <= interTop) {
        return 0.0;
    }

    // 计算交集面积
    int interArea = (interRight - interLeft) * (interBottom - interTop);

    // 计算并集面积
    int area1 = rect1.width * rect1.height;
    int area2 = rect2.width * rect2.height;
    int unionArea = area1 + area2 - interArea;

    // 避免除零错误
    if (unionArea <= 0) {
        return 0.0;
    }

    // 返回IOU
    return static_cast<double>(interArea) / unionArea;
}

// 过滤检测结果：当两个box的IOU不为0时，删除面积小的那个box
void ImgProcessTool::filterOverlappingBoxes(DetRes &detRes) {
    // 获取原始检测结果
    auto originalResults = detRes.getOriginalResults();

    if (originalResults.empty()) {
        return;
    }

    // 创建临时容器，用于存储最终结果
    std::vector<DetectionResult> filteredResults;
    filteredResults.reserve(originalResults.size());

    // 用于标记需要保留的结果
    std::vector<bool> toKeep(originalResults.size(), true);

    // 遍历所有检测结果对
    for (size_t i = 0; i < originalResults.size(); ++i) {
        if (!toKeep[i]) {
            continue; // 如果已经标记为删除，跳过
        }

        for (size_t j = i + 1; j < originalResults.size(); ++j) {
            if (!toKeep[j]) {
                continue; // 如果已经标记为删除，跳过
            }

            // 计算IOU
            double iou = calculateIOU(
                originalResults[i].boundingBox,
                originalResults[j].boundingBox
            );

            // 如果IOU不为0，标记面积小的那个为删除
            if (iou > 0.0) {
                int area_i = originalResults[i].boundingBox.width *
                             originalResults[i].boundingBox.height;
                int area_j = originalResults[j].boundingBox.width *
                             originalResults[j].boundingBox.height;

                if (area_i < area_j) {
                    toKeep[i] = false; // 删除面积小的i
                    break; // i已被标记删除，跳出内层循环
                } else {
                    toKeep[j] = false; // 删除面积小的j
                }
            }
        }
    }

    // 收集需要保留的结果
    for (size_t i = 0; i < originalResults.size(); ++i) {
        if (toKeep[i]) {
            filteredResults.push_back(originalResults[i]);
        }
    }

    // 清空原始结果并添加过滤后的结果
    detRes.clear();
    for (const auto &result: filteredResults) {
        detRes.addDetection(result.boundingBox, result.confidence, result.label);
    }

    // // 输出调试信息
    // if (originalResults.size() != filteredResults.size()) {
    //     std::cout << "过滤重叠框：原始数量=" << originalResults.size()
    //               << "，过滤后数量=" << filteredResults.size() << std::endl;
    // }
}

bool ImgProcessTool::isFormatValid(const QString &str) {
    return (str.length() >= 7) &&
           str[0].isLetter() &&
           str[1].isLetter() &&
           str[2].isDigit() &&
           str[3].isDigit() &&
           str[4].isDigit() &&
           str[5].isDigit() &&
           str[6].isDigit();
}

void ImgProcessTool::PreProcessOCRRes(std::vector<ocrInfo> &ocrRes) {
    std::vector<ocrInfo> res;

    // 定义前两个字符的替换规则
    const QMap<QChar, QChar> firstTwoRules = {
        {'V', 'W'},{'5', 'S'},{'8', 'B'},{'0', 'Q'},{'4','A'}
    };
    // 定义中间字符的替换规则
    const QMap<QChar, QChar> middleRules = {
        {'S', '5'}, {'I', '1'}, {'O', '0'},{'C', '0'},
        {'Q', '0'}, {'Z', '2'}, {'B', '8'},{'A', '4'},
        {'D', '0'}, {'L', '1'}, {'J', '1'}
    };

    for (auto &ocrRe: ocrRes) {

        if (ocrRe.word.length() >= 7)
            ocrRe.word.truncate(7);

        if (ocrRe.isValid) {
            res.emplace_back(ocrRe);
            continue;
        }

        ocrRe.word = ocrRe.word.toUpper();
        // 处理前两个字符
        if (ocrRe.word.length() >= 2) {
            for (int i = 0; i < 2; ++i) {
                QChar c = ocrRe.word[i];
                if (firstTwoRules.contains(c)) {
                    ocrRe.word[i] = firstTwoRules[c];
                }
            }
        }
        // 处理第3位到第7位
        int endIdx = std::min(7, ocrRe.word.length());
        for (int i = 2; i < endIdx; ++i) {
            QChar c = ocrRe.word[i];
            if (middleRules.contains(c)) {
                ocrRe.word[i] = middleRules[c];
            }
        }
        // QLOG_INFO() <<"ocr-post:"<<ocrRe.word;
        if (!ocrRe.isValid)
            ocrRe.isValid = ImgProcessTool::isFormatValid(ocrRe.word);
        res.emplace_back(ocrRe);
    }
    // QLOG_INFO() << "------------------------";
    std::swap(res, ocrRes);
}

ocrInfo ImgProcessTool::FilterOCRResult(const ocrInfo &inputOcrRes) {
    ocrInfo res;

    if (isFormatValid(inputOcrRes.word)) {
        res = inputOcrRes;
        res.word.truncate(7);
        res.word = res.word.toUpper();
    }

    return res;
    //display OCR word
    // for (const auto &res: ocrRes)
    //     QLOG_INFO() << "ocr：" << res.word;
}

std::vector<ocrInfo> ImgProcessTool::FilterOCRResult(const std::vector<ocrInfo> &ocrRes) {
    std::vector<ocrInfo> res;
    for (auto &ocrRe: ocrRes)
        if (ocrRe.isValid)
            res.emplace_back(ocrRe);

    return res;
}

const HTuple ImgProcessTool::FindDevice(const QString &deviceType) {
    HTuple hv_DLDeviceHandles, hv_Length, hv_typeStr, deviceHandle;

    QueryAvailableDlDevices((HTuple("runtime").Append("runtime")), (HTuple("cpu").Append("gpu")),
                            &hv_DLDeviceHandles);
    TupleLength(hv_DLDeviceHandles, &hv_Length);

    for (int i = 0; i < hv_Length.I(); ++i) {
        GetHandleTuple(HTuple(hv_DLDeviceHandles[i]), "type", &hv_typeStr);
        if (0 != (int(hv_typeStr == HTuple(deviceType.toLower().toStdString().data()))))
            deviceHandle = HTuple(hv_DLDeviceHandles[i]);
    }
    return deviceHandle;
}
