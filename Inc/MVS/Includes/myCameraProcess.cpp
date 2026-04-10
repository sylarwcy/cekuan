#include "myCameraGigE.h"
#include "MyApplication.h"
#include "appconfig.h"

#include <process.h>
#include <QString>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

//////////////////////////////////////////////////////////////////////////////
void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
                  HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box) {
    HTuple hv_GenParamName, hv_GenParamValue;

    //Convert the parameters for disp_text.
    if (0 != (HTuple(int(hv_Row == HTuple())).TupleOr(int(hv_Column == HTuple())))) {
        return;
    }
    if (0 != (int(hv_Row == -1))) {
        hv_Row = 12;
    }
    if (0 != (int(hv_Column == -1))) {
        hv_Column = 12;
    }

    //Convert the parameter Box to generic parameters.
    hv_GenParamName = HTuple();
    hv_GenParamValue = HTuple();
    if (0 != (int((hv_Box.TupleLength()) > 0))) {
        if (0 != (int(HTuple(hv_Box[0]) == HTuple("false")))) {
            //Display no box
            hv_GenParamName = hv_GenParamName.TupleConcat("box");
            hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
        } else if (0 != (int(HTuple(hv_Box[0]) != HTuple("true")))) {
            //Set a color other than the default.
            hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
            hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
        }
    }
    if (0 != (int((hv_Box.TupleLength()) > 1))) {
        if (0 != (int(HTuple(hv_Box[1]) == HTuple("false")))) {
            //Display no shadow.
            hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
            hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
        } else if (0 != (int(HTuple(hv_Box[1]) != HTuple("true")))) {
            //Set a shadow color other than the default.
            hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
            hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
        }
    }
    //Restore default CoordSystem behavior.
    if (0 != (int(hv_CoordSystem != HTuple("window")))) {
        hv_CoordSystem = "image";
    }

    if (0 != (int(hv_Color == HTuple("")))) {
        //disp_text does not accept an empty string for Color.
        hv_Color = HTuple();
    }

    DispText(hv_WindowHandle, hv_String, hv_CoordSystem, hv_Row, hv_Column, hv_Color, hv_GenParamName,
             hv_GenParamValue);
    return;
}

cv::Mat HImageToMat(HalconCpp::HObject &H_img) {
    cv::Mat cv_img;
    HalconCpp::HTuple channels, w, h;

    HalconCpp::ConvertImageType(H_img, &H_img, "byte");
    HalconCpp::CountChannels(H_img, &channels);

    if (channels.I() == 1) {
        HalconCpp::HTuple pointer;
        GetImagePointer1(H_img, &pointer, nullptr, &w, &h);
        int width = w.I(), height = h.I();
        int size = width * height;
        cv_img = cv::Mat::zeros(height, width, CV_8UC1);
        memcpy(cv_img.data, (void *) (pointer.L()), size);
    } else if (channels.I() == 3) {
        HalconCpp::HTuple pointerR, pointerG, pointerB;
        HalconCpp::GetImagePointer3(H_img, &pointerR, &pointerG, &pointerB, nullptr, &w, &h);
        int width = w.I(), height = h.I();
        int size = width * height;
        cv_img = cv::Mat::zeros(height, width, CV_8UC3);
        uchar *R = (uchar *) (pointerR.L());
        uchar *G = (uchar *) (pointerG.L());
        uchar *B = (uchar *) (pointerB.L());
        for (int i = 0; i < height; ++i) {
            uchar *p = cv_img.ptr<uchar>(i);
            for (int j = 0; j < width; ++j) {
                p[3 * j] = B[i * width + j];
                p[3 * j + 1] = G[i * width + j];
                p[3 * j + 2] = R[i * width + j];
            }
        }
    }
    return cv_img;
}

HalconCpp::HObject MatToHImage(cv::Mat &cv_img) {
    HalconCpp::HObject H_img;

    if (cv_img.channels() == 1) {
        int height = cv_img.rows, width = cv_img.cols;
        int size = height * width;
        uchar *temp = new uchar[size];

        memcpy(temp, cv_img.data, size);
        HalconCpp::GenImage1(&H_img, "byte", width, height, (Hlong) (temp));

        delete[] temp;
    } else if (cv_img.channels() == 3) {
        int height = cv_img.rows, width = cv_img.cols;
        int size = height * width;
        uchar *B = new uchar[size];
        uchar *G = new uchar[size];
        uchar *R = new uchar[size];

        for (int i = 0; i < height; i++) {
            uchar *p = cv_img.ptr<uchar>(i);
            for (int j = 0; j < width; j++) {
                B[i * width + j] = p[3 * j];
                G[i * width + j] = p[3 * j + 1];
                R[i * width + j] = p[3 * j + 2];
            }
        }
        HalconCpp::GenImage3(&H_img, "byte", width, height, (Hlong) (R), (Hlong) (G), (Hlong) (B));

        delete[] R;
        delete[] G;
        delete[] B;
    }
    return H_img;
}

////////////////////////////////////////////////////////////////////////////////////
int MyCameraGigE::GrabThreadProcess(void) {
    MyApplication *pApp = (MyApplication *) qApp;

    QString str;
    QTime cycle_time;

    MV_FRAME_OUT stImageInfo = {0};
    // MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};
    int nRet = MV_OK;

    HObject ho_image, ho_imageCrop, ho_imageScale, ho_ImageScaleMax;
    HTuple hv_height, hv_width;
    int nHeight, nWidth;
    int trans_width, trans_height;

    bool isOK;
    int scale_rtn_code;
    int find_rtn_code;

    while (m_bThreadState) {
        cycle_time.start();

        //非测试模式处理
        if (pApp->pNodeData->setting.test_mode == 0)
            nRet = m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
        else
            nRet = MV_OK;

        if (nRet == MV_OK) {
            //非测试模式
            if (pApp->pNodeData->setting.test_mode == 0) {
                //用于保存图片
                EnterCriticalSection(&m_hSaveImageMux);
                if (NULL == m_pSaveImageBuf || stImageInfo.stFrameInfo.nFrameLen > m_nSaveImageBufSize) {
                    if (m_pSaveImageBuf) {
                        free(m_pSaveImageBuf);
                        m_pSaveImageBuf = NULL;
                    }

                    m_pSaveImageBuf = (unsigned char *) malloc(
                        sizeof(unsigned char) * stImageInfo.stFrameInfo.nFrameLen);
                    if (m_pSaveImageBuf == NULL) {
                        LeaveCriticalSection(&m_hSaveImageMux);
                        return 0;
                    }
                    m_nSaveImageBufSize = stImageInfo.stFrameInfo.nFrameLen;
                }
                memcpy(m_pSaveImageBuf, stImageInfo.pBufAddr, stImageInfo.stFrameInfo.nFrameLen);
                memcpy(&m_stImageInfo, &(stImageInfo.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
                LeaveCriticalSection(&m_hSaveImageMux);
            }

            // stDisplayInfo.hWnd = m_hwndDisplay;
            // stDisplayInfo.pData = stImageInfo.pBufAddr;
            // stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
            // stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
            // stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
            // stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;

            //转换为halcon格式,返回ho_image,nWidth,nHeight
            ImgToHObject(stImageInfo, ho_image, nWidth, nHeight);

            //重新获取图像尺寸
            GetImageSize(ho_image, &hv_width, &hv_height);
            nWidth = hv_width[0];
            nHeight = hv_height[0];


            //显示原始图像
            if (!HtupleIsEmpty(winHandle_ori)) {
                if (HDevWindowStack::IsOpen()) {
                    SetPart(winHandle_ori, 0, 0, nHeight, nWidth);
                    DispObj(ho_image, winHandle_ori);
                }
            }
            //仿射变换,裁剪后返回
            isOK = ProjectiveTrans(ho_image, nWidth, nHeight, ho_imageCrop, trans_width, trans_height);
            if (!isOK) {
                //非测试模式
                if (pApp->pNodeData->setting.test_mode == 0)
                    m_pcMyCamera->FreeImageBuffer(&stImageInfo);

                QLOG_INFO() << u8"函数 ProjectiveTrans 执行失败！";

                continue;
            }

            //机前、机后图像处理
            processFrontImage(ho_imageCrop, trans_width, trans_height);

            //非测试模式
            if (pApp->pNodeData->setting.test_mode == 0)
                m_pcMyCamera->FreeImageBuffer(&stImageInfo); //释放本次缓存
        } else {
            if (MV_TRIGGER_MODE_ON == m_nTriggerMode)
                Sleep(5);

            QLOG_ERROR() << QString("m_pcMyCamera->GetImageBuffer函数发生错误,等待中...");
        }

        //等待时间
        timeBeginPeriod(1);
        Sleep(pApp->pNodeData->setting.sample_period);
        timeEndPeriod(1);

        //总循环时间计算
        thread_cycle_ms = cycle_time.elapsed();
    }
    return MV_OK;
}

//转换为halcon格式，送给相机处理程序
void MyCameraGigE::ImgToHObject(MV_FRAME_OUT &stImageInfo, HObject &ho_image, int &nWidth, int &nHeight) {
    MyApplication *pApp = (MyApplication *) qApp;

    HBYTE *pData = (HBYTE *) stImageInfo.pBufAddr;

    //非测试模式
    if (pApp->pNodeData->setting.test_mode == 0) {
        nHeight = stImageInfo.stFrameInfo.nHeight;
        nWidth = stImageInfo.stFrameInfo.nWidth;

        //转换为halcon格式
        GenImage1(&ho_image, "byte", nWidth, nHeight, (Hlong) pData);
    } else {

        nWidth = pApp->pNodeData->setting.front_camera_res_width;
        nHeight = pApp->pNodeData->setting.front_camera_res_height;
        //产生全黑图像
        GenImageConst(&ho_image, "byte", nWidth, nHeight);
    }

    //如果获取尺寸与设定文件不同，改变
    if (nWidth != pApp->pNodeData->setting.front_camera_res_width || nHeight != pApp->pNodeData->setting.
        front_camera_res_height)
        ZoomImageSize(ho_image, &ho_image, pApp->pNodeData->setting.front_camera_res_width,
                      pApp->pNodeData->setting.front_camera_res_height, "constant");
    else if (nWidth != pApp->pNodeData->setting.back_camera_res_width || nHeight != pApp->pNodeData->setting.
             back_camera_res_height)
        ZoomImageSize(ho_image, &ho_image, pApp->pNodeData->setting.back_camera_res_width,
                      pApp->pNodeData->setting.back_camera_res_height, "constant");

    //测试加载图片
    // if (pApp->isTestImage) {
    //     QString strName;
    //
    //     //读测试图片
    //     HTuple hv_Width, hv_Height;
    //     strName = QCoreApplication::applicationDirPath() + "/Image_test/image_test_front.jpg";
    //
    //     ReadImage(&ho_image, strName.toStdString().c_str());
    //     Rgb1ToGray(ho_image, &ho_image);
    //     GetImageSize(ho_image, &hv_Width, &hv_Height);
    //     nWidth = hv_Width[0];
    //     nHeight = hv_Height[0];
    // }

    //测试加载视频
    // if (pApp->isTestVideo) {
    //     HTuple hv_Width, hv_Height;
    //
    //     if (frame_num_front > pApp->frame_end_front - 1)
    //         frame_num_front = 0;
    //
    //     SetFramegrabberParam(pApp->hv_AcqHandle_front, "frame_number", frame_num_front);
    //     GrabImageAsync(&ho_image, pApp->hv_AcqHandle_front, -1);
    //     Rgb1ToGray(ho_image, &ho_image);
    //     frame_num_front++;
    //
    //     GetImageSize(ho_image, &hv_Width, &hv_Height);
    //     nWidth = hv_Width[0];
    //     nHeight = hv_Height[0];
    // } else {
        frame_num_front = 0;
        frame_num_back = 0;
    // }
}

//仿射变换
bool MyCameraGigE::ProjectiveTrans(HObject &ho_image, int width, int height, HObject &ho_imageCrop, int &trans_width,
                                   int &trans_height) {
    MyApplication *pApp = (MyApplication *) qApp;

    HObject ho_imageProj;
    HTuple hv_Width, hv_Height;
    HTuple hv_HomMat2D, hv_ErrorCode, hv_Exception;

    try {
        hv_Width = width;
        hv_Height = height;

        //投影变换矩阵

        HomVectorToProjHomMat2d(pApp->f_hv_x0, pApp->f_hv_y0, (((HTuple(1).Append(1)).Append(1)).Append(1)),
                                pApp->f_hv_x1, pApp->f_hv_y1, (((HTuple(1).Append(1)).Append(1)).Append(1)),
                                "normalized_dlt",
                                &hv_HomMat2D);

        ProjectiveTransImage(ho_image, &ho_imageProj, hv_HomMat2D, "nearest_neighbor",
                             "true", "false");

        //裁剪返回
        trans_width = AppConfig::f_crop_image_width_mm / AppConfig::f_each_pixel_equal_mm_width;
        trans_height = AppConfig::f_crop_image_height_mm / AppConfig::f_each_pixel_equal_mm_height;

        CropPart(ho_imageProj, &ho_imageCrop, 0, 0, trans_width, trans_height);
    } catch (HException &HDevExpDefaultException) {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        hv_ErrorCode = ((const HTuple &) hv_Exception)[0];

        QLOG_ERROR() << QString(u8"ProjectiveTrans处理图像发生错误,机前=%1,错误代码=%2").arg(isAtFront).arg(hv_ErrorCode[0].I());
        return false;
    }

    return true;
}

void MyCameraGigE::SaveImgToDisk(HObject &ho_image, QString ID, QString strFlag) {
    MyApplication *pApp = (MyApplication *) qApp;

    static unsigned int saveProcessNum = 0;
    static QString processSavePath;
    static int last_track_size = 0;

    bool isOK;

    //获取时间
    QDateTime time = QDateTime::currentDateTime();
    int year = time.date().year();
    int month = time.date().month();
    int day = time.date().day();
    int hour = time.time().hour();
    int min = time.time().minute();
    int sec = time.time().second();
    int msec = time.time().msec();

    QString save_path = pApp->pNodeData->setting.save_path;
    QString saveIDPath = QString("%1/%2_%3_%4/%5").arg(save_path).arg(year).arg(month).arg(day).arg(ID);
    QString fileName = QString("%1_%2-%3_%4_%5_%6_%7_%8.jpg")
            .arg(ID)
            .arg(strFlag)
            .arg(year)
            .arg(month)
            .arg(day)
            .arg(hour)
            .arg(min)
            .arg(sec);
    QString pathName = saveIDPath + "/" + fileName;

    //不存在就创建
    isOK = DirExistEx(saveIDPath);
    if (isOK)
        WriteImage(ho_image, "jpeg", 0, pathName.toStdString().c_str());
}

bool MyCameraGigE::SaveMp4Name(unsigned short ID, int width, int height) {
    MyApplication *pApp = (MyApplication *) qApp;

    QString processSavePath;
    bool isOK;

    //获取时间
    QDateTime time = QDateTime::currentDateTime();
    int year = time.date().year();
    int month = time.date().month();
    int day = time.date().day();
    int hour = time.time().hour();
    int min = time.time().minute();
    int sec = time.time().second();
    int msec = time.time().msec();

    QString save_path = pApp->pNodeData->setting.save_path;
    QString path1 = save_path + "/movie/";
    QString path2 = QString("%1_%2_%3")
            .arg(year)
            .arg(month)
            .arg(day);
    processSavePath = path1 + path2;

    isOK = DirExistEx(processSavePath);
    if (isOK) {
        QString name = QString("%1/%2-%3-%4-%5-%6-%7_%8_video.avi")
                .arg(processSavePath)
                .arg(year)
                .arg(month)
                .arg(day)
                .arg(hour)
                .arg(min)
                .arg(sec)
                .arg(ID);
        writer.open(name.toStdString().c_str(), VideoWriter::fourcc('M', 'J', 'P', 'G'), 25, Size(width, height),
                    false);
        return true;
    }

    return false;
}


//单级文件夹是否存储，不存在则创建
bool MyCameraGigE::DirExist(QString fullPath) {
    QDir dir(fullPath);
    if (dir.exists()) {
        //存在当前文件夹
        return true;
    } else {
        //不存在则创建
        bool ok = dir.mkdir(fullPath); //只创建一级子目录，即必须保证上级目录存在
        return ok;
    }
}

//文件夹是否存储，不存在则创建，支持多级文件夹
bool MyCameraGigE::DirExistEx(QString fullPath) {
    QDir dir(fullPath);
    if (dir.exists()) {
        return true;
    } else {
        //不存在当前目录，创建，可创建多级目录
        bool ok = dir.mkpath(fullPath);
        return ok;
    }
}


void MyCameraGigE::processFrontImage(HObject &ho_image, int width, int height) {
    MyApplication *pApp = (MyApplication *) qApp;
    NodeData *pNodeData = pApp->pNodeData;

    HTuple hv_Width, hv_Height;
    HTuple hv_Min, hv_Max, hv_Range, hv_scale_coff;
    HTuple hv_Min1, hv_Max1;

    HTuple hv_i, hv_j;
    HTuple hv_row_start, hv_row_end, hv_col_start, hv_col_end;
    HTuple hv_MetrologyHandle, hv_Index, hv_Row, hv_Column;
    HTuple hv_row_l, hv_col_l;
    HTuple hv_row_r, hv_col_r;
    HTuple hv_idx_l, hv_idx_r, hv_row_track;
    HTuple hv_mid_row, hv_mid_col;

    HObject ho_ImageScaled;
    HObject ho_Rect_corp, ho_Rect1;
    HObject ho_Contours, ho_Cross, ho_Cross_l, ho_Cross_r;
    HObject ho_mid_contour;

    try {
        hv_Width = width;
        hv_Height = height;

        //判断是否有钢板，如果有先第一次缩放
        GenRectangle1(&ho_Rect1, 10, hv_Width / 2, hv_Height, hv_Width / 2);
        MinMaxGray(ho_Rect1, ho_image, 0, &hv_Min, &hv_Max, &hv_Range);
        //有钢板
        if (hv_Max > 100) {
            ScaleImageMax(ho_image, &ho_ImageScaled);

            //边缘增强
            Emphasize(ho_ImageScaled, &ho_ImageScaled, width, height, 1.0);

            //自己图像处理
        } else {
            CopyImage(ho_image, &ho_ImageScaled);
        }

        //显示图像
        if (!HtupleIsEmpty(winHandle_pro)) {
            HDevWindowStack::SetActive(winHandle_pro);
            if (HDevWindowStack::IsOpen()) {
                //解决刷新问题
                SetSystem("flush_graphic", "false");

                SetPart(winHandle_pro, 0, 0, height, width);
                DispObj(ho_ImageScaled, winHandle_pro);

                SetSystem("flush_graphic", "true");
            }
        }
    } catch (HException &except) {
        HString str;
        str = except.ErrorMessage();
        QLOG_ERROR() << "front->ErrorMessage:" << str.ToUtf8() << ",ErrorCode:" << except.ErrorCode();
    }
}

void MyCameraGigE::processBackImage(HObject &ho_image, int width, int height) {
    MyApplication *pApp = (MyApplication *) qApp;
    NodeData *pNodeData = pApp->pNodeData;

    HTuple hv_Width, hv_Height;
    HTuple hv_Min, hv_Max, hv_Range, hv_scale_coff;
    HTuple hv_Min1, hv_Max1;

    HTuple hv_i, hv_j;
    HTuple hv_row_start, hv_row_end, hv_col_start, hv_col_end;
    HTuple hv_MetrologyHandle, hv_Index, hv_Row, hv_Column;
    HTuple hv_row_l, hv_col_l;
    HTuple hv_row_r, hv_col_r;
    HTuple hv_idx_l, hv_idx_r, hv_row_track;
    HTuple hv_mid_row, hv_mid_col;

    HObject ho_ImageScaled;
    HObject ho_Rect_corp, ho_Rect1;
    HObject ho_Contours, ho_Cross, ho_Cross_l, ho_Cross_r;
    HObject ho_mid_contour;

    //图像处理，自行编写算法
    try {
        hv_Width = width;
        hv_Height = height;

        //判断是否有钢板，如果有先第一次缩放
        GenRectangle1(&ho_Rect1, 0, hv_Width / 2, hv_Height, hv_Width / 2);
        MinMaxGray(ho_Rect1, ho_image, 0, &hv_Min, &hv_Max, &hv_Range);
        //有钢板
        if (hv_Max > 100) {
            ScaleImageMax(ho_image, &ho_ImageScaled);

            //边缘增强
            Emphasize(ho_ImageScaled, &ho_ImageScaled, width, height, 1.0);
        } else {
            CopyImage(ho_image, &ho_ImageScaled);
        }

        //显示图像
        if (!HtupleIsEmpty(winHandle_pro)) {
            HDevWindowStack::SetActive(winHandle_pro);
            if (HDevWindowStack::IsOpen()) {
                //解决刷新问题
                SetSystem("flush_graphic", "false");

                SetPart(winHandle_pro, 0, 0, height, width);
                DispObj(ho_ImageScaled, winHandle_pro);

                SetSystem("flush_graphic", "true");
            }
        }
    } catch (HException &except) {
        HString str;
        str = except.ErrorMessage();
        QLOG_ERROR() << "back->ErrorMessage:" << str.ToUtf8() << ",ErrorCode:" << except.ErrorCode();
    }
}
