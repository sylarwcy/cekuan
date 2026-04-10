#ifndef DBCONNTHREAD_H
#define DBCONNTHREAD_H

/**
 * 数据库通信管理线程类 作者:feiyangqingyun(QQ:517216493) 2021-4-25
 * 1. 可设置数据库类型,支持多种数据库类型
 * 2. 数据库类型包括但不限于odbc+sqlite+mysql+postgresql+sqlserver+oracle+人大金仓等
 * 3. 可设置数据库连接信息包括主机地址+用户信息等
 * 4. 具有自动重连机制,可设置是否检查连接以及检查间隔
 * 5. 支持单条sql语句队列,一般用于查询返回数据,每次插入一条执行一条
 * 6. 支持多条sql语句队列,一般用于远程提交数据,每次插入一条执行多条
 * 7. 支持批量sql语句队列,一般用于批量更新数据,每次插入多条执行多条
 * 8. 可设置队列最大数量,限定排队处理的sql语句集合
 * 9. 通过信号发出 打印信息+错误信息+查询结果
 */

#include "dbhelper.h"

class DbConnThread : public QThread
{
    Q_OBJECT
public:
    explicit DbConnThread(QObject *parent = 0);
    ~DbConnThread();

protected:
    void run();

private:
    QMutex mutex;               //锁对象
    volatile bool stopped;      //停止线程标志位

    bool dbOk;                  //数据库是否正常
    int errorCount;             //错误次数
    QSqlDatabase database;      //数据库连接对象
    QDateTime lastCheckTime;    //最后一次测试数据库连接时间

    int maxCount;               //sql语句队列最大数量
    bool checkConn;             //是否检查连接
    int checkInterval;          //检查数据库连接间隔

    QString dbFlag;             //数据库标识
    DbType dbType;              //数据库类型
    DbInfo dbInfo;              //数据库连接信息

    //单条sql语句队列,一般用于查询返回数据,每次插入一条执行一条
    QStringList sqlTag;
    QStringList sqlSelect;

    //多条sql语句队列,一般用于远程提交数据,每次插入一条执行多条
    QStringList sqlPlus;
    //批量sql语句队列,一般用于批量更新数据,每次插入多条执行多条
    QList<QStringList> sqlUpdate;

private:
    //获取执行用时
    QElapsedTimer time;
    QString getUseTime();

signals:
    //打印信息
    void debug(const QString &msg);
    //失败错误
    void error(const QString &msg);

    //收到总行数
    void receiveCount(const QString &tag, int count, int msec);
    //返回数据
    void receiveData(const QString &tag, const QStringList &data, int msec);

public slots:
    //设置数据库连接信息
    void setDbFlag(const QString &dbFlag);
    void setConnInfo(const DbType &dbType, const DbInfo &dbInfo);

    //设置是否检查数据库连接+检查间隔
    void setCheckConn(bool checkConn);
    void setCheckInterval(int checkInterval);

    //获取数据库是否正常+数据库对象
    bool getOk();
    QSqlDatabase getDatabase();

    //打开+检查+关闭 数据库
    bool openDb();
    bool checkDb();
    void closeDb();

    //停止线程
    void stop();

    //单条执行sql语句
    void execSqlSelect();
    //多条执行sql语句
    void execSqlPlus();
    //批量执行sql语句
    void execSqlUpdate();

    //指定表和字段查询所有数据
    void select(const QString &tableName, const QString &columnName, bool append = true);
    //查询记录行数
    void selectCount(const QString &tableName, const QString &countName, const QString &whereSql);

    //传入sql语句初始化数据库
    void initDb(const QStringList &sqls);
    //添加sql语句,只需要执行不需要返回结果
    void append(const QString &sql);

    //插入多条sql语句
    void append(const QStringList &sql);
    //指定数量插入重复的sql语句
    void append(int count, const QString &sql);
};

#endif // DBCONNTHREAD_H
