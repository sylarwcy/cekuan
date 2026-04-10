#ifndef DBCOUNTTHREAD_H
#define DBCOUNTTHREAD_H

#include "dbhead.h"

//计算复合条件的记录总行数
class DbCountThread : public QThread
{
    Q_OBJECT
public:
    explicit DbCountThread(QObject *parent = 0);

private:
    QString connName;   //数据库连接名称
    QString sql;        //要执行的查询语句

protected:
    void run();

signals:
    //收到总行数
    void receiveCount(quint32 count, double msec);

public slots:
    //设置数据库连接名称
    void setConnName(const QString &connName);
    //设置要执行的查询语句
    void setSql(const QString &sql);
    //查询行数
    void select();
};

#endif // DBCOUNTTHREAD_H
