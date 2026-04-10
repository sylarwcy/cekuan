#ifndef DBHTTPTHREAD_H
#define DBHTTPTHREAD_H

#include "dbhead.h"
#include <QtNetwork>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#include <QtScript>
#endif

class DbHttpThread : public QObject
{
    Q_OBJECT
public:
    //获取表格字段集合
    static QMap<QString, QString> getTables1();
    static QMap<QString, QString> getTables2();

    explicit DbHttpThread(QObject *parent = 0);
    ~DbHttpThread();

private:
    QMutex mutex;

    //请求地址
    QString url;
    //网络通信服务
    QNetworkAccessManager *manager;

    //队列标识符,默认用表名
    QStringList tags;
    //cmd语句队列
    QStringList cmds;

    //表信息集合 key=表名 value=字段1,字段2
    QMap<QString, QString> tables;

private:
    //获取执行用时
    QElapsedTimer time;
    QString getUseTime();

private slots:
    //请求完成
    void finished(QNetworkReply *reply);

signals:
    //打印信息信号
    void debug(const QString &msg);
    //失败错误信号
    void error(const QString &msg);
    //返回数据信号
    void receiveData(const QString &tag, const QStringList &data, int msec);

public slots:
    //设置请求地址
    void setUrl(const QString &url);
    QString getUrl() const;

    //设置请求表名+字段名
    void setTables(const QMap<QString, QString> &tables);

    //处理cmd语句
    void takeCmd();
    //指定表和字段查询所有数据
    void select(const QString &tableName, const QString &columnName, bool append = true);
    //直接请求数据
    void post(const QString &data, const QString &userName = QString(),
              const QString &userPwd = QString(), const QString &userKey = QString());
};

#endif // DBHTTPTHREAD_H
