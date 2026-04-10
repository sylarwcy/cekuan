#ifndef DBCLEANTHREAD_H
#define DBCLEANTHREAD_H

/**
 * 自动清理数据线程类 作者:feiyangqingyun(QQ:517216493) 2017-1-15
 * 1. 可设置要清理的对应数据库连接名称和表名
 * 2. 可设置条件字段
 * 3. 可设置排序字段
 * 4. 可设置最大保留的记录数
 * 5. 可设置执行自动清理的间隔
 * 6. 后期支持多个数据库和多个表
 * 7. 建议条件字段用数字类型的主键,速度极快
 * 8. 增加统计用字段名称设置
 * 9. 增加自动清理文件夹,超过大小自动删除文件夹中早期文件
 */

#include "dbhelper.h"

class DbCleanThread : public QThread
{
    Q_OBJECT
public:
    explicit DbCleanThread(QObject *parent = 0);
    ~DbCleanThread();

protected:
    void run();

private:
    volatile bool stopped;      //停止线程标志位

    bool dbOk;                  //数据库是否正常
    int errorCount;             //错误次数
    QSqlDatabase database;      //数据库连接对象
    QDateTime lastCheckTime;    //最后一次测试数据库连接时间
    QDateTime lastCleanTime;    //最后一次清理时间

    QString tableName;          //表名
    QString countName;          //统计记录数字段
    QString whereColumnName;    //条件字段
    QString orderSql;           //排序字段及排序规则

    int maxCount;               //最大记录数
    bool checkConn;             //是否检查连接
    int checkInterval;          //检查数据库连接间隔
    int cleanInterval;          //清理间隔

    QString dirPath;            //监听的文件夹
    QStringList dirFileFilter;  //监听的文件夹过滤类型
    int dirMaxSize;             //最大大小,单位MB

    QString dbFlag;             //数据库标识
    DbType dbType;              //数据库类型
    DbInfo dbInfo;              //数据库连接信息

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

public slots:
    //设置表名+统计总数字段+条件字段+排序字段
    void setTableName(const QString &tableName);
    void setCountName(const QString &countName);
    void setWhereColumnName(const QString &whereColumnName);
    void setOrderSql(const QString &orderSql);

    //设置最大保留的记录数+执行自动清理的间隔
    void setMaxCount(int maxCount);
    void setCleanInterval(int cleanInterval);

    //综合设置
    void setCleanInfo(const QString &tableName, const QString &countName,
                      const QString &whereColumnName, const QString &orderSql,
                      int maxCount, int cleanInterval);

    //设置监听的文件夹+监听的文件夹的类型+监听文件夹最大大小
    void setDirPath(const QString &dirPath);
    void setDirFileFilter(const QStringList &dirFileFilter);
    void setDirMaxSize(int dirMaxSize);

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

    //清理数据库记录
    int getCount();
    QStringList getCleanValue(int cleanCount);
    void cleanData();

    //清理文件夹
    void cleanPath();
    //删除指定文件夹
    void deletePath(const QString &path);
};

#endif // DBCLEANTHREAD_H
