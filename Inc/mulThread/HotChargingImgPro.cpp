//
// Created by sylar on 25-10-24.
//

#include "HotChargingImgPro.h"
#include <QElapsedTimer>
#include "Qslog.h"
#include "WorkerCamera.h"
#include "MyApplication.h"

HotChargingImgPro::HotChargingImgPro(QObject *parent)
    : ImageProcessor(parent) {
    className = "HotCharging";
}

void HotChargingImgPro::init(
    const HTuple &hv_x_pre, const HTuple &hv_y_pre,
    const HTuple &hv_x_post, const HTuple &hv_y_post,
    const std::vector<int> &region,
    const int &max_age, const int &min_hits, const double &iouThreshold) {
    if (m_initialized) {
        QLOG_WARN() << "HotChargingImgPro already initialized";
        return;
    }
    try {
        // 使用传入的坐标点初始化仿射变换
        img_width = HalconPreprocessing::affineWidth;
        img_height = HalconPreprocessing::affineHeight;

        HObject img1, img2, img3;
        GenImageConst(&img1, "byte", 2000, 1500);
        GenImageConst(&img2, "byte", 2000, 1500);
        GenImageConst(&img3, "byte", 2000, 1500);
        Compose3(img1, img2, img3, &m_bgImg);

        left_half_width = img_width * 0.5;
        left_one_third_width = img_width * 0.33;
        m_initialized = true;
        ratioX = static_cast<double>(physicalRight - physicalLeft) / imageXMax;
        frameNumber = 0;

        // 初始化钢板队列相关变量
        plates.clear();
        last_plate_count = 0;
        last_leftmost_plate = cv::Rect();
        last_rightmost_plate = cv::Rect();

        QLOG_INFO() << "HotChargingImgPro initialized successfully with custom affine parameters";
    } catch (const HalconCpp::HException &e) {
        QLOG_ERROR() << "HotChargingImgPro init failed:" << e.ErrorMessage().Text();
        throw;
    } catch (const std::exception &e) {
        QLOG_ERROR() << "HotChargingImgPro init failed:" << e.what();
        throw;
    }
}

void HotChargingImgPro::AffineImg(const HObject &src, HObject &affine) {
    HObject zoomImg, ho_cropImg, ho_ImageReduced;

    // Rgb1ToGray(src,&ho_GrayImage);
    Rectangle1Domain(src, &ho_ImageReduced, 140, 0, 1984, 4096);
    CropDomain(ho_ImageReduced, &ho_cropImg);
    ZoomImageSize(ho_cropImg, &affine, 2000, 900, "constant");
    // ZoomImageSize(src, &zoomImg, 2000, 900, "constant");
    // PaintGray(zoomImg, m_bgImg, &affine);
}

void HotChargingImgPro::cleanup() {
    m_initialized = false;
}

void HotChargingImgPro::GetGundaoMachineState() {
    // 0-停止，1-正向运行(靠近加热炉方向)，2-反向运行(远离加热炉方向)

    MyApplication *pApp = (MyApplication *) qApp;
    // 读取PLC数据
    pApp->m_mem_manager->ReadFromPLC();
    plcData = pApp->m_mem_manager->GetFromPLCData(m_serialNumber);

    int state = plcData.motor_direction;
    if (state != 0) {
        gundao_flow_direction = state > 0 ? 1 : 2;
        gundao_current_direction = gundao_flow_direction;
    } else
        gundao_current_direction = 0;
}

void HotChargingImgPro::processImage(const HObject &img, const DetRes &detRes, const std::vector<ocrInfo> &ocrRes) {
    // return;
    // QLOG_INFO() << "热送主循环-1";
    // 0.更新辊道流向
    GetGundaoMachineState();
    // QLOG_INFO() << "热送主循环-2";
    // qDebug()<<"0";

    // 1. 提取目标检测框
    std::vector<cv::Rect_<int> > gundao_boxes = detRes.getRectsByLabel("hot");
    // QLOG_INFO() << "热送主循环-3";
    // qDebug()<<"1";

    // 2. 按x坐标排序
    std::vector<cv::Rect_<int> > sorted_gundao = gundao_boxes;
    std::sort(sorted_gundao.begin(), sorted_gundao.end(),
              [](const cv::Rect_<int> &a, const cv::Rect_<int> &b) {
                  return a.x < b.x;
              });
    // 按 point.x 从左到右（从小到大）排序
    std::vector<ocrInfo> sorted_ocrRes = ocrRes;
    std::sort(sorted_ocrRes.begin(), sorted_ocrRes.end(), [](const ocrInfo &lhs, const ocrInfo &rhs) {
        return lhs.point.x < rhs.point.x;
    });
    // QLOG_INFO() << "热送主循环-4";
    // qDebug()<<"2";

    // 3. 更新钢板位置信息
    UpdatePlatePositions(sorted_gundao);
    // QLOG_INFO() << "热送主循环-5";
    // qDebug()<<"3";

    // 4. 根据辊道方向更新钢板队列
    if (gundao_flow_direction == 2) {
        // 反向移动
        UpdatePlatesForReverseMovement(sorted_gundao);
    } else {
        // 正向移动或停止
        UpdatePlatesForForwardMovement(sorted_gundao);
    }
    // QLOG_INFO() << "热送主循环-6";
    // qDebug() << "4";

    // 5. 匹配OCR结果（只有在正向移动且无反向钢板时才进行）
    MatchOCRToPlates(img, sorted_ocrRes);
    // QLOG_INFO() << "热送主循环-7";
    // QLOG_INFO() << "热送:ocr到plates匹配完成";

    // 7. 根据辊道方向处理上报逻辑
    // if (gundao_current_direction == 2) {
    //     // 反向移动（向右）
    //     ProcessReverseLogic();
    // } else {
    // 正向移动（向左）或停止

    if (gundao_current_direction != 2)
        ProcessForwardLogic(img);
    // QLOG_INFO() << "热送主循环-8";
    // QLOG_INFO() << "热送:统计信息";
    // }
    // qDebug() << "6";

    // 8. 发送信息
    SendInfo();
    // QLOG_INFO() << "热送主循环-9";
    // 9. 更新上一帧数据
    last_plate_count = gundao_boxes.size();

    // 10. 保存当前最左侧和最右侧钢板位置，用于下一帧判断
    if (!sorted_gundao.empty()) {
        last_leftmost_plate = sorted_gundao[0];
        last_rightmost_plate = sorted_gundao.back();
    } else {
        cv::Rect_<int> empty_left_rect, empty_right_rect;
        swap(empty_left_rect, last_leftmost_plate);
        swap(empty_right_rect, last_rightmost_plate);
    }
    // QLOG_INFO() << "热送主循环-10";
    frameNumber++;
}

// 为正向移动更新钢板队列
void HotChargingImgPro::UpdatePlatesForForwardMovement(const std::vector<cv::Rect_<int> > &sorted_boxes) {
    // 如果当前帧没有钢板，清空队列
    if (sorted_boxes.empty()) {
        plates.clear();
        return;
    }

    // 检查左边钢板是否离开
    bool left_plate_leaving = IsLeftPlateLeaving(sorted_boxes);

    // 如果钢板队列为空，直接初始化
    if (plates.empty()) {
        for (const auto &box: sorted_boxes) {
            PlateInfo plate;
            plate.rect = box;
            plate.is_reverse_plate = false;
            plates.push_back(plate);
        }
        return;
    }

    // 如果左边钢板离开，左移钢板信息
    if (left_plate_leaving) {
        // 记录移出的钢板信息
        PlateInfo leaving_plate = plates[0];

        // 左移钢板信息：第二块变成第一块，第三块变成第二块...
        for (size_t i = 0; i < plates.size() - 1; i++) {
            plates[i] = plates[i + 1];
        }

        // 如果有新钢板进入（最右边），更新最后一块钢板
        if (sorted_boxes.size() >= plates.size()) {
            plates.back().rect = sorted_boxes.back();
            plates.back().reset(); // 新钢板重置信息
            plates.back().is_reverse_plate = false;
        } else {
            // 钢板数量减少，调整大小
            plates.resize(sorted_boxes.size());
        }

        // 检查移出的钢板是否需要上报删除（正向移动，从左侧离开，不上报删除）
        if (leaving_plate.reported) {
            QLOG_INFO() << QString("正向移动：钢板从左侧离开，板号:%1，不上报删除")
                               .arg(leaving_plate.plate_number);
        }

        QLOG_INFO() << "正向移动：左边钢板离开，钢板信息左移";
    }

    // 更新所有钢板的位置
    for (size_t i = 0; i < plates.size() && i < sorted_boxes.size(); i++) {
        plates[i].rect = sorted_boxes[i];
    }

    // 如果钢板数量增加，添加新钢板（在右侧）
    if (sorted_boxes.size() > plates.size()) {
        for (size_t i = plates.size(); i < sorted_boxes.size(); i++) {
            // 防鬼影限制。如果在左侧极边缘突然出现孤立检测框，忽略它。
            if (sorted_boxes[i].x < left_one_third_width) {
                QLOG_WARN() << "正向移动：在离开区极左侧发现异常孤立检测框，判定为残影，忽略入队";
                continue;
            }
            PlateInfo new_plate;
            new_plate.rect = sorted_boxes[i];
            new_plate.is_reverse_plate = false;
            plates.push_back(new_plate);
        }
    }

    for (auto &plate: plates) {
        if (plate.steel_length != 0) continue;

        // plate.gundao_X_left = plate.rect.x;
        int centerPoint = plate.rect.x + plate.rect.width / 2;
        //判断该板坯X轴方向，是否在100~1900范围内，且过了中点
        if (plate.rect.x > 80
            && plate.rect.x + plate.rect.width < 1920
            && centerPoint < left_half_width + 100) {
            //计算该块板坯的长度
            plate.steel_length = calculateDistance(plate.rect.x, plate.rect.x + plate.rect.width);
        }
    }
}

// 为反向移动更新钢板队列
void HotChargingImgPro::UpdatePlatesForReverseMovement(const std::vector<cv::Rect_<int> > &sorted_boxes) {
    // 如果当前帧没有钢板，清空队列
    if (sorted_boxes.empty()) {
        plates.clear();
        return;
    }

    // 检查右边钢板是否离开
    bool right_plate_leaving = IsRightPlateLeaving(sorted_boxes);

    // 如果钢板队列为空，直接初始化（反向移动时，所有钢板都标记为已上报的反向钢板）
    if (plates.empty()) {
        for (const auto &box: sorted_boxes) {
            PlateInfo plate;
            plate.rect = box;
            if (box.x < left_one_third_width) {
                plate.reported = true; // 反向移动时，钢板视为已上报
                plate.is_reverse_plate = true; // 标记为反向钢板
            } else {
                plate.reported = false;
                plate.is_reverse_plate = false;
            }
            plates.push_back(plate);
        }
        QLOG_INFO() << QString("反向移动：初始化钢板队列，%1块钢板都标记为已上报").arg(plates.size());
        return;
    }

    // 如果右边钢板离开，右移钢板信息
    if (right_plate_leaving) {
        // 记录移出的钢板信息
        PlateInfo leaving_plate = plates.back();

        // 右移钢板信息：倒数第二块变成最后一块，倒数第三块变成倒数第二块...
        for (int i = plates.size() - 1; i > 0; i--) {
            plates[i] = plates[i - 1];
        }

        // 如果有新钢板进入（最左边），更新第一块钢板
        if (sorted_boxes.size() >= plates.size()) {
            plates[0].rect = sorted_boxes[0];
            plates[0].reported = true; // 新钢板也标记为已上报
            plates[0].is_reverse_plate = true;
        } else {
            // 钢板数量减少，调整大小
            plates.erase(plates.begin(), plates.begin() + plates.size() - sorted_boxes.size());
        }

        // 检查移出的钢板是否需要上报删除（反向移动，从右侧离开，上报删除）
        if (leaving_plate.reported) {
            info.delete_flag = 1;
            info.recog_finished_flag = 0;
            info.ID = "";
            info.img_name = "";
            QLOG_INFO() << QString("反向移动：钢板从右侧离开，上报删除信号");

            // 如果这块离开的钢板有记录板号，从最近上报缓存中同步移除
            if (!leaving_plate.plate_number.isEmpty()) {
                // QQueue 继承自 QList，可以直接使用 removeAll 或 removeOne
                int removedCount = recentPlates.removeAll(leaving_plate.plate_number);
                if (removedCount > 0) {
                    QLOG_INFO() << QString("同步清理缓存：已从 recentPlates 中移除被删除的板号[%1]").arg(leaving_plate.plate_number);
                }
            }
        }
    }

    // 更新所有钢板的位置
    for (size_t i = 0; i < plates.size() && i < sorted_boxes.size(); i++) {
        plates[plates.size() - 1 - i].rect = sorted_boxes[sorted_boxes.size() - 1 - i];
    }

    // 如果钢板数量增加，添加新钢板（在左侧）
    if (sorted_boxes.size() > plates.size()) {
        // 在队列开头插入新钢板
        int new_plate_count = sorted_boxes.size() - plates.size();
        for (int i = new_plate_count - 1; i >= 0; i--) {
            PlateInfo new_plate;
            new_plate.rect = sorted_boxes[i];
            if (sorted_boxes[i].x < left_one_third_width) {
                new_plate.reported = true; // 反向移动的新钢板也标记为已上报
                new_plate.is_reverse_plate = true;
            } else {
                new_plate.reported = false;
                new_plate.is_reverse_plate = false;
            }
            plates.insert(plates.begin(), new_plate);
        }
        QLOG_INFO() << QString("反向移动：左侧新进入%1块钢板，标记为已上报").arg(new_plate_count);
    }
}

// 判断左边钢板是否离开
bool HotChargingImgPro::IsLeftPlateLeaving(const std::vector<cv::Rect_<int> > &sorted_boxes) {
    // 如果钢板数量减少，说明有钢板离开
    if (sorted_boxes.size() < last_plate_count) {
        return true;
    }

    // 如果钢板数量不变，但最左侧钢板右边界变化很大，说明左边钢板离开同时有右边新钢板进入
    if (!sorted_boxes.empty() && !last_leftmost_plate.empty()) {
        int last_right = last_leftmost_plate.x + last_leftmost_plate.width;
        int current_right = sorted_boxes[0].x + sorted_boxes[0].width;

        // 如果右边界变化超过画面宽度的三分之一，认为左边钢板离开
        if (abs(current_right - last_right) > img_width / 3) {
            return true;
        }
    }

    return false;
}

// 判断右边钢板是否离开
bool HotChargingImgPro::IsRightPlateLeaving(const std::vector<cv::Rect_<int> > &sorted_boxes) {
    // 如果钢板数量减少，说明有钢板离开
    if (sorted_boxes.size() < last_plate_count) {
        return true;
    }

    // 如果钢板数量不变，但最右侧钢板左边界变化很大，说明右边钢板离开同时有左边新钢板进入
    if (!sorted_boxes.empty() && !last_rightmost_plate.empty()) {
        int last_left = last_rightmost_plate.x;
        int current_left = sorted_boxes.back().x;

        // 如果左边界变化超过画面宽度的三分之一，认为右边钢板离开
        if (abs(current_left - last_left) > img_width / 3) {
            return true;
        }
    }

    return false;
}

int HotChargingImgPro::ModifyPlateNumber(QString &plate_number, bool isValid) {
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
            QLOG_INFO() << "热送：" << oldPlate_number << ",修正为:" << plate_number;
            return 0;
        } else {
            QString tmp = recentPlates.back();
            bool ok;
            int curentNum = plate_number.mid(2, 5).toInt(&ok);
            int previousNum = tmp.mid(2, 5).toInt(&ok);
            if (abs(curentNum - previousNum) < 30 && plate_number.mid(0, 2) != tmp.mid(0, 2)) {
                plate_number = tmp.mid(0, 2).append(plate_number.mid(2, 5));
                QLOG_INFO() << "热送：" << oldPlate_number << ",修正为:" << plate_number;
                return 0;
            } else if (plate_number.mid(5, 2).toInt(&ok) > 40)
            // ||(plate_number.mid(0, 2) == tmp.mid(0, 2) && abs(curentNum - previousNum) > 40))
            {
                plate_number = tmp.mid(0, 5) + QString("%1").arg(tmp.mid(5, 2).toInt(&ok) + 1, 2, 10, QLatin1Char('0'));
                QLOG_INFO() << "热送：" << oldPlate_number << ",修正为:" << plate_number;
                return 0;
            }
            return 1;
        }
    }
    return isValid;
}

// 匹配OCR到钢板
void HotChargingImgPro::MatchOCRToPlates(const HObject &img, const std::vector<ocrInfo> &ocrRes) {
    if (ocrRes.empty() || plates.empty() || gundao_flow_direction > 1)
        return;
    // QLOG_INFO()<<"**热送板号[开始]**";
    for (const auto &ocr: ocrRes) {
        // QLOG_INFO()<<"原始板号: "<<ocr.word<<"，置信度："<<ocr.minConf;
        // 寻找OCR中心点所在的钢板
        // QLOG_INFO()<<"热送板子数量："<<plates.size();
        for (auto &plate: plates) {
            if (plate.reported) continue;
            if (plate.rect.contains(ocr.srcImgPoint)) {
                QString plate_number = ocr.word;

                int isValidScore = ModifyPlateNumber(plate_number, ocr.isValid);
                plate.total_ocr_count++; // 累加总次数
                plate.valid_ocr_count += isValidScore; // 累加有效次数 (0或1)
                plate.conf = static_cast<double>(plate.valid_ocr_count) / plate.total_ocr_count; // 计算全局平均置信度

                auto it = plate.gundao_ocr_count.find(plate_number);
                if (it != plate.gundao_ocr_count.end()) // 已存在，递增
                    it.value()++;
                else // 不存在，初始化为1
                    plate.gundao_ocr_count.insert(plate_number, 1);
                // QLOG_INFO()<<"板号放入map中";
                SaveImage(ocr.img, ocr.GetRect(), plate_number);
                // QLOG_INFO()<<"板号图片保存到硬盘";
                break;
            }
        }
    }
    // QLOG_INFO()<<"**热送板号[结束]**";
}

// 处理正向逻辑（向左移动）
void HotChargingImgPro::ProcessForwardLogic(const HObject &img) {
    // 正向移动时，钢板从右向左移动
    for (size_t i = 0; i < plates.size(); i++) {
        // 只处理正向钢板（非反向钢板）
        if (plates[i].is_reverse_plate || plates[i].reported)
            continue;

        if (!plates[i].reported) {
            if (plates[i].steel_length > 0) {
                QMap<QString, int> tmpGundao_ocr_count = plates[i].gundao_ocr_count;
                if (!tmpGundao_ocr_count.isEmpty()) {
                    QMap<QString, int>::const_iterator constIt = tmpGundao_ocr_count.constBegin();
                    for (; constIt != tmpGundao_ocr_count.constEnd(); ++constIt) {
                        if (constIt.value() >= REPORT_THRESHOLD) {
                            plates[i].plate_number = constIt.key();

                            info.ID = plates[i].plate_number;
                            info.img_name = QString("hotCharging_%1.jpg").arg(frameNumber % 20);
                            info.recog_finished_flag = 1;

                            info.gundao_X_left = plates[i].rect.x;
                            info.steel_length = plates[i].steel_length;
                            info.conf = plates[i].conf > 0.5;

                            plates[i].reported = true;
                            SendImage(info.ID);
                            // CleanBufferImg(info.ID, plates[i].gundao_ocr_count);
                            plates[i].gundao_ocr_count.clear();
                            // QLOG_INFO() << "上报：" << it.key();
                            break;
                        }
                    }
                }
            }

            if (!plates[i].reported && (plates[i].rect.x + plates[i].rect.width < left_one_third_width) && plates[i].
                plate_number.isEmpty()) {
                if (plates[i].steel_length > 150) {
                    //至少1米5长

                    info.ID = "";
                    info.img_name = QString("hotCharging_%1.jpg").arg(frameNumber % 20);
                    info.recog_finished_flag = 1;

                    info.gundao_X_left = plates[i].rect.x;
                    info.steel_length = plates[i].steel_length;
                    info.conf = 0;

                    plates[i].reported = true;
                    SendImage(img);
                    CleanBufferImg(info.ID, plates[i].gundao_ocr_count);
                    plates[i].gundao_ocr_count.clear();
                } else {
                    QLOG_WARN() << "拦截异常兜底上报：左侧发现目标但长度不足(" << plates[i].steel_length << ")，判定为残影丢弃";
                    plates[i].reported = true;
                }
            }
        }
    }
}

// 处理反向逻辑（向右移动）
/*void HotChargingImgPro::ProcessReverseLogic() {
    // 反向移动时，钢板从左向右移动

    // 反向移动时不进行正常的板号上报
    // 删除信号已经在UpdatePlatesForReverseMovement中处理

    // 这里只处理一些特殊情况，比如钢板接近右侧边缘但尚未离开
    for (size_t i = 0; i < plates.size(); i++) {
        // 检查钢板是否接近右侧边缘
        if (plates[i].rect.x + plates[i].rect.width > img_width - 100) {
            QLOG_DEBUG() << QString("反向移动：钢板[%1]接近右侧边缘，板号：%2")
                                .arg(i)
                                .arg(plates[i].plate_number);
        }
    }
}*/

// 更新钢板位置信息
void HotChargingImgPro::UpdatePlatePositions(const std::vector<cv::Rect_<int> > &gundao_boxes) {
    info.has_steel_plate = gundao_boxes.size();
    info.gundao_X_left = 0;
    info.recog_finished_flag = 0;
    info.delete_flag = 0;
    info.steel_length = 0;

    // if (!gundao_boxes.empty()) {
    //     // 获取最左侧钢板的位置
    //     info.gundao_X_left = gundao_boxes[0].x < 200 ? 0 : gundao_boxes[0].x;
    //     if (info.gundao_X_left > imageXMax)
    //         info.gundao_X_left = 0;
    //     //获取宽度最大的板坯目标
    //     auto max_width_rect = *std::max_element(
    //         gundao_boxes.begin(), gundao_boxes.end(),
    //         [](const cv::Rect_<int> &a, const cv::Rect_<int> &b) {
    //             return a.width < b.width;
    //         });
    //     //获取该板坯的x轴中心点
    //     int centerPoint = max_width_rect.x + max_width_rect.width / 2;
    //
    //     //判断该板坯X轴方向，是否在100~1900范围内，且过了中点
    //     if (max_width_rect.x > 50
    //         && max_width_rect.x + max_width_rect.width < 1950
    //         && centerPoint < left_half_width + 100) {
    //         //计算该块板坯的长度
    //         info.steel_length = calculateDistance(max_width_rect.x, max_width_rect.x + max_width_rect.width);
    //     } else
    //         info.steel_length = 0;
    // }
}

void HotChargingImgPro::SendInfo() {
    if (info.recog_finished_flag == 1 && info.delete_flag == 0 && info.steel_length <= 0) {
        QLOG_WARN() << "强制拦截：触发上报但钢板有效长度为0，停止向PLC下发数据。";
        info.recog_finished_flag = 0; // 重置标志位，中断本次上报
        return;
    }
    try {
        // QLOG_INFO() << "HotChargingImgPro 上报钢板信息";

        //处理重复上报
        if (!shouldReportPlate(info.ID) && info.recog_finished_flag) {
            info.recog_finished_flag = 0;
            QLOG_INFO() << QString("%1，板号重复上报，屏蔽").arg(info.ID);
        }

        MyApplication *pApp = (MyApplication *) qApp;
        //更新PLC传来的数据
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_steel_plate = info.has_steel_plate;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).gundao_X_left = coordinateXTrans(info.gundao_X_left);
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).ID, info.ID.toStdString().c_str(), 32);
        memcpy(pApp->m_mem_manager->GetToPLCData(m_serialNumber).img_name, info.img_name.toStdString().c_str(), 32);
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).recog_finished_flag = info.recog_finished_flag;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak1 = info.delete_flag;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak2 = info.steel_length;
        pApp->m_mem_manager->GetToPLCData(m_serialNumber).bak3 = info.conf;

        pApp->m_mem_manager->GetToPLCData(m_serialNumber).has_cam_broken = 0;

        pApp->m_mem_manager->CommitWorkstationData(m_serialNumber);
        pApp->m_mem_manager->WriteToPLC();
    } catch (const HalconCpp::HException &e) {
        QLOG_ERROR() << "HotChargingImgPro processing error:" << e.ErrorMessage().Text();
        throw;
    }

    if (info.recog_finished_flag) {
        Sleep(200);
        QLOG_INFO() << "热送上报：" << info.ID << "，长度：" << info.steel_length;
        if (!info.ID.isEmpty())
            updateRecentPlatesNumber(info.ID);
        info.ID = "";
    }
}

int HotChargingImgPro::coordinateXTrans(int imageX) {
    if (imageX == 0) return 0;
    // if (imageX < 0 || imageX > 2000) // 参数验证
    //     throw std::out_of_range("imageX is out of valid range");

    return static_cast<int>(physicalLeft + ratioX * imageX);
}

void HotChargingImgPro::CleanBufferImg(QString plateNumber, QMap<QString, int> ocr_count) {
    for (auto element: ocr_count.keys()) {
        if (element == plateNumber) continue;
        QString imgNameLocal = QString("%1/localBuffer/hotCharging_%2.jpg").arg(ocrBufferPath).arg(element);
        QString imgNameLocalCrop = QString("%1/localBuffer/hotCharging_%2_crop.jpg").arg(ocrBufferPath).arg(
            element);
        QFile::remove(imgNameLocal);
        QFile::remove(imgNameLocalCrop);
    }
}

void HotChargingImgPro::SaveImage(const HObject &img, const cv::Rect_<int> &box, const QString &plateNumber) {
    // QLOG_INFO()<<"save image:"<<plateNumber;
    HObject imgReduced, imgPart, rectangle;
    //修正box越界
    // HTuple Row1, Column1, Row2, Column2;
    // Row1 = box.y < 0 ? 0 : box.y;
    // Column1 = box.x < 0 ? 0 : box.x;
    // Row2 = (box.y + box.height) > 900 ? 900 : (box.y + box.height);
    // Column2 = (box.x + box.width) > 2000 ? 2000 : (box.x + box.width);
    // // QLOG_INFO() << "修正坐标完成";
    // if (Row2 != 0) {
    //     GenRectangle1(&rectangle, Row1, Column1, Row2, Column2);
    //     ReduceDomain(img, rectangle, &imgReduced);
    //     CropDomain(imgReduced, &imgPart);
    // }
    // QLOG_INFO()<<"热送 save image:1";
    QString imgNameLocal_SRC = QString("%1/dataSet/hotCharging_%2_dataSet.jpg").arg(ocrBufferPath).arg(plateNumber);
    QString imgNameLocal = QString("%1/dataSet/hotCharging_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    QString imgNameLocalCrop = QString("%1/dataSet/hotCharging_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);
    // QLOG_INFO() << "热送 save image:2";
    // QLOG_INFO() << "热送 box:" << box.x << "," << box.y << "|" << box.width << "," << box.height;
    HObject imgRender;
    if (box.width != 0) {
        GenRectangle1(&rectangle, box.y, box.x, box.y + box.height, box.x + box.width);
        ReduceDomain(img, rectangle, &imgReduced);
        CropDomain(imgReduced, &imgPart);
        PaintRegion(rectangle, img, &imgRender, ((HTuple(255).Append(0)).Append(0)), "margin");
        // QLOG_INFO() << "热送 save image:3";
        WriteImage(imgRender, "jpeg", 0, imgNameLocal.toStdString().c_str());
        WriteImage(imgPart, "jpeg", 0, imgNameLocalCrop.toStdString().c_str());
    }
    // QLOG_INFO() << "热送 save image:4";
    WriteImage(img, "jpeg", 0, imgNameLocal_SRC.toStdString().c_str());
    // QLOG_INFO()<<"热送 save image:5";
}

void HotChargingImgPro::SendImage(const HObject &img) {
    QString tmpRemotePath, tmpRemoteCropPath, desPathSrc, desPathCrop;
    tmpRemotePath = QString("%1/hotCharging_%2.jpg").arg(remotePath).arg(frameNumber % 20);
    tmpRemoteCropPath = QString("%1/hotCharging_%2_crop.jpg").arg(remotePath).arg(frameNumber % 20);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateStr = currentDateTime.toString("MM_dd_hh_mm_ss_zzz");
    desPathSrc = QString("%1/localBuffer/hotCharging_(%2).jpg").arg(ocrBufferPath).arg(dateStr);
    // desPathCrop = QString("%1/localBuffer/hotCharging_(%2)_crop.jpg").arg(ocrBufferPath).arg(dateStr);

    WriteImage(img, "jpeg", 0, desPathSrc.toStdString().c_str());
    // WriteImage(img, "jpeg", 0,desPathCrop.toStdString().c_str());
    try {
        WriteImage(img, "jpeg", 0, tmpRemotePath.toStdString().c_str());
        WriteImage(img, "jpeg", 0, tmpRemoteCropPath.toStdString().c_str());
    } catch (HException &exception) {
        QLOG_INFO("图片传不出去了");
    }
}

void HotChargingImgPro::SendImage(const QString &plateNumber) {
    QString RfilePathSrcImage, RfilePathCropImage, desPathSrc, desPathCrop;
    RfilePathSrcImage = QString("%1/dataSet/hotCharging_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    RfilePathCropImage = QString("%1/dataSet/hotCharging_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);
    desPathSrc = QString("%1/localBuffer/hotCharging_%2.jpg").arg(ocrBufferPath).arg(plateNumber);
    desPathCrop = QString("%1/localBuffer/hotCharging_%2_crop.jpg").arg(ocrBufferPath).arg(plateNumber);

    CopyFile(RfilePathSrcImage.toStdString().c_str(), desPathSrc.toStdString().c_str());
    CopyFile(RfilePathCropImage.toStdString().c_str(), desPathCrop.toStdString().c_str());

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

    QString tmpRemotePath, tmpRemoteCropPath;
    tmpRemotePath = QString("%1/hotCharging_%2.jpg").arg(remotePath).arg(frameNumber % 20);
    tmpRemoteCropPath = QString("%1/hotCharging_%2_crop.jpg").arg(remotePath).arg(frameNumber % 20);

    try {
        if (fileInfo1.exists()) {
            WriteImage(img, "jpeg", 0, tmpRemotePath.toStdString().c_str());
            // QLOG_INFO() << "[上]已发送大图";
        }

        if (fileInfo2.exists()) {
            WriteImage(imgPart, "jpeg", 0, tmpRemoteCropPath.toStdString().c_str());
            // QLOG_INFO() << "[上]已发送小图";
        }
    } catch (HException &exception) {
        QLOG_INFO("图片传不出去了");
    }
}

// 检查板号是否需要上报
bool HotChargingImgPro::shouldReportPlate(const QString &plateNumber) {
    if (plateNumber.isEmpty())
        return true;
    // 检查板号是否在最近5个中出现过
    if (recentPlates.contains(plateNumber)) {
        qDebug() << "板号" << plateNumber << "最近已出现，跳过上报";
        return false;
    }

    return true; // 需要上报
}

// 更新最近板号缓存
void HotChargingImgPro::updateRecentPlatesNumber(const QString &newPlate) {
    // 如果缓存已满，移除最老的板号
    if (recentPlates.size() >= MAX_RECENT_PLATES) {
        recentPlates.dequeue();
    }

    // 添加新板号
    recentPlates.enqueue(newPlate);
}

double HotChargingImgPro::calculateDistance(double left_px, double right_px) {
    // 确保坐标在有效范围内
    left_px = std::max(0.0, std::min(left_px, 2000.0));
    right_px = std::max(0.0, std::min(right_px, 2000.0));

    // 确保左坐标不大于右坐标
    if (left_px > right_px) {
        std::swap(left_px, right_px);
    }

    double total_distance = 0.0;

    // 遍历所有区间，累加距离
    for (const auto &interval: INTERVALS) {
        // 如果区间与[left_px, right_px]有重叠
        if (interval.right_px > left_px && interval.left_px < right_px) {
            // 计算重叠部分的像素长度
            double overlap_left = std::max(left_px, interval.left_px);
            double overlap_right = std::min(right_px, interval.right_px);
            double overlap_px = overlap_right - overlap_left;

            if (overlap_px > 0) {
                // 根据每像素物理距离计算物理距离
                total_distance += overlap_px * interval.meters_per_px;
            }
        }
    }

    return total_distance;
}
