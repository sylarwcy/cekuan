#ifndef _ORCLDATA_
#define _ORCLDATA_

#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

class ORCLDATA
{
public:
    ORCLDATA(QString serverName,QString databaseName,QString user,QString pass);
    ~ORCLDATA();

    bool Connect();
    bool ExcuteSQL(QString strsql);

	int GetRecordCount(void);
	bool ExecProc(void);

public:
    QSqlDatabase db;
    QSqlQuery *pQuery;
private:
    QString hostName;
    QString dbName;
    QString userName;
    QString password;
    int port;
};
#endif
