//
// Created by sylar on 25-10-11.
//

#ifndef SHAREDMEMORYMANAGER_H
#define SHAREDMEMORYMANAGER_H

#include "mysharemem.h"
#include "SharedMemoryDataStructure.h"
#include <QObject>
#include <QMap>
#include <QMutex>
#include "Workstation.h"

class SharedMemoryManager : public QObject {
    Q_OBJECT

public:
    SharedMemoryManager(const QList<Workstation *> &workstationList);

    ~SharedMemoryManager() = default;

    // 共享内存对象
    MyShareMem m_to_plc_mem, m_from_plc_mem; //共享内存对象

    // 从PLC读取的数据
    QVector<From_PLC_DATA> m_from_plc_data_list;
    // 发送到PLC的数据 - 双缓冲区
    QVector<TO_PLC_DATA> m_to_plc_data_list;           // 稳定缓冲区（用于打包发送）
    QVector<TO_PLC_DATA> m_to_plc_data_list_temp;      // 临时缓冲区（用于线程修改）

    QList<Workstation *> m_workstationList; //工位列表

    void init(const char *from_plc_mem_name, const char *to_plc_mem_name); //初始化

    void ReadFromPLC();
    void WriteToPLC();

    // 获取所有工作站的PLC数据（raw）
    QVector<From_PLC_DATA> &GetRawFromPLCData();
    QVector<TO_PLC_DATA> &GetRawToPLCData();

    // 更新特定工作站的数据到共享内存缓冲区
    void UpdateWorkstationData(int workstationIndex, const From_PLC_DATA &fromData, const TO_PLC_DATA &toData);

    // 获取特定工作站的PLC数据 - 返回临时缓冲区的引用供修改
    From_PLC_DATA &GetFromPLCData(int workstationIndex);
    TO_PLC_DATA &GetToPLCData(int workstationIndex);

    // 提交临时缓冲区的修改到稳定缓冲区（用于线程完成修改后调用）
    void CommitWorkstationData(int workstationIndex);

private:
    // 将多个工作站数据拼接成完整的数据包
    void PackAllWorkstationData();

    // 将完整数据包拆分为各个工作站的数据
    void UnpackAllWorkstationData();

    // 完整的数据包缓冲区
    QByteArray m_from_plc_packed_data;
    QByteArray m_to_plc_packed_data;

    int m_workStationCount;//工位数量

    QMutex m_data_mutex; // 数据访问互斥锁
    QMutex m_commit_mutex; // 提交操作互斥锁
};


#endif //SHAREDMEMORYMANAGER_H