//
// Created by sylar on 26-6-1.
//

#include "databasemanager.h"

DatabaseManager::~DatabaseManager() {
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::database(m_connectionName).close();
    }
}

bool DatabaseManager::initDatabase(const QString& dbName) {
    QSqlDatabase db;
    if (QSqlDatabase::contains(m_connectionName)) {
        db = QSqlDatabase::database(m_connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/" + dbName);
    }

    if (!db.open()) {
        qCritical() << "[数据库错误] 无法打开SQLite实例:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS PlateRecord (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            plateID TEXT,
            length REAL,
            thickness REAL,
            targetWidth REAL,
            avgWidth REAL,
            maxWidth REAL,
            minWidth REAL,
            measureTime DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    if (!query.exec(createTableSql)) {
        qCritical() << "[数据库错误] 建表失败:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::saveRecordToDb(const PlateRecord& record) {
    if (!QSqlDatabase::contains(m_connectionName)) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO PlateRecord "
                  "(plateID, length, thickness, targetWidth, avgWidth, maxWidth, minWidth) "
                  "VALUES (:plateID, :length, :thickness, :targetWidth, :avgWidth, :maxWidth, :minWidth)");

    query.bindValue(":plateID", record.plateID);
    query.bindValue(":length", record.length);
    query.bindValue(":thickness", record.thickness);
    query.bindValue(":targetWidth", record.targetWidth);
    query.bindValue(":avgWidth", record.avgWidth);
    query.bindValue(":maxWidth", record.maxWidth);
    query.bindValue(":minWidth", record.minWidth);

    if (!query.exec()) {
        qWarning() << "[数据库警告] 历史记录写入失败:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<PlateRecord> DatabaseManager::loadHistoryFromDb(int limit) {
    QList<PlateRecord> list;
    if (!QSqlDatabase::contains(m_connectionName)) return list;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) return list;

    QSqlQuery query(db);
    query.prepare(QString("SELECT plateID, length, thickness, targetWidth, avgWidth, maxWidth, minWidth "
                          "FROM PlateRecord ORDER BY id DESC LIMIT %1").arg(limit));

    if (!query.exec()) {
        qWarning() << "[数据库警告] 查询历史记录失败:" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        PlateRecord rec;
        rec.plateID     = query.value(0).toString();
        rec.length      = query.value(1).toDouble();
        rec.thickness   = query.value(2).toDouble();
        rec.targetWidth = query.value(3).toDouble();
        rec.avgWidth    = query.value(4).toDouble();
        rec.maxWidth    = query.value(5).toDouble();
        rec.minWidth    = query.value(6).toDouble();
        list.append(rec);
    }
    return list;
}
