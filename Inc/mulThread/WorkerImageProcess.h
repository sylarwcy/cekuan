#pragma once
#include <QObject>
#include "workStationDataStructure.h"

class WorkerImageProcess : public QObject
{
    Q_OBJECT
public:
    explicit WorkerImageProcess(QObject *parent = nullptr);
    ~WorkerImageProcess();

public slots:
    // 唯一接收入口
    void onProcessDualChunk(const DualCameraChunk& chunk);

    signals:
        // 唯一输出出口
        void signalMeasureFinished(const MeasureResult& result);
};