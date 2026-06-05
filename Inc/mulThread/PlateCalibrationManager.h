//
// Created by sylar on 26-6-5.
//

#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <halconcpp/HalconCpp.h>
#include "workStationDataStructure.h"

class PlateCalibrationManager {
public:
    PlateCalibrationManager() = default;
    ~PlateCalibrationManager() = default;

    // 1. 全局全新标定初始化
    void resetGlobalCalibration() {
        m_globalCalculatedOffsets.clear();
        m_passPointCounts.clear(); // 清空趟数历史
        clearSinglePassCache();
    }

    // 2. 清空单趟的临时点云
    void clearSinglePassCache() {
        m_singlePassRows.clear();
        m_singlePassColsLeft.clear();
        m_singlePassColsRight.clear();
    }

    // 3. 在线收集单帧点云
    void feedCalibrationFrame(const WidthResult& res) {
        if (res.contourRows.isEmpty()) return;
        int pRows = res.contourRows.size();
        int halfLen = (pRows - 1) / 2;
        for (int i = 0; i < halfLen; ++i) {
            m_singlePassRows.append(res.contourRows[i]);
            m_singlePassColsLeft.append(res.contourColsLeft[i]);
            m_singlePassColsRight.append(res.contourColsLeft[pRows - 2 - i]);
        }
    }

    // 4. 当单趟钢坯完全离开时触发：结算该趟偏航角，剥离误差后并入全局大池子
    bool finishCurrentPass(double trueWidthMm, double nominalMmPerPixel) {
        int totalPoints = m_singlePassRows.size();
        if (totalPoints < 50) {
            clearSinglePassCache();
            return false;
        }

        // 最小二乘法拟合当前趟独立的斜率 k
        double sum_y = 0, sum_x = 0, sum_y2 = 0, sum_xy = 0;
        for (int i = 0; i < totalPoints; ++i) {
            double y = m_singlePassRows[i];
            double x = (m_singlePassColsLeft[i] + m_singlePassColsRight[i]) / 2.0;
            sum_y += y; sum_x += x; sum_y2 += y * y; sum_xy += x * y;
        }
        double denom = totalPoints * sum_y2 - sum_y * sum_y;
        double k = (std::abs(denom) > 1e-6) ? (totalPoints * sum_xy - sum_x * sum_y) / denom : 0.0;
        double cosTheta = 1.0 / std::sqrt(1.0 + k * k);

        // 剥离当前趟倾斜引起的虚宽投影
        int pointsAdded = 0;
        for (int i = 0; i < totalPoints; ++i) {
            double u_m = m_singlePassColsLeft[i] + 700.0;
            double u_s = (m_singlePassColsRight[i] - 3396.0) + 0.0;
            double offset = (trueWidthMm / cosTheta) - nominalMmPerPixel * (u_s - u_m);
            m_globalCalculatedOffsets.append(offset);
            pointsAdded++;
        }

        // 🌟 记录这一趟贡献了多少个有效特征点，以供后续精准撤销
        m_passPointCounts.append(pointsAdded);

        clearSinglePassCache();
        return true;
    }

    // 🌟 新增：一键删除/撤销上一趟录入的数据包
    bool removeLastPass() {
        if (m_passPointCounts.isEmpty() || m_globalCalculatedOffsets.isEmpty()) {
            return false;
        }
        // 弹出最后一趟的点数，并在大池子尾部等量斩断
        int pointsToRemove = m_passPointCounts.takeLast();
        for (int i = 0; i < pointsToRemove; ++i) {
            if (!m_globalCalculatedOffsets.isEmpty()) {
                m_globalCalculatedOffsets.removeLast();
            }
        }
        return true;
    }

    // 5. 最终一键合并多趟数据计算、生成 Halcon 字典文件落盘
    bool finalizeGlobalCalibration(double nominalMmPerPixel, const QString& mPath, const QString& sPath) {
        if (m_globalCalculatedOffsets.isEmpty()) return false;

        std::sort(m_globalCalculatedOffsets.begin(), m_globalCalculatedOffsets.end());
        double finalStitchOffset = m_globalCalculatedOffsets[m_globalCalculatedOffsets.size() / 2];

        try {
            HalconCpp::HDict masterDict, slaveDict;
            masterDict.CreateDict(); slaveDict.CreateDict();

            masterDict.SetDictTuple("Coef_a", 0.0); masterDict.SetDictTuple("Coef_b", 0.0);
            masterDict.SetDictTuple("Coef_c", nominalMmPerPixel); masterDict.SetDictTuple("Coef_d", 0.0);

            slaveDict.SetDictTuple("Coef_a", 0.0); slaveDict.SetDictTuple("Coef_b", 0.0);
            slaveDict.SetDictTuple("Coef_c", nominalMmPerPixel); slaveDict.SetDictTuple("Coef_d", finalStitchOffset);

            HalconCpp::WriteDict(masterDict, mPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple());
            HalconCpp::WriteDict(slaveDict, sPath.toLocal8Bit().constData(), HalconCpp::HTuple(), HalconCpp::HTuple());
            return true;
        } catch (...) { return false; }
    }

    int getGlobalPointsCount() const { return m_globalCalculatedOffsets.size(); }
    int getPassesCount() const { return m_passPointCounts.size(); }

private:
    QVector<double> m_singlePassRows;
    QVector<double> m_singlePassColsLeft;
    QVector<double> m_singlePassColsRight;
    QVector<double> m_globalCalculatedOffsets;
    QVector<int> m_passPointCounts; // 趟数历史记账本
};