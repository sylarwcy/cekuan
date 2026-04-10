//
// Created by wcy on 25-8-1.
//


#include "HalconPreprocessing.h"

int HalconPreprocessing::affineHeight = 899;
int HalconPreprocessing::affineWidth = 2000;

void HalconPreprocessing::AffineInit(const HTuple &hv_x_pre, const HTuple &hv_y_pre,
                                     const HTuple &hv_x_post, const HTuple &hv_y_post) {
    HomVectorToProjHomMat2d(hv_x_pre, hv_y_pre, (((HTuple(1).Append(1)).Append(1)).Append(1)),
                            hv_x_post, hv_y_post, (((HTuple(1).Append(1)).Append(1)).Append(1)), "normalized_dlt",
                            &hv_HomMat2D);

    // HObject img1, img2, img3;
    // GenImageConst(&img1, "byte", 2000, 1500);
    // GenImageConst(&img2, "byte", 2000, 1500);
    // GenImageConst(&img3, "byte", 2000, 1500);
    // Compose3(img1, img2, img3, &m_bgImg);

    // GenImageConst(&m_bgImg, "byte", 2000, 1500);
}

void HalconPreprocessing::Affine(const HObject &img, HObject &ho_TransImage) {
    HObject ho_ImageReduced, ho_ImagePart, ho_Background, ho_cropImg;

    // Rgb1ToGray(img,&ho_GrayImage);
    ProjectiveTransImage(img, &ho_TransImage, hv_HomMat2D, "bilinear", "false",
                         "false");
    // HObject copy_img;
    Rectangle1Domain(ho_TransImage, &ho_ImageReduced, 0, 0, affineHeight, affineWidth);
    CropDomain(ho_ImageReduced, &ho_TransImage);


    // CopyImage(m_bgImg,&copy_img);
    // OverpaintGray(copy_img, ho_TransImage);
    // ho_TransImage = copy_img;
    // Rectangle1Domain(ho_TransImage, &ho_ImageReduced, 0, 0, affineHeight, affineWidth);
    // CropDomain(ho_ImageReduced, &ho_cropImg);
    // PaintGray (ho_cropImg,m_bgImg, &ho_TransImage);
}

HTuple HalconPreprocessing::GetSuitableInferDevice() {
    HTuple hv_DLDeviceHandles, hv_Length, hv_Type, hv_DeviceName;
    QueryAvailableDlDevices((HTuple("runtime").Append("runtime")), (HTuple("cpu").Append("gpu")),
                            &hv_DLDeviceHandles);
    TupleLength(hv_DLDeviceHandles, &hv_Length);
    int deviceCount = hv_Length.I();
    if (deviceCount > 0) {
        for (int i = 0; i < deviceCount; ++i) {
            GetHandleTuple(HTuple(hv_DLDeviceHandles[i]), "type", &hv_Type);
            GetHandleTuple(HTuple(hv_DLDeviceHandles[i]), "name", &hv_DeviceName);
            std::cout << "device[" << i << "]: [" << hv_Type.S() << "]" << hv_DeviceName.S() << std::endl;
        }
    }
    return hv_DLDeviceHandles;
}
