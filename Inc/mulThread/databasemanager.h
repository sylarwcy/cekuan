//
// Created by sylar on 26-6-1.
//
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QDebug>

struct PlateRecord {
    QString plateID{"001"};
    double length{0};
    double thickness{0};
    double targetWidth{0};
    double avgWidth{0};
    double maxWidth{0};
    double minWidth{0};
};

class DatabaseManager {
public:
    // 获取单例全局唯一实例
    static DatabaseManager& instance() {
        static DatabaseManager s_instance;
        return s_instance;
    }

    // 初始化数据库连接（创建表）
    bool initDatabase(const QString& dbName = "history_data.db");

    // 存储单条钢板历史记录
    bool saveRecordToDb(const PlateRecord& record);

    // 获取最近的 N 条记录
    QList<PlateRecord> loadHistoryFromDb(int limit = 5);

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QString m_connectionName{"measure_db_conn"};
};

#endif // DATABASEMANAGER_H