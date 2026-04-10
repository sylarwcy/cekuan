// StackerTableImgPro.cpp
//
// Created by sylar on 25-10-24.
//

#include "StackerTableImgPro.h"
#include <QElapsedTimer>
#include <QMap>
#include "Qslog.h"
#include "WorkerCamera.h"
#include "MyApplication.h"

StackerTableImgPro::StackerTableImgPro(QObject *parent)
    : ImageProcessor(parent) {
    className = "StackerTable";

    m_pusherStatus = PAUSED;
    m_lastPusherStatus = PAUSED;
    m_hasStartedRecognition = false;
    m_hasReported = false;

    // 统计相关变量
    m_tableDetectionCount = 0;
    m_totalDetectionFrames = 0;
    // m_recognizingFrameCount = 0;

    // OCR识别相关
    m_ocrStopThreshold = 50; // 当板号底部与推头顶部交叉像素超过50时停止收集

    // 用于记录所有收集到的板号
    m_plateNumberCounter.clear();
    m_plateValidCounter.clear();

    m_retractFrameCount = 0;
    m_skipNextFrames = false;
    m_framesToSkip = 0;
    shouldStopOCR = false;
}

void StackerTableImgPro::init(
    const HTuple &hv_x_pre, const HTuple &hv_y_pre,
    const HTuple &hv_x_post, const HTuple &hv_y_post,
    const std::vector<int> &region,
    const int &max_age, const int &min_hits, const double &iouThreshold) {
    if (m_initialized) {
        QLOG_WARN() << "StackerTableImgPro already initialized";
        return;
    }
    try {
        // 使用传入的坐标点初始化仿射变换
        m_ins_PreProcessing.AffineInit(hv_x_pre, hv_y_pre, hv_x_post, hv_y_post);
        // m_tracker = new SORTTracker(max_age, min_hits, iouThreshold);
        m_region = cv::Rect_<int>(region[1], region[0], region[3] - region[1], region[2] - region[0]);

        HObject templateImg, modelImg, modelImgGray;

        ReadImage(&templateImg, "PusherModel.bmp");
        ReadRegion(&m_InitPositionRegion, "roi.hobj");
        ReduceDomain(templateImg, m_InitPositionRegion, &modelImg);
        Rgb1ToGray(modelImg, &modelImgGray);
        CreateNccModel(modelImgGray, "auto", HTuple(0).TupleRad(), HTuple(360).TupleRad(),
                       "auto", "use_polarity", &hv_ModelID);

        m_initialized = true;
        frameNumber = 0;
        QLOG_INFO() << "StackerTableImgPro initialized successfully with custom affine parameters";
    } catch (const HalconCpp::HException &e) {
        QLOG_ERROR() << "StackerTableImgPro init failed:" << e.ErrorMessage().Text();
        throw;
    } catch (const std::exception &e) {
        QLOG_ERROR() << "StackerTableImgPro init failed:" << e.what();
        throw;
    }
}

void StackerTableImgPro::AffineImg(const HObject &src, HObject &affine) {
    m_ins_PreProcessing.Affine(src, affine);
}

void StackerTableImgPro::cleanup() {
    m_hasStartedRecognition = false;

    // m_recognizingFrameCount = 0;

    m_tableDetectionCount = 0;
    m_totalDetectionFrames = 0;

    // 重置OCR相关变量
    m_plateNumberCounter.clear();
    m_plateValidCounter.clear();

    info.ID = "";
    info.img_name = "";
    info.recog_finished_flag = 0;
    info.conf = 0;

    m_retractFrameCount = 0;
    m_skipNextFrames = false;
    m_framesToSkip = 0;
    shouldStopOCR = false;
}

void StackerTableImgPro::AllocationBox(const DetRes &detRes, std::vector<cv::Rect_<int> > &plate_boxes) {
    auto all_boxes = detRes.getOriginalRects();
    for (auto &box: all_boxes) {
        cv::Point boxCenterPoint = ImgProcessTool::GetRectCenter(box);

        if (ImgProcessTool::IsPointInRegion(m_region, boxCenterPoint))
            plate_boxes.emplace_back(box);
    }
}

bool StackerTableImgPro::IsAtInitPosition(const HObject &img) {
    HObject ho_ImageReduced, ho_findImg;
    HTuple hv_Row, hv_Column, hv_Angle, hv_Score, hv_NumMatches;

    ReduceDomain(img, m_InitPositionRegion, &ho_ImageReduced);
    Rgb1ToGray(ho_ImageReduced, &ho_findImg);
    FindNccModel(ho_findImg, hv_ModelID, HTuple(0).TupleRad(), HTuple(360).TupleRad(),
                 0.2, 1, 0.5, "true", 0, &hv_Row, &hv_Column, &hv_Angle, &hv_Score);
    hv_NumMatches = hv_Row.TupleLength();
    // QLOG_INFO()<<"识别是否在原点";
    if (0 != (int(hv_NumMatches > 0))) {
        // QLOG_INFO() << "hv_Score:" << hv_Score.D();
        if (hv_Score.D() > 0.8) {
            // initPositionCount++;
            // if (initPositionCount > 50) {
            //     // QLOG_INFO() << "在原点超过50帧，更新原点比对模板";
            //     HObject modelImg, modelImgGray;
            //     ReduceDomain(img, m_InitPositionRegion, &modelImg);
            //     Rgb1ToGray(modelImg, &modelImgGray);
            //     CreateNccModel(modelImgGray, "auto", HTuple(0).TupleRad(), HTuple(360).TupleRad(),
            //                    "auto", "use_polarity", &hv_ModelID);
            //     WriteImage(img, "bmp", 0, "PusherModel.bmp");
            //     initPositionCount = 0;
            // }
            return true;
        }
    }
    // initPositionCount = 0;
    return false;
}

void StackerTableImgPro::UpdatePusherStatus(const HObject &img) {
    MyApplication *pApp = (MyApplication *) qApp;
    pApp->m_mem_manager->ReadFromPLC();

    // 1. 检测推钢机状态变化
    int isMotorStatus = pApp->m_mem_manager->GetFromPLCData(m_serialNumber).motor_direction;

    if (isMotorStatus == 0)
        m_pusherStatus = PAUSED;
    else if (isMotorStatus == 1)
        m_pusherStatus = PUSHING;
    else if (isMotorStatus == -1)
        m_pusherStatus = RETRACTING;

    statusChanged = (m_lastPusherStatus != m_pusherStatus);
    // QLOG_INFO() << "推钢机状态: " << colorMap[m_pusherStatus] << " (上次: " << colorMap[m_lastPusherStatus] << ")";

    // 新增：当状态变为RETRACTING时重置帧计数
    if (statusChanged && m_pusherStatus == RETRACTING) {
        m_retractFrameCount = 0; // 重置RETRACTING帧计数
    }

    // 新增：状态变化检测（RETRACTING -> PUSHING）
    if (statusChanged && m_lastPusherStatus == RETRACTING && m_pusherStatus == PUSHING) {
        m_skipNextFrames = true; // 标记需要跳过
        m_framesToSkip = m_retractFrameCount; // 跳过RETRACTING持续的帧数
        QLOG_INFO() << "状态从[收回]转为[前推]，需跳过" << m_framesToSkip << "帧";
    }

    if (m_pusherStatus == RETRACTING && !statusChanged) {
        initPositionCount++;
        if (initPositionCount > 100 && !m_hasUpdatedTemplate) {
            QLOG_INFO() << "推钢机已稳定在原点，更新推头比对模板图像";
            m_hasUpdatedTemplate = true; // 锁定，本周期不再更新
            // initPositionCount = 0;
            HObject modelImg, modelImgGray;
            ReduceDomain(img, m_InitPositionRegion, &modelImg);
            Rgb1ToGray(modelImg, &modelImgGray);
            CreateNccModel(modelImgGray, "auto", HTuple(0).TupleRad(), HTuple(360).TupleRad(),
                           "auto", "use_polarity", &hv_ModelID);
            WriteImage(img, "bmp", 0, "PusherModel.bmp");
        }
    } else {
        initPositionCount = 0;
        // 当状态离开 RETRACTING 时，重置更新锁
        if (statusChanged && m_pusherStatus != RETRACTING) {
            m_hasUpdatedTemplate = false;
        }
    }
}

cv::Rect_<int> StackerTableImgPro::GetBox(const DetRes &detRes) {
    cv::Rect_<int> pusherBox{0, 0, 0, 0};
    std::vector<cv::Rect_<int> > tableBoxes = detRes.getRectsByLabel("table");
    if (!tableBoxes.empty())
        pusherBox = tableBoxes[0];
    return pusherBox;
}

void StackerTableImgPro::processImage(const HObject &img, const DetRes &detRes, const std::vector<ocrInfo> &ocrRes) {
    // 0.刷新PLC给的信号
    // QLOG_INFO() << "垛板台主循环-1";
    UpdatePusherStatus(img);
    // QLOG_INFO() << "垛板台主循环-2";
    // 1. 获取目标检测结果（每帧只有一个box）
    cv::Rect_<int> pusherBox = GetBox(detRes);
    // QLOG_INFO() << "垛板台主循环-3";
    // 2. 处理推钢机在原点的状态
    bool is_init_position = IsAtInitPosition(img);
    // QLOG_INFO() << "垛板台主循环-4";
    if (is_init_position) {
        // if (logPrintCount % 6 == 0) //防止日志刷屏
        // if (!initPositionFlag_LOG) {
        //     initPositionFlag_LOG = true;
        //     QLOG_INFO() << "推钢机在原点";
        // }
        if (statusChanged && m_pusherStatus == PUSHING && !m_hasStartedRecognition) {
            //推钢机状态改变 、 推钢机当前为推进状态 、 当前未切换成开始识别状态
            cleanup();
            m_hasStartedRecognition = true;
            m_hasReported = false;
            shouldStopOCR = false;
            CopyImage(img, &m_startImg); // 保存起始图片
            QLOG_INFO() << "推钢机在原点，且开始推进，开始收集板号";
        }
    }
    // QLOG_INFO() << "垛板台主循环-5";
    // else
    //     initPositionFlag_LOG = false;
    // else
    // QLOG_INFO() << "推钢机不在原点";

    // 3. 处理推钢机推进过程中的状态变化
    if (m_hasStartedRecognition) {
        // m_recognizingFrameCount++; // 帧数自增
        if (m_pusherStatus == PAUSED) {
            if (!is_init_position) {
                if (statusChanged) {
                    QLOG_INFO() << "推钢机暂停中";
                }
            } else {
                m_hasStartedRecognition = false;
            }
        } else if (m_pusherStatus == RETRACTING) {
            if (!is_init_position) {
                m_retractFrameCount++; // 计数RETRACTING状态持续帧数
                if (statusChanged) {
                    QLOG_INFO() << "推钢机回退中";
                }
            } else {
                m_hasStartedRecognition = false;
            }
        } else if (m_pusherStatus == PUSHING) {
            if (statusChanged) {
                QLOG_INFO() << "推钢机推进中";
            }
            // 有推头
            // QLOG_INFO() << "pusher.box下边缘:" << pusherBox.y + pusherBox.height;
            if (m_skipNextFrames) {
                // 跳过统计和OCR收集
                // QLOG_INFO() << "跳过回退的帧数" << pusherBox.y + pusherBox.height;
                m_framesToSkip--;
                if (m_framesToSkip <= 0) {
                    m_skipNextFrames = false;
                    QLOG_INFO() << "跳帧结束，恢复推头统计和OCR收集";
                }
            } else {
                updateDetectionStatistics(img, pusherBox); //更新判断推头是否有板坯的数量
                bool lastShouldStopOCR = shouldStopOCR;
                // shouldStopOCR = shouldStopOcrCollection(ocrRes, pusherBox); // 检查是否应该停止OCR收集
                if (!shouldStopOCR) {
                    // 继续收集板号
                    // QLOG_INFO() << "持续收集并统计板号";
                    // collectOcrResult(img, ocrValues, pusherBox);
                    collectOcrResult(img, ocrRes, pusherBox);
                } else if (!lastShouldStopOCR && shouldStopOCR) {
                    QLOG_INFO() << "板号底部与推头距离小于阈值，结束当前板号收集";
                }
            }
            if ((pusherBox.y + pusherBox.height) < m_pushThreshold && pusherBox.y != 0 && !m_hasReported) {
                // 推到位，准备上报
                processPushCompletion();
            }
        }
    }
    // QLOG_INFO() << "垛板台主循环-6";
    // 4. 发送信息到PLC
    SendInfo();
    // QLOG_INFO() << "垛板台主循环-7";
    // 5. 保存上一帧状态
    m_lastPusherStatus = m_pusherStatus;
    frameNumber++;
    // logPrintCount++;
}

bool StackerTableImgPro::HasPlate(const HObject &img, const cv::Rect_<int> &pusherBox) {
    HObject reduceImg, cropImg, cropRectangle;
    GenRectangle1(&cropRectangle, pusherBox.y, pusherBox.x, pusherBox.y + pusherBox.height,
                  pusherBox.x + pusherBox.width);
    ReduceDomain(img, m_InitPositionRegion, &reduceImg);
    CropDomain(reduceImg, &cropImg);

    //todo:改成图像分类模型推理

    return true;
}

// 更新检测统计
void StackerTableImgPro::updateDetectionStatistics(const HObject &img, const cv::Rect_<int> &pusherBox) {
    //只统计y值再200到600区间的推头
    if (pusherBox.y < 200 || pusherBox.y > 600) {
        // QLOG_INFO() << "pusherBox.y:" << pusherBox.y << " 位置不在范围内，不统计推头";
        return;
    }
    m_totalDetectionFrames++; //检测到推头的总帧数
    if (HasPlate(img, pusherBox)) {
        m_tableDetectionCount++; //检测到推头带板坯的帧数
        // QLOG_INFO() << "统计: table计数=" << m_tableDetectionCount << "/" << m_totalDetectionFrames;
    }
}

int StackerTableImgPro::ModifyPlateNumber(QString &plate_number, bool isValid) {
    QString oldPlate_number = plate_number;
    if (!recentPlates.empty()) {
        if (!isValid) {
            if (plate_number.length() == 7 && plate_number[0].isLetter() && !plate_number[1].isLetter()) {
                //只获取到一位字母的情况
                QString prefix = recentPlates.back();
                prefix.truncate(2);
                plate_number = prefix.append(plate_number.mid(1, 5));
            } else {
                QString tmp = recentPlates.back();
                bool ok;
                int num = tmp.mid(5, 2).toInt(&ok);
                if (ok)
                    plate_number = tmp.mid(0, 5) + QString("%1").arg(num + 1, 2, 10, QLatin1Char('0'));
            }
            QLOG_INFO() << "垛板台：" << oldPlate_number << ",修正为:" << plate_number;
            return 0;
        } else {
            QString tmp = recentPlates.back();
            bool ok;
            int curentNum = plate_number.mid(2, 5).toInt(&ok);
            int previousNum = tmp.mid(2, 5).toInt(&ok);
            if (abs(curentNum - previousNum) < 30 && plate_number.mid(0, 2) != tmp.mid(0, 2)) {
                plate_number = tmp.mid(0, 2).append(plate_number.mid(2, 5));
                QLOG_INFO() << "垛板台：" << oldPlate_number << ",修正为:" << plate_number;
                return 0;
            } else if (plate_number.mid(5, 2).toInt(&ok) > 40)
            // ||(plate_number.mid(0, 2) == tmp.mid(0, 2) && abs(curentNum - previousNum) > 40))
            {
                plate_number = tmp.mid(0, 5) + QString("%1").arg(tmp.mid(5, 2).toInt(&ok) + 1, 2, 10,
                                                                 QLatin1Char('0'));
                QLOG_INFO() << "垛板台：" << oldPlate_number << ",修正为:" << plate_number;
                return 0;
            }
            return 1;
        }
    }

    return isValid;
}

// 收集OCR结果
void StackerTableImgPro::collectOcrResult(const HObject &img, const std::vector<ocrInfo> &ocrRes,
                                          const cv::Rect_<int> &pusherBox) {
    if (ocrRes.empty())
        return; // 没有识别到板号，继续收集
    // QLOG_INFO() << "垛板台板号数量:" <<ocrRes.size();
    int ocrBottom = ocrRes[0].GetRectSrc().y + ocrRes[0].GetRectSrc().height;
    if (!ocrRes.empty() && (ocrBottom < pusherBox.y || pusherBox.y == 0)) {
        ocrInfo topOcr = ocrRes[0];

        int isValidScore = ModifyPlateNumber(topOcr.word, topOcr.isValid);
        // plate.conf = (plate.conf + ModifyPlateNumber(plate_number, ocr.isValid)) / 2;
        // QLOG_INFO() << "垛板台当前帧板号为: " << topOcr.word;
        // 统计板号出现次数
        m_plateNumberCounter[topOcr.word]++;
        m_plateValidCounter[topOcr.word] += isValidScore;
        // 保存图片
        SaveImage(topOcr.img, topOcr.GetRect(), topOcr.word);

        QLOG_INFO() << "收集到顶部板号: " << topOcr.word
                    << "，置信度：" << QString::number(topOcr.minConf)
                    << "，出现次数: " << m_plateNumberCounter[topOcr.word];
    }

    if (!ocrRes.empty() && pusherBox.y != 0) {
        int pixelDis = pusherBox.y - ocrBottom;
        if (pixelDis < m_ocrStopThreshold || pusherBox.y < 600)
            shouldStopOCR = true;
    }
}

// 判断是否应该停止OCR收集
bool StackerTableImgPro::shouldStopOcrCollection(const std::vector<ocrInfo> &ocrRes, const cv::Rect_<int> &pusherBox) {
    if (ocrRes.empty()) {
        return false; // 没有识别到板号，继续收集
    }

    // 计算板号底部与推头顶部的交叉像素值
    int plateBottom = ocrRes[0].GetRect().y + ocrRes[0].GetRect().height;
    int pusherTop = pusherBox.y;
    int pixelDis = pusherTop - plateBottom;

    QLOG_INFO() << "板号底部: " << plateBottom << ", 推头顶部: " << pusherTop << ", 相距像素: " << pixelDis;

    // 如果交叉像素超过阈值，停止收集
    return pixelDis < m_ocrStopThreshold || pusherBox.y < 600;
}

// 处理推到位的情况
void StackerTableImgPro::processPushCompletion() {
    QLOG_INFO() << "推钢机推到位，准备上报";

    // 判断本次推钢是否有板坯
    bool hasPlate = false;
    if (m_totalDetectionFrames > 0) {
        float tableRatio = static_cast<float>(m_tableDetectionCount) / m_totalDetectionFrames;
        hasPlate = (tableRatio > 0.5);
        QLOG_INFO() << "table检测比例: " << tableRatio * 100 << "%，判定" << (hasPlate ? "有板坯" : "空推");
    }

    //***********************************************************
    hasPlate = true;
    //***********************************************************
    if (hasPlate) {
        // 有板坯的情况，从收集到的板号中找出出现次数最多的
        if (!m_plateNumberCounter.isEmpty()) {
            // 找出出现次数最多的板号
            QString finalPlateNumber = "";

            auto it = m_plateNumberCounter.constBegin();
            finalPlateNumber = it.key();
            int maxValue = it.value();

            // 遍历剩下的元素，找到最大值
            for (++it; it != m_plateNumberCounter.constEnd(); ++it) {
                if (it.value() > maxValue) {
                    maxValue = it.value();
                    finalPlateNumber = it.key();
                }
            }

            SendImage(finalPlateNumber);

            // 计算全局平均置信度
            int totalCount = m_plateNumberCounter[finalPlateNumber];
            int validCount = m_plateValidCounter[finalPlateNumber];
            double globalConf = static_cast<double>(validCount) / totalCount;
            info.conf = globalConf > 0.5 ? 1 : 0; // 综合可信度大于50%则赋1，否则为0

            info.has_steel_plate = 0;
            info.gundao_X_left = 0;
            info.ID = finalPlateNumber;
            info.img_name = QString("stackerTable_%1.jpg").arg(frameNumber % 20);
            info.recog_finished_flag = 1;

            QLOG_INFO() << "本次上料板坯号: " << finalPlateNumber
                        << "，综合可信度："<<info.conf<<"("<<validCount<<"/"<<totalCount<<")";
        } else {
            // 有板坯但未收集到合规的板号
            QLOG_INFO() << "未收集到合规的板号，上报空板号";
            info.ID = "";
            info.conf = 0;
            info.img_name = QString("stackerTable_%1.jpg").arg(frameNumber % 20);
            SendImage(m_startImg);
            info.recog_finished_flag = 1;
        }
    } else {
        // 空推头的情况，不上报板号
        QLOG_INFO() << "空推，结束当前识别周期，准备复位";
        info.ID = "";
        info.conf = 0;
        info.recog_finished_flag = 1;
    }

    m_hasReported = true;
}

void StackerTableImgPro::SendInfo() {
    try {
        MyApplication *pApp = (MyApplication *) qApp;
        //更新PLC传来的数据
        pApp->m_mem_manager->ReadFromPLC();
        plcData = pApp->m_mem_manager->GetFromPLCData(m_serialNumber);
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_steel_plate = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).gundao_X_left = 0;
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).ID, info.ID.toStdString().c_str(), 32);
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).img_name, info.img_name.toStdString().c_str(), 32);

        pApp->m_mem_manager->GetToPLCData(m_serialNumber).recog_finished_flag = info.recog_finished_flag;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_cam_broken = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak1 = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak2 = 0;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak3 = info.conf;

        pApp->m_mem_manager->CommitWorkstationData(m_serialNumber);
        pApp->m_mem_manager->WriteToPLC();
    } catch (const HalconCpp::HException &e) {
        QLOG_ERROR() << "StagingNorthImgPro processing error:" << e.ErrorMessage().Text();
        throw;
    }

    // 上报后立即重置状态，任务完成
    if (info.recog_finished_flag != 0) {
        QLOG_INFO() << "垛板台板号:" << info.ID << "上报完成";
        Sleep(50);
        info.recog_finished_flag = 0;
        if (!info.ID.isEmpty())
            updateRecentPlatesNumber(info.ID);
        cleanup();
    }
}

void StackerTableImgPro::SaveImage(const HObject &img, const cv::Rect_<int> &box, const QString &plateNumber) {
    HObject imgReduced, imgPart, rectangle;
    //修正box越界
    // HTuple Row1, Column1, Row2, Column2;
    // Row1 = box.y < 0 ? 0 : box.y;
    // Column1 = box.x < 0 ? 0 : box.x;
    // Row2 = (box.y + box.height) > 900 ? 900 : (box.y + box.height);
    // Column2 = (box.x + box.width) > 2000 ? 2000 : (box.x + box.width);
    //
    // if (Row2 != 0) {
    //     GenRectangle1(&rectangle, Row1, Column1, Row2, Column2);
    //     ReduceDomain(img, rectangle, &imgReduced);
    //     CropDomain(imgReduced, &imgPart);
    // }

    // QLOG_INFO()<<"垛板台 save image:1";
    QString imgNameLocal_SRC = QString("%1/dataSet/stackerTable_%2_dataSet.jpg").arg(ocrBufferPath).arg(plateNumber);
    QString imgNameLocal = QString("%1/dataSet/stackerTable_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    QString imgNameLocalCrop = QString("%1/dataSet/stackerTable_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);
    // QLOG_INFO()<<"垛板台 save image:2";
    // QLOG_INFO()<<"垛板台 box:"<<box.x<<","<<box.y<<"|"<<box.width<<","<<box.height;
    HObject imgRender;
    if (box.width != 0) {
        GenRectangle1(&rectangle, box.y, box.x, box.y + box.height, box.x + box.width);
        ReduceDomain(img, rectangle, &imgReduced);
        CropDomain(imgReduced, &imgPart);
        PaintRegion(rectangle, img, &imgRender, ((HTuple(255).Append(0)).Append(0)), "margin");
        // QLOG_INFO()<<"垛板台 save image:3";
        WriteImage(imgRender, "jpeg", 0, imgNameLocal.toStdString().c_str());
        WriteImage(imgPart, "jpeg", 0, imgNameLocalCrop.toStdString().c_str());
    }
    // QLOG_INFO()<<"垛板台 save image:4";
    WriteImage(img, "jpeg", 0, imgNameLocal_SRC.toStdString().c_str());
    // QLOG_INFO()<<"垛板台 save image:5";
}

void StackerTableImgPro::SendImage(const QString &plateNumber) {
    QString RfilePathSrcImage, RfilePathCropImage, desPathSrc, desPathCrop;
    RfilePathSrcImage = QString("%1/dataSet/stackerTable_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    RfilePathCropImage = QString("%1/dataSet/stackerTable_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);
    desPathSrc = QString("%1/localBuffer/stackerTable_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    desPathCrop = QString("%1/localBuffer/stackerTable_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);
    // QLOG_INFO() << "测试1："<<RfilePathSrcImage;
    CopyFile(RfilePathSrcImage.toStdString().c_str(), desPathSrc.toStdString().c_str());
    CopyFile(RfilePathCropImage.toStdString().c_str(), desPathCrop.toStdString().c_str());
    // QLOG_INFO() << "测试2："<<desPathSrc;
    HObject imgPart, img;
    QFileInfo fileInfo1(RfilePathSrcImage), fileInfo2(RfilePathCropImage);
    if (fileInfo1.exists()) {
        // QLOG_INFO() << "[上]已读取到磁盘中的大图";
        ReadImage(&img, RfilePathSrcImage.toStdString().c_str());
    }
    if (fileInfo2.exists()) {
        // QLOG_INFO() << "[上]已读取到磁盘中的小图";
        ReadImage(&imgPart, RfilePathCropImage.toStdString().c_str());
    }
    // QLOG_INFO() << "测试3";
    QString tmpRemotePath, tmpRemoteCropPath;
    tmpRemotePath = QString("%1/stackerTable_%2.jpg").arg(remotePath).arg(frameNumber % 20);
    tmpRemoteCropPath = QString("%1/stackerTable_%2_crop.jpg").arg(remotePath).arg(frameNumber % 20);

    try {
        if (fileInfo1.exists()) {
            WriteImage(img, "jpeg", 0, tmpRemotePath.toStdString().c_str());
            // QLOG_INFO() << "[上]已发送大图";
        }
        // QLOG_INFO() << "测试4";
        if (fileInfo2.exists()) {
            WriteImage(imgPart, "jpeg", 0, tmpRemoteCropPath.toStdString().c_str());
            // QLOG_INFO() << "[上]已发送小图";
        }
    } catch (HException &exception) {
        QLOG_INFO("图片传不出去了");
    }
    // QLOG_INFO() << "测试5";
}

void StackerTableImgPro::SendImage(const HObject &img) {
    QString tmpRemotePath, tmpRemoteCropPath, desPathSrc, desPathCrop;
    tmpRemotePath = QString("%1/stackerTable_%2.jpg").arg(remotePath).arg(frameNumber % 20);
    tmpRemoteCropPath = QString("%1/stackerTable_%2_crop.jpg").arg(remotePath).arg(frameNumber % 20);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateStr = currentDateTime.toString("MM_dd_hh_mm_ss_zzz");
    desPathSrc = QString("%1/localBuffer/stackerTable_(%2).jpg").arg(ocrBufferPath).arg(dateStr);
    // desPathCrop = QString("%1/localBuffer/stackerTable_(%2)_crop.jpg").arg(ocrBufferPath).arg(dateStr);

    WriteImage(img, "jpeg", 0, desPathSrc.toStdString().c_str());
    // WriteImage(img, "jpeg", 0,desPathCrop.toStdString().c_str());

    try {
        WriteImage(img, "jpeg", 0, tmpRemotePath.toStdString().c_str());
        WriteImage(img, "jpeg", 0, tmpRemoteCropPath.toStdString().c_str());
    } catch (HException &exception) {
        QLOG_INFO("图片传不出去了");
    }
}

// 更新最近板号缓存
void StackerTableImgPro::updateRecentPlatesNumber(const QString &newPlate) {
    // 如果缓存已满，移除最老的板号
    if (recentPlates.size() >= MAX_RECENT_PLATES) {
        recentPlates.dequeue();
    }

    // 添加新板号
    recentPlates.enqueue(newPlate);
}
