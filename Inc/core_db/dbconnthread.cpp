#include "dbconnthread.h"

DbConnThread::DbConnThread(QObject *parent) : QThread(parent)
{
    stopped = false;

    dbOk = false;
    errorCount = 0;
    lastCheckTime = QDateTime::currentDateTime();

    maxCount = 100;
    checkConn = true;
    checkInterval = 30;

    dbFlag = "本地";
    dbType = DbType_Sqlite;
    DbHelper::debugDbInfo();
}

DbConnThread::~DbConnThread()
{
    this->stop();
    this->wait();
}

void DbConnThread::run()
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
            execSqlSelect();
            execSqlPlus();
            execSqlUpdate();
            msleep(10);
        }
    }

    stopped = false;
}

QString DbConnThread::getUseTime()
{
    return QString::number((double)time.elapsed() / 1000, 'f', 3);
}

void DbConnThread::setDbFlag(const QString &dbFlag)
{
    this->dbFlag = dbFlag;
}

void DbConnThread::setConnInfo(const DbType &dbType, const DbInfo &dbInfo)
{
    this->dbType = dbType;
    this->dbInfo = dbInfo;
}

void DbConnThread::setCheckConn(bool checkConn)
{
    this->checkConn = checkConn;
}

void DbConnThread::setCheckInterval(int checkInterval)
{
    if (checkInterval > 5 && this->checkInterval != checkInterval) {
        this->checkInterval = checkInterval;
    }
}

bool DbConnThread::getOk()
{
    return dbOk;
}

QSqlDatabase DbConnThread::getDatabase()
{
    return database;
}

bool DbConnThread::openDb()
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

bool DbConnThread::checkDb()
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
        emit error(QString("检查%1数据库连接状态%2").arg(dbFlag).arg(msg));
        qDebug() << TIMEMS << this->objectName() << database.lastError();
    }

    return dbOk;
}

void DbConnThread::closeDb()
{
    errorCount = 0;
    database.close();
    QSqlDatabase::removeDatabase(dbInfo.connName);
    dbOk = false;
    emit debug(QString("关闭%1数据库成功").arg(dbFlag));
}

void DbConnThread::stop()
{
    stopped = true;
}

void DbConnThread::execSqlSelect()
{
    int count = sqlSelect.count();
    if (!dbOk || count == 0) {
        return;
    }

    //取出队列sql语句执行,从最前面开始取
    time.restart();
    mutex.lock();
    QString tag = sqlTag.takeFirst();
    QString sql = sqlSelect.takeFirst();
    mutex.unlock();

    QSqlQuery query(database);
    if (query.exec(sql)) {
        QStringList data;
        while (query.next()) {
            int count = query.record().count();
            for (int i = 0; i < count; i++) {
                data << query.value(i).toString();
            }
        }

        if (data.count() > 0) {
            //统计行数的按照另外的信号发出去
            if (sql.contains("count(")) {
                int count = data.first().toInt();
                QString msg = QString("(共 %1 条/用时 %2 秒)").arg(count).arg(getUseTime());
                emit debug(QString("%1数据库获取记录行数%2").arg(dbFlag).arg(msg));
                emit receiveCount(tag, count, time.elapsed());
            } else {
                emit receiveData(tag, data, time.elapsed());
            }
        }
    }
}

void DbConnThread::execSqlPlus()
{
    int count = sqlPlus.count();
    if (!dbOk || count == 0) {
        return;
    }

    time.restart();
    mutex.lock();
    QStringList sqls = sqlPlus;
    sqlPlus.clear();
    mutex.unlock();

    //开启数据库事务加快速度
    database.transaction();

    //逐个执行sql语句
    foreach (QString sql, sqls) {
        QSqlQuery query(database);
        query.exec(sql);
    }

    QString msg = QString("(共 %1 条/用时 %2 秒)").arg(count).arg(getUseTime());
    if (database.commit()) {
        emit debug(QString("%1数据库实时更新数据%2").arg(dbFlag).arg(msg));
    } else {
        //提交失败则需要回滚事务
        database.rollback();
        dbOk = false;
        QString text = database.lastError().text();
        emit error(QString("%1数据库实时更新数据提交事务失败%2,原因: %3").arg(dbFlag).arg(msg).arg(text));
        qDebug() << TIMEMS << this->objectName() << text;
    }
}

void DbConnThread::execSqlUpdate()
{
    int count = sqlUpdate.count();
    if (!dbOk || count == 0) {
        return;
    }

    time.restart();
    mutex.lock();
    QStringList sqls = sqlUpdate.takeFirst();
    mutex.unlock();

    //开启数据库事务加快速度
    database.transaction();

    //逐个执行sql语句
    count = sqls.count();
    foreach (QString sql, sqls) {
        QSqlQuery query(database);
        query.exec(sql);
    }

    QString msg = QString("(共 %1 条/用时 %2 秒)").arg(count).arg(getUseTime());
    if (database.commit()) {
        emit debug(QString("%1数据库批量更新数据%2").arg(dbFlag).arg(msg));
    } else {
        //提交失败则需要回滚事务
        database.rollback();
        dbOk = false;
        QString text = database.lastError().text();
        emit error(QString("%1数据库批量更新数据提交事务失败%2,原因: %3").arg(dbFlag).arg(msg).arg(text));
        qDebug() << TIMEMS << this->objectName() << text;
    }
}

void DbConnThread::select(const QString &tableName, const QString &columnName, bool append)
{
    //超过队列中最大数量限制则无需处理
    if (sqlSelect.count() >= maxCount) {
        return;
    }

    //可以自行调整sql语句比如增加排序等
    QString sql = QString("select %1 from %2").arg(columnName).arg(tableName);
    //表字段名和关键字冲突需要替换
    if (dbType != DbType_MySql) {
        DbHelper::replaceColumnName(sql, "current");
        DbHelper::replaceColumnName(sql, "plan");
    }

    mutex.lock();
    if (append) {
        sqlTag << tableName;
        sqlSelect << sql;
    } else {
        sqlTag.insert(0, tableName);
        sqlSelect.insert(0, sql);
    }
    mutex.unlock();
}

void DbConnThread::selectCount(const QString &tableName, const QString &countName, const QString &whereSql)
{
    QString sql = QString("select count(%1) from %2 %3").arg(countName).arg(tableName).arg(whereSql);
    sqlTag << tableName;
    sqlSelect << sql;
}

void DbConnThread::initDb(const QStringList &sqls)
{
    //保证至少有一条数据
    int count = sqls.count();
    if (dbOk && count > 1) {
        time.restart();

        //开启数据库事务加快速度
        database.transaction();

        //严格规定第一条sql语句为清空该表所有数据
        QSqlQuery query(database);
        query.exec(sqls.first());

        //新建初始值数据
        for (int i = 1; i < count; i++) {
            QSqlQuery query(database);
            query.exec(sqls.at(i));
        }

        QString msg = QString("(共 %1 条/用时 %2 秒)").arg(count).arg(getUseTime());
        if (database.commit()) {
            emit debug(QString("%1数据库初始化数据%2").arg(dbFlag).arg(msg));
        } else {
            //提交失败则需要回滚事务
            database.rollback();
            dbOk = false;
            QString text = database.lastError().text();
            emit error(QString("%1数据库初始化数据提交事务失败%2,原因: %3").arg(dbFlag).arg(msg).arg(text));
            qDebug() << TIMEMS << this->objectName() << text;
        }
    }
}

void DbConnThread::append(const QString &sql)
{
    //超过队列中最大数量限制则无需处理
    if (sqlPlus.count() >= maxCount) {
        return;
    }

    mutex.lock();
    sqlPlus << sql;
    mutex.unlock();
}

void DbConnThread::append(const QStringList &sql)
{
    //超过队列中最大数量限制则无需处理
    if (sqlUpdate.count() >= maxCount) {
        return;
    }

    mutex.lock();
    sqlUpdate << sql;
    mutex.unlock();
}

void DbConnThread::append(int count, const QString &sql)
{
    QStringList sqls;
    for (int i = 0; i < count; ++i) {
        sqls << sql;
    }

    append(sqls);
}
