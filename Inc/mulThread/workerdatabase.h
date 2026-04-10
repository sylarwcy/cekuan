#ifndef WORKERDATABASE_H
#define WORKERDATABASE_H

#include <QObject>
#include "workStationDataStructure.h"
#include "dbconnthread.h"
#include "dbhead.h"

class WorkerDatabase : public QObject
{
    Q_OBJECT
public:
    explicit WorkerDatabase(QObject *parent = nullptr);
    ~WorkerDatabase();
    void init(const WorkStation_DATA& paramData);

    DbConnThread *m_dbConn_local;
    DbInfo m_dbInfo;
    QString m_dbType;              // 数据库类型
    QString m_dbTableName;         // 数据库表名
    QString m_dbKeyName;   // 数据库表的字段名

public slots:
    void connectDB();


signals:
    void sig_finish_open_db();
};

#endif // WORKERDATABASE_H
