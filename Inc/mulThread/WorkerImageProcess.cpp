#include "WorkerImageProcess.h"

WorkerImageProcess::WorkerImageProcess(QObject *parent) : QObject(parent) {}
WorkerImageProcess::~WorkerImageProcess() {}

void WorkerImageProcess::onProcessDualChunk(const DualCameraChunk& chunk)
{
    MeasureResult result;
    result.frameID = chunk.frameID;
    
    // -------------------------------------------------------------
    // 将来在这里调用 Halcon 的函数处理 chunk.imgLeft 和 chunk.imgRight
    // 目前随便写死几个假数据，保证程序跑通。
    // -------------------------------------------------------------
    result.leftEdgeX = 100.0;
    result.rightEdgeX = 2000.0;
    result.finalWidth = 1900.0;
    result.isValid = true;

    emit signalMeasureFinished(result);
}