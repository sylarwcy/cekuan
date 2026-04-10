#include "dbcleanthread.h"

DbCleanThread::DbCleanThread(QObject *parent) : QThread(parent)
{
    stopped = false;

    dbOk = false;
    errorCount = 0;
    lastCheckTime = QDateTime::currentDateTime();
    lastCleanTime = QDateTime::currentDateTime();

    //rowid 是sqlite数据库内置的自增字段
    tableName = "LogInfo";
    countName = "*";
    whereColumnName = "rowid";
    orderSql = "rowid asc";

    maxCount = 10000;
    checkConn = true;
    checkInterval = 30;
    cleanInterval = 30 * 60;

    dirPath = "";
    dirFileFilter = QStringList("*.jpg");
    dirMaxSize = 1024;

    dbFlag = "本地";
    dbType = DbType_Sqlite;
    DbHelper::debugDbInfo();
}

DbCleanThread::~DbCleanThread()
{
    this->stop();
    this->wait();
}

void DbCleanThread::run()
{
    //如果还没打开数据库则先打开数据库
    if (!dbOk) {
        openDb();
        msleep(100);
    }

    //sqlite数据库属于本地文件数据库永远不需要校验
    if (dbType == DbType_Sqlite) {
        checkConn = false;
    }

    while (!stopped) {
        //定时执行一次校验数据库连接是否正常
        QDateTime now = QDateTime::currentDateTime();
        if (checkConn && lastCheckTime.secsTo(now) >= checkInterval) {
            checkDb();
            lastCheckTime = now;
            continue;
        }

        //如果数据库连接正常则处理数据,不正常则重连数据库
        if (!dbOk) {
            //可以修改最大的错误数
            if (errorCount >= 2) {
                closeDb();
                msleep(3000);
                openDb();
                msleep(3000);
            }
        } else {
            //定时执行一次校验时间是否到了该清理的时候
            if (lastCleanTime.secsTo(now) >= cleanInterval) {
                cleanData();
                cleanPath();
                lastCleanTime = now;
            }
        }

        msleep(10);
    }

    stopped = false;
}

QString DbCleanThread::getUseTime()
{
    return QString::number((double)time.elapsed() / 1000, 'f', 3);
}

void DbCleanThread::setTableName(const QString &tableName)
{
    this->tableName = tableName;
}

void DbCleanThread::setCountName(const QString &countName)
{
    this->countName = countName;
}

void DbCleanThread::setWhereColumnName(const QString &whereColumnName)
{
    this->whereColumnName = whereColumnName;
}

void DbCleanThread::setOrderSql(const QString &orderSql)
{
    this->orderSql = orderSql;
}

void DbCleanThread::setMaxCount(int maxCount)
{
    this->maxCount = maxCount;
}

void DbCleanThread::setCleanInterval(int cleanInterval)
{
    this->cleanInterval = cleanInterval;
}

void DbCleanThread::setCleanInfo(const QString &tableName, const QString &countName,
                                 const QString &whereColumnName, const QString &orderSql,
                                 int maxCount, int cleanInterval)
{
    this->tableName = tableName;
    this->countName = countName;
    this->whereColumnName = whereColumnName;
    this->orderSql = orderSql;
    this->maxCount = maxCount;
    this->cleanInterval = cleanInterval;
}

void DbCleanThread::setDirPath(const QString &dirPath)
{
    this->dirPath = dirPath;
}

void DbCleanThread::setDirFileFilter(const QStringList &dirFileFilter)
{
    this->dirFileFilter = dirFileFilter;
}

void DbCleanThread::setDirMaxSize(int dirMaxSize)
{
    this->dirMaxSize = dirMaxSize;
}

void DbCleanThread::setDbFlag(const QString &dbFlag)
{
    this->dbFlag = dbFlag;
}

void DbCleanThread::setConnInfo(const DbType &dbType, const DbInfo &dbInfo)
{
    this->dbType = dbType;
    this->dbInfo = dbInfo;
}

void DbCleanThread::setCheckConn(bool checkConn)
{
    this->checkConn = checkConn;
}

void DbCleanThread::setCheckInterval(int checkInterval)
{
    if (checkInterval > 5 && this->checkInterval != checkInterval) {
        this->checkInterval = checkInterval;
    }
}

bool DbCleanThread::getOk()
{
    return dbOk;
}

QSqlDatabase DbCleanThread::getDatabase()
{
    return database;
}

bool DbCleanThread::openDb()
{
    errorCount = 0;

    //初始化数据库
    QString type = DbHelper::getDbType(dbType);
    if (!DbHelper::initDatabase(false, type, database, dbInfo)) {
        return false;
    }

    dbOk = database.open();
    if (dbOk) {
        emit debug(QString("打开%1数据库成功").arg(dbFlag));
    } else {
        QString text = database.lastError().text();
        emit error(QString("打开%1数据库失败,原因: %2").arg(dbFlag).arg(text));
        qDebug() << TIMEMS << this->objectName() << text;
    }

    return dbOk;
}

bool DbCleanThread::checkDb()
{
    time.restart();

    QString sql = "select 1";
    QSqlQuery query(database);
    dbOk = query.exec(sql);

    QString msg = QString("(连接%1/用时 %2 秒)").arg(dbOk ? "正常" : "异常").arg(getUseTime());
    if (dbOk) {
        errorCount = 0;
        emit debug(QString("检查%1数据库连接状态%2").arg(dbFlag).arg(msg));
    } else {
        errorCount++;
        QString text = database.lastError().text();
        emit error(QString("检查%1数据库连接状态%2,原因: %3").arg(dbFlag).arg(msg).arg(text));
        qDebug() << TIMEMS << this->objectName() << text;
    }

    return dbOk;
}

void DbCleanThread::closeDb()
{
    errorCount = 0;
    database.close();
    QSqlDatabase::removeDatabase(dbInfo.connName);
    dbOk = false;
    emit debug(QString("关闭%1数据库成功").arg(dbFlag));
}

void DbCleanThread::stop()
{
    stopped = true;
}

int DbCleanThread::getCount()
{
    int count = -1;
    if (!dbOk) {
        return count;
    }

    time.restart();
    QString sql = QString("select count(%1) from %2").arg(countName).arg(tableName);
    QSqlQuery query(database);
    if (query.exec(sql)) {
        if (query.next()) {
            count = query.value(0).toInt();
            QString msg = QString("(共 %1 条/用时 %2 秒)").arg(count).arg(getUseTime());
            emit debug(QString("%1数据库获取记录行数%2").arg(dbFlag).arg(msg));
            emit receiveCount(tableName, count, time.elapsed());
        }
    }

    return count;
}

QStringList DbCleanThread::getCleanValue(int cleanCount)
{
    QStringList list;
    if (!dbOk) {
        return list;
    }

    QSqlQuery query(database);
    QString sql = QString("select %2 from %1 order by %3 limit %4").arg(tableName).arg(whereColumnName).arg(orderSql).arg(cleanCount);
    if (query.exec(sql)) {
        while (query.next()) {
            list << query.value(0).toString();
        }
    }

    return list;
}

void DbCleanThread::cleanData()
{
    if (!dbOk) {
        return;
    }

    //首先查找总记录数,如果总记录数超过限制,则将超出的部分按照字段排序进行删除
    int count = getCount();
    int cleanCount = (count - maxCount);
    if (cleanCount < 100) {
        return;
    }

    time.restart();

    //每次最大清理1000条数据
    cleanCount = cleanCount > 1000 ? 1000 : cleanCount;
    //将要删除的数据指定字段集合查询出来
    QStringList list = getCleanValue(cleanCount);
    if (list.count() == 0) {
        return;
    }

    //删除数据
    QSqlQuery query(database);
    QString sql = QString("delete from %1 where %2 in(%3)").arg(tableName).arg(whereColumnName).arg(list.join(","));
    dbOk = query.exec(sql);
    //qDebug() << TIMEMS << sql;

    QString msg = QString("(共 %1 条/用时 %2 秒)").arg(cleanCount).arg(getUseTime());
    if (dbOk) {
        emit debug(QString("%1数据库清理数据成功%2").arg(dbFlag).arg(msg));
    } else {
        QString text = database.lastError().text();
        emit error(QString("%1数据库清理数据失败%2,原因: %3").arg(dbFlag).arg(msg).arg(text));
        qDebug() << TIMEMS << this->objectName() << text;
    }
}

void DbCleanThread::cleanPath()
{
    if (dirPath.isEmpty()) {
        return;
    }

    //找出该文件夹下的所有文件夹
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    //按照目录查找,过滤文件夹,按照文件名称排序
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    QStringList list = dir.entryList();

    //遍历所有目录,对所有文件大小相加得到总大小,文件就在文件夹下,不会再有子目录
    qint64 size = 0;
    foreach (QString path, list) {
        QDir d(dirPath + "/" + path);
        QFileInfoList infos = d.entryInfoList(dirFileFilter);
        foreach (QFileInfo info, infos) {
            size += info.size();
        }

        //转化成MB,超过预定大小自动删除第一个文件夹,跳出循环无需继续判断
        int sizeMB = size / (1024 * 1024);
        if (sizeMB >= dirMaxSize) {
            //删除该目录下的所有文件
            QString path = dirPath + "/" + list.at(0);
            QDir dir(path);
            dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
            QStringList files = dir.entryList();
            foreach (QString file, files) {
                dir.remove(file);
                qDebug() << TIMEMS << "删除文件" << path << file;
            }

            //删除文件夹本身
            dir.rmdir(path);
            QString msg = QString("(共 %1 个文件/用时 %2 秒)").arg(files.count()).arg(getUseTime());
            emit debug(QString("%1数据库自动清理目录成功%2").arg(dbFlag).arg(msg));
            break;
        }
    }
}

void DbCleanThread::deletePath(const QString &path)
{
    QDir dir(path);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //这个方法可以递归彻底删除文件夹 不管文件夹下是否有文件 比较暴力
    //此方法慎用 必须指定明确的文件夹 不然删除默认的目录哭都来不及 网上多个人中招
    dir.removeRecursively();
#else
    //循环遍历删除文件及文件夹
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo fi, fileList) {
        if (fi.isFile()) {
            fi.dir().remove(fi.fileName());
        } else {
            deletePath(fi.absoluteFilePath());
            dir.rmpath(fi.absoluteFilePath());
        }
    }
    //最后删除最外层的目录
    dir.rmpath(path);
#endif
}
