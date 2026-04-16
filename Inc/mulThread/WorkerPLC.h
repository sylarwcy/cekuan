//
// Created by sylar on 26-4-10.
//

#pragma once
#include <QObject>
#include <QString>

class WorkerPLC : public QObject {
    Q_OBJECT
public:
    // serialNumber 对应当前工位的编号，用于从 shared memory 获取对应数据
    WorkerPLC(QObject *parent = nullptr);
    ~WorkerPLC();

public slots:
    // 接收算法线程发来的测宽正常数据
    void onReceiveDataToPLC(double widthValue, double centerOffset, int statusFlag);

    // 接收算法线程发来的异常/报警状态
    void onSendErrorState(const QString& errorMsg);

private:
    int m_serialNumber;
};
