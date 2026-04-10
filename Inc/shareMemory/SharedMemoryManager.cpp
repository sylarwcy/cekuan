//
// Created by sylar on 25-10-11.
//

#include "SharedMemoryManager.h"
#include "QsLogDisableForThisFile.h"

SharedMemoryManager::SharedMemoryManager(const QList<Workstation *> &workstationList) {
    m_workstationList = workstationList;

    // 根据工作站数量初始化数据列表
    m_workStationCount = m_workstationList.size();
    m_from_plc_data_list.resize(m_workStationCount);
    m_to_plc_data_list.resize(m_workStationCount);
    m_to_plc_data_list_temp.resize(m_workStationCount); // 初始化临时缓冲区

    // 初始化数据包缓冲区大小
    m_from_plc_packed_data.resize(sizeof(From_PLC_DATA) * m_workStationCount);
    m_to_plc_packed_data.resize(sizeof(TO_PLC_DATA) * m_workStationCount);

    // 清零初始化
    for (int i = 0; i < m_workStationCount; ++i) {
        memset(&m_from_plc_data_list[i], 0, sizeof(From_PLC_DATA));
        memset(&m_to_plc_data_list[i], 0, sizeof(TO_PLC_DATA));
        memset(&m_to_plc_data_list_temp[i], 0, sizeof(TO_PLC_DATA)); // 临时缓冲区也清零
    }
    memset(m_from_plc_packed_data.data(), 0, m_from_plc_packed_data.size());
    memset(m_to_plc_packed_data.data(), 0, m_to_plc_packed_data.size());
}

void SharedMemoryManager::init(const char *from_plc_mem_name, const char *to_plc_mem_name) {
    // int number = m_workstationList.size();

    m_from_plc_mem.SetShareMem(QString::fromUtf8(from_plc_mem_name), sizeof(From_PLC_DATA) * m_workStationCount);
    m_to_plc_mem.SetShareMem(QString::fromUtf8(to_plc_mem_name), sizeof(TO_PLC_DATA) * m_workStationCount);

    QString str;
    str = QString(u8"数据采集共享内存名:%1, 共享内存长度:%2 建立完毕！").arg(from_plc_mem_name).arg(sizeof(From_PLC_DATA) * m_workStationCount);
    QLOG_INFO() << str;

    str = QString(u8"数据发送共享内存名:%1, 共享内存长度:%2 建立完毕！").arg(to_plc_mem_name).arg(sizeof(TO_PLC_DATA) * m_workStationCount);
    QLOG_INFO() << str;
}

void SharedMemoryManager::ReadFromPLC() {
    QMutexLocker locker(&m_data_mutex);
    // 从共享内存读取完整数据包
    if (m_from_plc_mem.ReadFromShareMem(m_from_plc_packed_data.data())) {
        // 将数据包拆分为各个工作站的数据
        UnpackAllWorkstationData();
        QLOG_DEBUG() << "成功从PLC读取所有工作站数据";
    } else {
        QLOG_ERROR() << "从PLC读取数据失败";
    }
}

void SharedMemoryManager::WriteToPLC() {
    QMutexLocker locker(&m_data_mutex);
    // 将所有工作站数据拼接成完整数据包
    PackAllWorkstationData();

    // 将完整数据包写入共享内存
    if (m_to_plc_mem.WriteToShareMem(m_to_plc_packed_data.data()))
        QLOG_DEBUG() << "成功向PLC写入所有工作站数据";
    else
        QLOG_ERROR() << "向PLC写入数据失败";
}

From_PLC_DATA &SharedMemoryManager::GetFromPLCData(int workstationIndex) {
    QMutexLocker locker(&m_data_mutex);
    if (workstationIndex > 0 && workstationIndex <= m_from_plc_data_list.size()) {
        return m_from_plc_data_list[workstationIndex - 1];
    }
    static From_PLC_DATA invalid_data = {0};
    QLOG_ERROR() << "无效的工作站索引:" << workstationIndex;
    return invalid_data;
}

TO_PLC_DATA &SharedMemoryManager::GetToPLCData(int workstationIndex) {
    // 注意：这里返回临时缓冲区的引用，线程修改的是临时缓冲区
    QMutexLocker locker(&m_data_mutex);
    if (workstationIndex > 0 && workstationIndex <= m_to_plc_data_list_temp.size()) {
        return m_to_plc_data_list_temp[workstationIndex - 1];
    }
    static TO_PLC_DATA invalid_data = {0};
    QLOG_ERROR() << "无效的工作站索引:" << workstationIndex;
    return invalid_data;
}

QVector<From_PLC_DATA> &SharedMemoryManager::GetRawFromPLCData() {
    return m_from_plc_data_list;
}

QVector<TO_PLC_DATA> &SharedMemoryManager::GetRawToPLCData() {
    return m_to_plc_data_list;
}

void SharedMemoryManager::UpdateWorkstationData(int workstationIndex, const From_PLC_DATA &fromData,
                                                const TO_PLC_DATA &toData) {
    QMutexLocker locker(&m_data_mutex);
    if (workstationIndex > 0 && workstationIndex <= m_workstationList.size()) {
        m_from_plc_data_list[workstationIndex - 1] = fromData;
        m_to_plc_data_list_temp[workstationIndex - 1] = toData; // 更新到临时缓冲区
        // 同时提交到稳定缓冲区
        m_to_plc_data_list[workstationIndex - 1] = toData;
    } else
        QLOG_ERROR() << "更新工作站数据失败，无效的索引:" << workstationIndex;
}

void SharedMemoryManager::CommitWorkstationData(int workstationIndex) {
    QMutexLocker locker(&m_commit_mutex);
    if (workstationIndex > 0 && workstationIndex <= m_workstationList.size()) {
        // 将临时缓冲区的数据提交到稳定缓冲区
        m_to_plc_data_list[workstationIndex - 1] = m_to_plc_data_list_temp[workstationIndex - 1];
        QLOG_DEBUG() << "工作站" << workstationIndex << "数据已提交到稳定缓冲区";
    } else {
        QLOG_ERROR() << "提交工作站数据失败，无效的索引:" << workstationIndex;
    }
}

void SharedMemoryManager::PackAllWorkstationData() {
    char *to_data_ptr = m_to_plc_packed_data.data();
    int to_data_size = sizeof(TO_PLC_DATA);

    for (int i = 0; i < m_to_plc_data_list.size(); ++i) {
        memcpy(to_data_ptr + i * to_data_size, &m_to_plc_data_list[i], to_data_size);
    }
}

void SharedMemoryManager::UnpackAllWorkstationData() {
    const char *from_data_ptr = m_from_plc_packed_data.constData();
    int from_data_size = sizeof(From_PLC_DATA);

    for (int i = 0; i < m_from_plc_data_list.size(); ++i)
        memcpy(&m_from_plc_data_list[i], from_data_ptr + i * from_data_size, from_data_size);
}