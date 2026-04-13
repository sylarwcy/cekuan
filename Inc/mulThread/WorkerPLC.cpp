#include "WorkerPLC.h"
#include "MyApplication.h"
#include "QsLog.h" // 假设你使用的是 QsLog 记录日志

WorkerPLC::WorkerPLC(QObject *parent)
    : QObject(parent) {
}

WorkerPLC::~WorkerPLC() {
}

void WorkerPLC::onReceiveDataToPLC(double widthValue, double centerOffset, int statusFlag) {
    try {
        MyApplication *pApp = (MyApplication *) qApp;
        if (!pApp || !pApp->m_mem_manager) {
            QLOG_ERROR() << "Memory manager is null!";
            return;
        }

        // 1. (可选) 更新PLC传来的最新状态，看你的业务逻辑是否需要在写之前先读一次
        // pApp->m_mem_manager->ReadFromPLC();

        // 2. 获取对应该工位的共享内存数据引用
        // 注意：这里的字段名 (width_value, center_offset) 需要替换为你实际结构体里定义的变量名
        auto& plcDataTo = pApp->m_mem_manager->GetToPLCData(m_serialNumber);

        // 3. 赋值业务数据
        if (statusFlag == 4) {
            // 算法或相机异常
            plcDataTo.has_steel_plate = 0;      // 认为当前无有效钢板
            plcDataTo.has_cam_broken = 1;       // 相机/系统故障标志
            plcDataTo.recog_finished_flag = 4;  // 状态 4
            plcDataTo.bak1 = 0;                 // 清空宽度
            plcDataTo.bak2 = 0;                 // 清空跑偏量
        }
        else {
            // 正常测量到了数据 (包含正常、超上限、超下限)
            plcDataTo.has_steel_plate = 1;      // 有钢板
            plcDataTo.has_cam_broken = 0;       // 系统正常
            plcDataTo.recog_finished_flag = statusFlag; // 写入具体状态(1,2,3)

            // 注意：你实际的结构体里如果是用 bak1/bak2 存数据，或者有专用的 width_value 字段
            // 这里为了演示，假设直接存入你定义的字段或浮点备用字段
            // plcDataTo.width_value = widthValue;
            // plcDataTo.center_offset = centerOffset;

            // 如果 PLC 结构体只能存整数（常见做法：乘 10 变整数下发）
            plcDataTo.bak1 = static_cast<int>(widthValue * 10.0);
            plcDataTo.bak2 = static_cast<int>(centerOffset * 10.0);
        }

        // 4. 提交数据并写入到底层(PLC/共享内存)
        pApp->m_mem_manager->CommitWorkstationData(m_serialNumber);
        pApp->m_mem_manager->WriteToPLC();

        // QLOG_INFO() << "[WorkerPLC] Sent Success: Width=" << widthValue;

    } catch (const std::exception &e) {
        QLOG_ERROR() << "WorkerPLC processing error:" << e.what();
    } catch (...) {
        QLOG_ERROR() << "WorkerPLC unknown error during send width result.";
    }
}

void WorkerPLC::onSendErrorState(const QString& errorMsg) {
    try {
        MyApplication *pApp = (MyApplication *) qApp;
        if (!pApp || !pApp->m_mem_manager) return;

        auto& plcDataTo = pApp->m_mem_manager->GetToPLCData(m_serialNumber);

        // 发生异常时，给 PLC 发送异常标志
        plcDataTo.recog_finished_flag = 2; // 假设 2 代表处理失败/异常
        plcDataTo.has_steel_plate = 0;

        // 如果异常信息里包含"Camera"字眼，可以给PLC报相机故障
        if(errorMsg.contains("Camera", Qt::CaseInsensitive)){
             plcDataTo.has_cam_broken = 1;
        }

        pApp->m_mem_manager->CommitWorkstationData(m_serialNumber);
        pApp->m_mem_manager->WriteToPLC();

    } catch (...) {
        QLOG_ERROR() << "WorkerPLC unknown error during send error state.";
    }
}
