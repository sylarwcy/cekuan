#include "dbcountthread.h"

DbCountThread::DbCountThread(QObject *parent) : QThread(parent)
{
    connName = "qt_sql_default_connection";
    sql = "select 1";

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void DbCountThread::run()
{
    select();
}

void DbCountThread::setConnName(const QString &connName)
{
    this->connName = connName;
}

void DbCountThread::setSql(const QString &sql)
{
    this->sql = sql;
}

void DbCountThread::select()
{
    //计算用时
    QTime time;
    time.start();

    int count = 0;
    QSqlQuery query(QSqlDatabase::database(connName));
    if (query.exec(sql)) {
        if (query.next()) {
            count = query.value(0).toUInt();
        }
    }

    emit receiveCount(count, time.elapsed());
}
