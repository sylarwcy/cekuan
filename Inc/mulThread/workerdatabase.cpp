#include "workerdatabase.h"
#include <QSqlQuery>
#include <QDateTime>

#include "MyApplication.h"
#include "appconfig.h"


////////////////////////////////////////////////////////////////
WorkerDatabase::WorkerDatabase(QObject *parent)
    : QObject{parent}{

}

WorkerDatabase::~WorkerDatabase(){
    delete m_dbConn_local;
}

void WorkerDatabase::connectDB()
{
    m_dbConn_local = new DbConnThread(this);
    m_dbConn_local->setConnInfo(DbHelper::getDbType(m_dbType), m_dbInfo);
    if (m_dbConn_local->openDb()){
        m_dbConn_local->start();
        QLOG_INFO()<<QString(u8"SQLServer 数据库: %1 连接成功！").arg(m_dbInfo.dbName);
    }
    else{
        QString error = m_dbConn_local->getDatabase().lastError().text();
        qDebug()<<error;
        QLOG_ERROR()<<QString(u8"SQLServer 数据库: %1 连接失败！错误信息：%2").arg(m_dbInfo.dbName).arg(error);
    }

    emit sig_finish_open_db();
}

void WorkerDatabase::init(const WorkStation_DATA& paramData) {
    m_dbInfo.connName = paramData.dbConnName;
    m_dbInfo.dbName = paramData.dbName;
    m_dbInfo.hostName = paramData.dbHostName;
    m_dbInfo.hostPort = paramData.dbHostPort;
    m_dbInfo.userName = paramData.dbUserName;
    m_dbInfo.userPwd = paramData.dbUserPwd;

    m_dbType = paramData.dbType;
    m_dbTableName = paramData.dbTableName;
    m_dbKeyName = paramData.dbKeyName;
}
//各种数据库函数处理

