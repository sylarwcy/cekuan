#include "dbhelper.h"
#include "qtcpsocket.h"
#include "qregexp.h"

void DbHelper::debugDbInfo()
{
    static bool isDebug = false;
    if (!isDebug) {
        isDebug = true;
        //打印支持的数据库,可以自行增加其他打印信息
        qDebug() << TIMEMS << "QSqlDatabase drivers" << QSqlDatabase::database().drivers();
    }
}

void DbHelper::appendMsg(QTextEdit *txtMain, int &maxCount, int &currentCount, int type, const QString &data, bool clear)
{
    if (currentCount >= maxCount) {
        txtMain->clear();
        currentCount = 0;
    }

    //执行清空数据命令
    if (clear) {
        txtMain->clear();
        currentCount = 0;
        return;
    }

    //执行条数到了清空数据
    if (currentCount >= maxCount) {
        txtMain->clear();
        currentCount = 0;
    }

    //过滤回车换行符
    QString strData = data;
    strData.replace("\r", "");
    strData.replace("\n", "");

    //不同类型不同颜色显示
    QString strType;
    if (type == 0) {
        strType = "信息";
        txtMain->setTextColor(QColor("#32B9CF"));
    } else if (type == 1) {
        strType = "错误";
        txtMain->setTextColor(QColor("#D64D54"));
    } else if (type == 2) {
        strType = "行数";
        txtMain->setTextColor(QColor("#34495E"));
    } else {
        strType = "其他";
        txtMain->setTextColor(QColor("#8D44AF"));
    }

    strData = QString("%1 [%2]: %3").arg(TIMEMS).arg(strType).arg(strData);
    txtMain->append(strData);
    currentCount++;
}

QStringList DbHelper::getDbType()
{
    static QStringList listDbType;
    if (listDbType.count() == 0) {
        listDbType << "ODBC" << "Sqlite" << "MySql" << "PostgreSQL" << "SqlServer" << "Oracle" << "KingBase";
    }

    return listDbType;
}

QString DbHelper::getDbType(const DbType &dbType)
{
    QString type = "ODBC";
    if (dbType == DbType_Sqlite) {
        type = "SQLITE";
    } else if (dbType == DbType_MySql) {
        type = "MYSQL";
    } else if (dbType == DbType_SqlServer) {
        type = "SQLSERVER";
    } else if (dbType == DbType_PostgreSQL) {
        type = "POSTGRESQL";
    } else if (dbType == DbType_KingBase) {
        type = "KINGBASE";
    }

    return type;
}

DbType DbHelper::getDbType(const QString &dbType)
{
    //强制转成大写再比较防止出错
    QString temp = dbType.toUpper();
    DbType type = DbType_ODBC;
    if (temp == "SQLITE") {
        type = DbType_Sqlite;
    } else if (temp == "MYSQL") {
        type = DbType_MySql;
    } else if (temp == "SQLSERVER") {
        type = DbType_SqlServer;
    } else if (temp == "POSTGRESQL") {
        type = DbType_PostgreSQL;
    } else if (temp == "KINGBASE") {
        type = DbType_KingBase;
    }

    return type;
}

void DbHelper::getDbDefaultInfo(const QString &dbType, QString &hostPort,
                                QString &userName, QString &userPwd)
{
    if (dbType == "MYSQL") {
        hostPort = "3306";
        userName = "root";
        userPwd = "root";
    } else if (dbType == "SQLSERVER") {
        hostPort = "1433";
        userName = "sa";
        userPwd = "123456";
    } else if (dbType == "POSTGRESQL") {
        hostPort = "5432";
        userName = "postgres";
        userPwd = "123456";
    } else if (dbType == "ORACLE") {
        hostPort = "1521";
        userName = "system";
        userPwd = "123456";
    } else if (dbType == "KINGBASE") {
        hostPort = "54321";
        userName = "SYSTEM";
        userPwd = "123456";
    }
}

QString DbHelper::appName()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static QString name;
    if (name.isEmpty()) {
        name = qApp->applicationFilePath();
        //下面的方法主要为了过滤安卓的路径 lib程序名_armeabi-v7a
        QStringList list = name.split("/");
        name = list.at(list.count() - 1).split(".").at(0);
    }

    return name;
}

QString DbHelper::appPath()
{
#ifdef Q_OS_ANDROID
    //return QString("/sdcard/Android/%1").arg(appName());
    return QString("/storage/emulated/0/%1").arg(appName());
#else
    return qApp->applicationDirPath();
#endif
}

QString DbHelper::getDbDefaultFile(const QString &name)
{
    QString dbName = name.isEmpty() ? appName() : name;
    dbName = QString("%1/db/%2.db").arg(appPath()).arg(dbName);
    return dbName;
}

bool DbHelper::existNoSupportSql(const QString &sql)
{
    if (sql.length() < 5 ||
        sql.startsWith("--") ||
        sql.startsWith("PRAGMA") ||
        sql.startsWith("BEGIN TRANSACTION") ||
        sql.startsWith("COMMIT TRANSACTION") ||
        sql.startsWith("DROP INDEX IF EXISTS")) {
        return true;
    }
    return false;
}

void DbHelper::checkSql(const QString &dbType, QString &sql)
{
    //表字段类型替换+表字段关键字替换
    replaceColumnType(sql, "INTEGER");
    if (dbType != "MYSQL") {
        replaceColumnName(sql, "current");
        replaceColumnName(sql, "plan");
    }

    //替换部分数据类型
    sql.replace("REAL(11,1)", "REAL");
    //替换自增字段 自增字段容易出问题各种数据库处理方式不一致不建议使用
    sql.replace(" AUTOINCREMENT", "");
    //sqlserver2016以前的版本不认识 DROP TABLE IF EXISTS
    if (dbType == "SQLSERVER") {
        sql.replace("DROP TABLE IF EXISTS", "DROP TABLE");
    }
}

void DbHelper::replaceColumnName(QString &sql, const QString &key)
{
    if (sql.contains(key) && !sql.contains(QString("%1_").arg(key))) {
        sql.replace(key, QString("\"%1\"").arg(key));
    }
}

void DbHelper::replaceColumnType(QString &sql, const QString &key)
{
    if (sql.contains(key)) {
        //先去掉字段类型和长度之间的空格
        sql.replace("INTEGER (", "INTEGER(");
#if 1
        QRegExp regExp(QString("%1\\(\\d+\\)").arg(key));
        int start = regExp.indexIn(sql);
        int length = regExp.matchedLength();
        if (length > 0) {
            QString result = sql.mid(start, length);
            sql.replace(result, key);
        }
#elif 1
        sql.replace(QRegExp(QString("%1\\(\\d+\\)").arg(key)), key);
#else
        QStringList listAfter;
        QStringList listBefore = sql.split(",");
        foreach (QString str, listBefore) {
            if (str.contains(key)) {
                for (int i = 1; i < 20; i++) {
                    QString flag = QString("%1(%2)").arg(key).arg(i);
                    str.replace(flag, key);
                }
            }
            listAfter << str;
        }
        sql = listAfter.join(",");
#endif
    }
}

bool DbHelper::ipLive(const QString &ip, int port, int timeout)
{
    //局部的事件循环,不卡主界面
    QEventLoop eventLoop;

    //设置超时
    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    timer.setSingleShot(true);
    timer.start(timeout);

    QTcpSocket tcpSocket;
    QObject::connect(&tcpSocket, SIGNAL(connected()), &eventLoop, SLOT(quit()));
    tcpSocket.connectToHost(ip, port);
    eventLoop.exec();
    bool ok = (tcpSocket.state() == QAbstractSocket::ConnectedState);
    return ok;
}

bool DbHelper::checkDatabase(const QString &dbType, const DbInfo &dbInfo)
{
    if (dbType == "SQLITE") {
        //判断数据库文件是否存在,不存在则尝试从资源文件复制出来
        QString sourceFile = QString(":/%1.db").arg(appName());
        QString targetFile = dbInfo.dbName;
        QFile file(dbInfo.dbName);
        if (!file.exists() || file.size() == 0) {
            file.remove();
            file.copy(sourceFile, targetFile);
            //将复制过去的文件只读属性取消
            file.setPermissions(targetFile, QFile::WriteOwner);
        }

        //执行完成以后不管成功与否再判断下文件大小
        if (QFile(dbInfo.dbName).size() <= 4) {
            //这里不用返回假因为就算文件不存在也可以通过执行sql脚本生成数据库文件
        }
    } else {
        //先判断数据库服务器IP地址是否存在,否则会卡很久
        if (!ipLive(dbInfo.hostName, dbInfo.hostPort, 1000)) {
            return false;
        }
    }
    return true;
}

bool DbHelper::initDatabase(bool testConnect, const QString &dbType,
                            QSqlDatabase &database, const DbInfo &dbInfo)
{
    //先检查数据库连接对象前置条件
    if (!checkDatabase(dbType, dbInfo)) {
        return false;
    }

    //不同数据库连接字符串不一样
    QString connName = dbInfo.connName;
    if (dbType == "SQLITE") {
        database = QSqlDatabase::addDatabase("QSQLITE", connName);
    } else if (dbType == "MYSQL") {
        database = QSqlDatabase::addDatabase("QMYSQL", connName);
        database.setConnectOptions("MYSQL_OPT_RECONNECT=1;MYSQL_OPT_CONNECT_TIMEOUT=1;MYSQL_OPT_READ_TIMEOUT=1;MYSQL_OPT_WRITE_TIMEOUT=1");
    } else if (dbType == "SQLSERVER") {
        database = QSqlDatabase::addDatabase("QODBC", connName);
    } else if (dbType == "POSTGRESQL") {
        database = QSqlDatabase::addDatabase("QPSQL", connName);
    } else if (dbType == "KINGBASE") {
        database = QSqlDatabase::addDatabase("QKINGBASE", connName);
    } else {
        database = QSqlDatabase::addDatabase("QODBC", connName);
    }

    //如果是用来测试连接则对应数据库为空
    //在测试阶段可能真正的数据库还不存在,需要测试完执行新建数据库
    if (dbType == "SQLSERVER") {
#if 0
        //如果设置了数据源的形式只需要下面一行代码就行
        database.setDatabaseName(dbInfo.dbName);
#else
        //下面这种方式不需要设置数据源
        QStringList list;
        list << QString("DRIVER={%1}").arg("SQL SERVER");
        list << QString("SERVER=%1,%2").arg(dbInfo.hostName).arg(dbInfo.hostPort);
        if (!testConnect) {
            list << QString("DATABASE=%1").arg(dbInfo.dbName);
            list << QString("UID=%1").arg(dbInfo.userName);
            list << QString("PWD=%1").arg(dbInfo.userPwd);
        }
        database.setDatabaseName(list.join(";"));
#endif
    } else {
        if (!testConnect) {
            database.setDatabaseName(dbInfo.dbName);
        }
    }

    //设置主机地址端口和用户信息
    database.setHostName(dbInfo.hostName);
    database.setPort(dbInfo.hostPort);
    database.setUserName(dbInfo.userName);
    database.setPassword(dbInfo.userPwd);
    return true;
}

bool DbHelper::execSql(const QString &sql, QSqlDatabase database)
{
    QSqlQuery query(database);
    if (!query.exec(sql)) {
        qDebug() << TIMEMS << query.lastError().text();
        return false;
    }
    return true;
}

void DbHelper::clearTable(const QString &tableName, const QString &dbType, QSqlDatabase database)
{
    QString sql;
    if (dbType.toUpper() == "SQLITE") {
        //SQLite数据库自增列信息存储在 sqlite_sequence 表,表包含两个列：name记录自增列所在的表,seq记录当前序号
        sql = QString("delete from %1").arg(tableName);
        execSql(sql, database);
        sql = QString("update sqlite_sequence set seq=0 where name='%1'").arg(tableName);
        //execSql(sql, database);
    } else if (dbType.toUpper() == "MYSQL") {
        //重置自增主键,用 truncate table table_name 效率高,而不是 alter table table_name auto_increment = 1
        sql = QString("truncate table %1").arg(tableName);
        execSql(sql, database);
    }
}

int DbHelper::getMaxID(const QString &tableName, const QString &columnName, QSqlDatabase database)
{
    int maxID = 0;
    QSqlQuery query(database);
    QString sql = QString("select max(%1) from %2").arg(columnName).arg(tableName);
    if (query.exec(sql)) {
        if (query.next()) {
            maxID = query.value(0).toInt();
        }
    } else {
        qDebug() << TIMEMS << query.lastError().text();
    }

    return maxID;
}

void DbHelper::bindData(const QString &columnName, const QString &orderSql,
                        const QString &tableName, QComboBox *cbox, QSqlDatabase database)
{
    QSqlQuery query(database);
    QString sql = QString("select %1 from %2 order by %3").arg(columnName).arg(tableName).arg(orderSql);
    if (query.exec(sql)) {
        while (query.next()) {
            QString value = query.value(0).toString();
            cbox->addItem(value);
        }
    } else {
        qDebug() << TIMEMS << query.lastError().text();
    }
}

void DbHelper::bindData(const QString &columnName, const QString &orderSql,
                        const QString &tableName, QList<QComboBox *> cboxs, QSqlDatabase database)
{
    QSqlQuery query(database);
    QString sql = QString("select %1 from %2 order by %3").arg(columnName).arg(tableName).arg(orderSql);
    if (query.exec(sql)) {
        while (query.next()) {
            QString value = query.value(0).toString();
            foreach (QComboBox *cbox, cboxs) {
                cbox->addItem(value);
            }
        }
    } else {
        qDebug() << TIMEMS << query.lastError().text();
    }
}
