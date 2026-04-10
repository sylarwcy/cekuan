//
// Created by wcy on 25-8-1.
//

#include "OCR/HalconOCR.h"
#include "ImgProcessTool.h"
#include "Qslog.h"

void HalconOCR::Init(std::string det1ModelPath, std::string det2ModelPath, std::string rec2ModelPath,
                     const HTuple &hv_DLDeviceHandles) {
    this->det1ModelPath = det1ModelPath;
    this->det2ModelPath = det2ModelPath;
    this->rec2ModelPath = rec2ModelPath;
    SetOCRModelParam(hv_DLDeviceHandles); //初始化OCR模型参数
}

void HalconOCR::SetOCRModelParam(const HTuple &hv_DLDeviceHandles) {
    CreateDeepOcr(HTuple(), HTuple(), &hv_DeepOcrHandle);
    SetDeepOcrParam(hv_DeepOcrHandle, "detection_model", det1ModelPath.c_str());
    // SetDeepOcrParam(hv_DeepOcrHandle, "detection_batch_size", 1);
    SetDeepOcrParam(hv_DeepOcrHandle, "device", hv_DLDeviceHandles);

    CreateDeepOcr(HTuple(), HTuple(), &hv_DeepOcrHandle_crop);
    SetDeepOcrParam(hv_DeepOcrHandle_crop, "recognition_model", rec2ModelPath.c_str());
    SetDeepOcrParam(hv_DeepOcrHandle_crop, "detection_model", det2ModelPath.c_str());
    // SetDeepOcrParam(hv_DeepOcrHandle_crop, "recognition_batch_size", 1);
    SetDeepOcrParam(hv_DeepOcrHandle_crop, "device", hv_DLDeviceHandles);
}

void HalconOCR::RenderBox(const HObject &img, HObject *renderImg, std::vector<ocrInfo> &res) {
    HObject rectangle;
    CopyImage(img, renderImg);
    for (int i = 0; i < res.size(); i++) {
        GenRectangle1(&rectangle,
                      res[i].GetRectSrc().y,
                      res[i].GetRectSrc().x,
                      res[i].GetRectSrc().y + res[i].GetRectSrc().height,
                      res[i].GetRectSrc().x + res[i].GetRectSrc().width);
        PaintRegion(rectangle, *renderImg, renderImg, ((HTuple(255).Append(0)).Append(0)), "margin");
    }
}

std::vector<ocrInfo> HalconOCR::Infer(const HObject &img, const QString &type) {
    // QElapsedTimer cycle_time;
    // cycle_time.start();

    HTuple hv_DeepOcrResult;
    ApplyDeepOcr(img, hv_DeepOcrHandle, "detection", &hv_DeepOcrResult);
    auto res = ParseOCRResults(hv_DeepOcrResult);

    // static int test_index=0;
    // HObject renderImg;
    // if (!res.empty()) {
    //     // QLOG_INFO() << "字符目标数量："<<res.size();
    //     RenderBox(img, &renderImg, res);
    //     std::string path,srcPath;
    //     if(type == "StackerTable") {
    //         path = "E:/test/stackerTable/render/"+std::to_string(test_index)+".jpg";
    //         srcPath = "E:/test/stackerTable/src/"+std::to_string(test_index++)+".jpg";
    //     }else if (type == "HotCharging") {
    //         path = "E:/test/hotCharging/render/"+std::to_string(test_index)+".jpg";
    //         srcPath = "E:/test/hotCharging/src/"+std::to_string(test_index++)+".jpg";
    //     }
    //     WriteImage(img, "jpeg", 0, srcPath.c_str());
    //     WriteImage(renderImg, "jpeg", 0, path.c_str());
    // }

    if (type == "StackerTable")
        Infer_StackerTable(img, res);
    if (type == "HotCharging")
        Infer_HotCharging(img, res);

    // QLOG_INFO() << type << ",ocr-耗时：" << cycle_time.elapsed() << " ms";

    return res;
}

void HalconOCR::Infer_StackerTable(const HObject &img, std::vector<ocrInfo> &res) {
    // ImgProcessTool::FilterOCRResult(res);
    // QLOG_INFO() << "进入垛板台ocr识别函数";
    if (res.empty()) return;
    // QLOG_INFO() << "垛板台ocr目标不为空";
    // qDebug() <<"1";
    // 1. 删除不满足条件的元素
    // for (auto re:res) {
    //     QLOG_INFO() << "length1:"<<re.length1<<",length2:"<<re.length2<<",point.y:"<<re.srcImgPoint.y;
    //     QLOG_INFO() << "----------------------------------";
    // }
    auto newEnd = std::remove_if(res.begin(), res.end(),
                                 [](const ocrInfo &info) {
                                     // 删除条件：point.y < 250 或 length1 < 75
                                     //y小于250是垛板台位置辊道上的板号，length1小于75的字符有可能是[定尺]
                                     return (info.srcImgPoint.y + info.length2) < 250 || info.length1 < 85;
                                 });
    res.erase(newEnd, res.end());
    if (res.size() > 1) {
        std::sort(res.begin(), res.end(),
                  [](const ocrInfo &a, const ocrInfo &b) {
                      return a.srcImgPoint.y < b.srcImgPoint.y;
                  });
    }
    // QLOG_INFO() << "垛板台ocr目标数量："<<res.size();
    HTuple hv_DeepOcrResultCrop;
    std::vector<ocrInfo> tmpVec;
    // std::vector<QString> strVec;
    // std::vector<ocrInfo> errorVec;

    // for (int i = 0; i < res.size() && i < 2; i++) {
    //在排序后的字符中，循环取字符，如果取到目标格式的字符，直接break，否则再取一个。
    //当板号、钢种、定尺这三样分开喷时，如果钢板发生倾斜，钢种会在
    // if (!tmpVec.empty()) break;
    if (res.empty()) return;

    ocrInfo tmp;
    tmp.srcImgPoint = res[0].srcImgPoint;
    cv::Point point = tmp.srcImgPoint;

    // if (point.x - 256 < 0 || point.x + 256 > 2000 || point.y - 192 < 0 || point.y + 192 > 900) {
    // if (point.x - 150 < 0 || point.x + 150 > 2000) {
    // QLOG_INFO()<<"ocr位置不满足裁剪条件，跳过";
    // continue;
    // }

    HTuple hv_CropRow1, hv_CropCol1, hv_CropRow2, hv_CropCol2;
    HTuple hv_HomMat2DIdentity, hv_HomMat2DTranslate;
    HObject ho_ImageAffineTrans, ImagePart;
    hv_CropRow1 = point.y - res[0].length2 - 150;
    hv_CropCol1 = point.x - res[0].length1 - 131;
    hv_CropRow2 = point.y - res[0].length2 + 233;
    hv_CropCol2 = point.x - res[0].length1 + 380;
    HomMat2dIdentity(&hv_HomMat2DIdentity);
    HomMat2dTranslate(hv_HomMat2DIdentity, -hv_CropRow1, -hv_CropCol1, &hv_HomMat2DTranslate);
    AffineTransImageSize(img, &ho_ImageAffineTrans, hv_HomMat2DTranslate, "constant",
                         512, 384);
    FullDomain(ho_ImageAffineTrans, &ImagePart);

    // GenRectangle1(&Rectangle, point.y - 192, point.x - 256, point.y + 191,
    //               point.x + 255);
    // ReduceDomain(img, Rectangle, &ho_ImageReduced);
    // CropDomain(ho_ImageReduced, &ImagePart);
    CopyImage(ImagePart, &tmp.img);

    HTuple hv_Words, hv_Words_word;
    ApplyDeepOcr(ImagePart, hv_DeepOcrHandle_crop, "auto", &hv_DeepOcrResultCrop);
    GetDictTuple(hv_DeepOcrResultCrop, "words", &hv_Words);
    GetDictTuple(hv_Words, "word", &hv_Words_word);
    int length = hv_Words_word.TupleLength();
    if (length > 0) {
        tmp.word = QString::fromUtf8(hv_Words_word[0].S().Text());
        // QLOG_INFO() << "垛板台ocr识别结果：" << tmp.word;
        HTuple hv_Words_col, hv_Words_row, hv_char_candidates, hv_conf_value, hv_conf_single_value;

        // QLOG_INFO() << "ocr"<<tmp.word<<" ,length :" << length;
        // QLOG_INFO() << "*start*";
        GetDictTuple(hv_Words, "char_candidates", &hv_char_candidates);
        // QLOG_INFO() << "*1*";
        GetDictTuple(hv_char_candidates, 0, &hv_conf_value);
        // QLOG_INFO() << "*2*";
        for (int index = 0; index < tmp.word.length() && index < 7; ++index) {
            // QLOG_INFO() << "*3*";
            GetDictTuple(HTuple(hv_conf_value[index]), "confidence", &hv_conf_single_value);
            // QLOG_INFO() << "*4*";
            if (hv_conf_single_value[0].D() < tmp.minConf)
                tmp.minConf = hv_conf_single_value[0].D();
            // QLOG_INFO() << "*5*";
        }
        // QLOG_INFO() << "*over*";
        GetDictTuple(hv_Words, "col", &hv_Words_col);
        GetDictTuple(hv_Words, "row", &hv_Words_row);
        tmp.point = cv::Point2d(hv_Words_col[0].D(), hv_Words_row[0].D());
        HTuple hv_Words_phi, hv_Words_length1, hv_Words_length2;
        GetDictTuple(hv_Words, "phi", &hv_Words_phi);
        GetDictTuple(hv_Words, "length1", &hv_Words_length1);
        GetDictTuple(hv_Words, "length2", &hv_Words_length2);
        tmp.phi = hv_Words_phi[0].D();
        tmp.length1 = hv_Words_length1[0].D();
        tmp.length2 = hv_Words_length2[0].D();

        tmp.isValid = ImgProcessTool::isFormatValid(tmp.word);
        tmpVec.emplace_back(tmp);
    }
    // }
    std::swap(tmpVec, res);

    // SaveImg(img, strVec,"E:/ocr/StackerTable");
}

void HalconOCR::Infer_HotCharging(const HObject &img, std::vector<ocrInfo> &res) {
    auto newEnd = std::remove_if(res.begin(), res.end(),
                                 [](const ocrInfo &info) {
                                     // 删除条件：length1 < 75
                                     // length1小于75的字符有可能是[定尺]
                                     return info.srcImgPoint.y < 500 || info.srcImgPoint.y > 650 || info.length1 < 85;
                                 });
    res.erase(newEnd, res.end());
    //从左到右排序
    if (res.size() > 1) {
        std::sort(res.begin(), res.end(),
                  [](const ocrInfo &a, const ocrInfo &b) {
                      return a.srcImgPoint.x < b.srcImgPoint.x;
                  });
    }
    // QLOG_INFO() << "热送ocr目标数量(过滤后)："<< QString::number(res.size());
    // HObject ho_ImageReduced, ImagePart, Rectangle;
    HTuple hv_DeepOcrResultCrop;
    std::vector<ocrInfo> tmpVec;
    // std::vector<QString> strVec;
    // std::vector<ocrInfo> errorVec;

    for (int i = 0; i < res.size(); i++) {
        //在排序后的字符中，遍历所有字符

        ocrInfo tmp;
        tmp.srcImgPoint = res[i].srcImgPoint;
        cv::Point point = tmp.srcImgPoint;

        // if (point.x - 256 < 0 || point.x + 256 > 2000 || point.y - 192 < 0 || point.y + 192 > 990) {
        if (point.x - res[i].length1 - 100 < 0 || point.x + res[i].length1 + 100 > 2000) {
            // QLOG_INFO()<<"ocr位置不满足裁剪条件，跳过";
            continue;
        }

        // GenRectangle1(&Rectangle, point.y - 192, point.x - 256, point.y + 191,
        //               point.x + 256);
        // ReduceDomain(img, Rectangle, &ho_ImageReduced);
        // CropDomain(ho_ImageReduced, &ImagePart);
        HTuple hv_CropRow1, hv_CropCol1, hv_CropRow2, hv_CropCol2;
        HTuple hv_HomMat2DIdentity, hv_HomMat2DTranslate;
        HObject ho_ImageAffineTrans, ImagePart;
        hv_CropRow1 = point.y - res[i].length2 - 150;
        hv_CropCol1 = point.x - res[i].length1 - 131;
        hv_CropRow2 = point.y - res[i].length2 + 233;
        hv_CropCol2 = point.x - res[i].length1 + 380;
        HomMat2dIdentity(&hv_HomMat2DIdentity);
        HomMat2dTranslate(hv_HomMat2DIdentity, -hv_CropRow1, -hv_CropCol1, &hv_HomMat2DTranslate);
        AffineTransImageSize(img, &ho_ImageAffineTrans, hv_HomMat2DTranslate, "constant",
                             512, 384);
        FullDomain(ho_ImageAffineTrans, &ImagePart);
        CopyImage(ImagePart, &tmp.img);

        HTuple hv_Words, hv_Words_word;
        ApplyDeepOcr(ImagePart, hv_DeepOcrHandle_crop, "auto", &hv_DeepOcrResultCrop);
        GetDictTuple(hv_DeepOcrResultCrop, "words", &hv_Words);
        GetDictTuple(hv_Words, "word", &hv_Words_word);
        int length = hv_Words_word.TupleLength();
        if (length > 0) {
            tmp.word = QString::fromUtf8(hv_Words_word[0].S().Text());
            // QLOG_INFO() << "热送ocr识别结果：" << tmp.word;
            HTuple hv_Words_col, hv_Words_row, hv_char_candidates, hv_conf_value, hv_conf_single_value;

            // QLOG_INFO() << "ocr:"<<tmp.word<<" ,length :" << length;
            // QLOG_INFO() << "ocr-word:"<<tmp.word.length();
            GetDictTuple(hv_Words, "char_candidates", &hv_char_candidates);
            GetDictTuple(hv_char_candidates, 0, &hv_conf_value);
            for (int index = 0; index < tmp.word.length() && index < 7; ++index) {
                GetDictTuple(HTuple(hv_conf_value[index]), "confidence", &hv_conf_single_value);
                if (hv_conf_single_value[0].D() < tmp.minConf)
                    tmp.minConf = hv_conf_single_value[0].D();
            }

            GetDictTuple(hv_Words, "col", &hv_Words_col);
            GetDictTuple(hv_Words, "row", &hv_Words_row);
            tmp.point = cv::Point2d(hv_Words_col[0].D(), hv_Words_row[0].D());
            HTuple hv_Words_phi, hv_Words_length1, hv_Words_length2;
            GetDictTuple(hv_Words, "phi", &hv_Words_phi);
            GetDictTuple(hv_Words, "length1", &hv_Words_length1);
            GetDictTuple(hv_Words, "length2", &hv_Words_length2);
            tmp.phi = hv_Words_phi[0].D();
            tmp.length1 = hv_Words_length1[0].D();
            tmp.length2 = hv_Words_length2[0].D();

            tmp.isValid = ImgProcessTool::isFormatValid(tmp.word);
            tmpVec.emplace_back(tmp);
        }
    }
    std::swap(tmpVec, res);

    // SaveImg(img, strVec,"E:/ocr/HotCharging");
}

std::vector<ocrInfo> HalconOCR::ParseOCRResults(const HTuple &ocrResult) {
    std::vector<ocrInfo> resVec;

    HTuple hv_Words, hv_Words_colTmp;
    GetDictTuple(ocrResult, "words", &hv_Words);
    GetDictTuple(hv_Words, "col", &hv_Words_colTmp);

    int countString = hv_Words_colTmp.TupleLength();
    for (int i = 0; i < countString; i++) {
        ocrInfo parseRes;

        // HTuple hv_Words_candidates, hv_Words_candidates_0;
        // GetDictTuple(hv_Words, "char_candidates", &hv_Words_candidates);
        // GetDictTuple(hv_Words_candidates, i, &hv_Words_candidates_0);
        // parseRes.charLength = hv_Words_candidates_0.Length();
        // parseRes.word = QString::fromUtf8(hv_Words_word[i].S().Text());
        parseRes.word = "";

        HTuple hv_Words_col, hv_Words_row;
        GetDictTuple(hv_Words, "col", &hv_Words_col);
        GetDictTuple(hv_Words, "row", &hv_Words_row);
        parseRes.srcImgPoint = cv::Point2d(hv_Words_col[i].D(), hv_Words_row[i].D());

        HTuple hv_Words_phi, hv_Words_length1, hv_Words_length2;
        GetDictTuple(hv_Words, "phi", &hv_Words_phi);
        GetDictTuple(hv_Words, "length1", &hv_Words_length1);
        GetDictTuple(hv_Words, "length2", &hv_Words_length2);
        parseRes.phi = hv_Words_phi[i].D();
        parseRes.length1 = hv_Words_length1[i].D();
        parseRes.length2 = hv_Words_length2[i].D();

        //对识别的字符串排序
        // std::vector<double> length1Vec;
        // for (int j = 0; j < parseRes.charLength; ++j) {
        //     HTuple hv_Words_candidates_0_i_char, hv_Words_candidates_0_i_confidence;
        //     GetDictTuple(hv_Words_candidates_0[j], "candidate", &hv_Words_candidates_0_i_char);
        //     GetDictTuple(hv_Words_candidates_0[j], "confidence", &hv_Words_candidates_0_i_confidence);
        //     parseRes.charInfoVec.push_back(std::make_pair(hv_Words_candidates_0_i_char[0].S().Text(),
        //                                                   hv_Words_candidates_0_i_confidence[0].D()));
        // }
        resVec.emplace_back(parseRes);
    }

    return resVec;
}

void HalconOCR::SaveImg(const HObject &image, const std::vector<QString> &ocrRes, const QString &ocrBufferPath) {
    if (ocrRes.empty()) return;

    QString all_str;
    for (const auto &res: ocrRes)
        all_str = QString("%1_%2").arg(all_str).arg(res);
    //获取时间
    QDateTime time = QDateTime::currentDateTime();
    int day = time.date().day();
    int hour = time.time().hour();
    int min = time.time().minute();
    int sec = time.time().second();
    int msec = time.time().msec();

    QString srcPathPrefix = QString("%1/").arg(ocrBufferPath);
    QString fileName_prefix = QString::asprintf("%02d_%02d_%02d_%02d_%03d",
                                                day, hour, min, sec, msec);
    QString fileName = QString("%1_%2.jpg").arg(fileName_prefix).arg(all_str);
    QString srcPathName = srcPathPrefix + fileName;

    WriteImage(image, "jpeg", 0, srcPathName.toStdString().c_str());
}
