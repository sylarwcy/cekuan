//
// Created by wcy on 25-8-1.
//

#include "Det/HalconDet.h"
#include "Qslog.h"

void HalconDet::Init(std::string modelPath, std::string preprocessParamPath, const HTuple &hv_DLDeviceHandle) {
    ReadDict(preprocessParamPath.c_str(), HTuple(), HTuple(), &hv_PreprocessParam);

    hv_imgWidth = hv_PreprocessParam.TupleGetDictTuple("image_width"); //512
    hv_imgHeight = hv_PreprocessParam.TupleGetDictTuple("image_height"); //320

    ReadDlModel(modelPath.c_str(), &hv_DLModelHandle);
    SetDlModelParam(hv_DLModelHandle, "batch_size", 1);
    SetDlModelParam(hv_DLModelHandle, "device", hv_DLDeviceHandle);
}

DetRes HalconDet::Infer(const HObject &img) {
    // HTuple hv_value;
    // GetDlModelParam(hv_DLModelHandle,"device",&hv_value);
    // QString device_type = QString::fromUtf8(hv_value.S().Text());
    // QLOG_INFO() <<"det-device：" <<device_type;

    HTuple hv_DLSampleInference, hv_DLResult;

    gen_dl_samples_from_images(img, &hv_DLSampleInference);
    preprocess_dl_samples(hv_DLSampleInference, hv_PreprocessParam);
    ApplyDlModel(hv_DLModelHandle, hv_DLSampleInference, HTuple(), &hv_DLResult);

    int objectNum = hv_DLResult.TupleGetDictTuple("bbox_row1").Length();
    float ratio_x = 2000 / 512.0;
    float ratio_y = 900/320.0;
    // float ratio_y = 1500 / 320.0;
    //判断是否检测到目标
    DetRes res;
    if (objectNum > 0) {
        for (auto index = 0; index < objectNum; ++index) {
            // HTuple hv_bbox_row1 = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_row1");
            // HTuple hv_bbox_col1 = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_col1");
            // HTuple hv_bbox_row2 = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_row2");
            // HTuple hv_bbox_col2 = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_col2");
            // HTuple hv_bbox_class_id = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_class_id");
            // HTuple hv_bbox_class_name = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_class_name");
            // HTuple hv_bbox_confidence = HTuple(hv_DLResult[index]).TupleGetDictTuple("bbox_confidence");
            HTuple hv_bbox_row1 = hv_DLResult.TupleGetDictTuple("bbox_row1")[index];
            HTuple hv_bbox_col1 = hv_DLResult.TupleGetDictTuple("bbox_col1")[index];
            HTuple hv_bbox_row2 = hv_DLResult.TupleGetDictTuple("bbox_row2")[index];
            HTuple hv_bbox_col2 = hv_DLResult.TupleGetDictTuple("bbox_col2")[index];
            HTuple hv_bbox_class_id = hv_DLResult.TupleGetDictTuple("bbox_class_id")[index];
            HTuple hv_bbox_class_name = hv_DLResult.TupleGetDictTuple("bbox_class_name")[index];
            HTuple hv_bbox_confidence = hv_DLResult.TupleGetDictTuple("bbox_confidence")[index];
            // QLOG_INFO() << "conf:" << hv_bbox_confidence.D();
            if (hv_bbox_confidence.D() < 0.95)
                continue;
            cv::Rect_<int> tmp_box = cv::Rect(hv_bbox_col1.D() * ratio_x, hv_bbox_row1.D() * ratio_y,
                                              (hv_bbox_col2.D() - hv_bbox_col1.D()) * ratio_x,
                                              (hv_bbox_row2.D() - hv_bbox_row1.D()) * ratio_y);

            res.addDetection(tmp_box, hv_bbox_confidence.D(), hv_bbox_class_name.S().Text());
        }
    }
    // QLOG_INFO() << "conf:*************************************";
    return res;
}
