#include "orcldata.h"

ORCLDATA::ORCLDATA(QString serverName,QString databaseName,QString user,QString pass)
{
    db=QSqlDatabase::addDatabase("QOCI");

    hostName=serverName;
    dbName=databaseName;
    userName=user;
    password=pass;
    port=1521;

    qDebug()<<hostName<<dbName<<user<<pass;

    pQuery=new QSqlQuery(db);
}

ORCLDATA::~ORCLDATA()
{
    db.close();
    delete pQuery;
}

bool ORCLDATA::Connect()
{
    db.setPort(port);
    db.setHostName(hostName);
    db.setDatabaseName(dbName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!db.open())
    {
        qDebug()<<"Unable to establish a database connection!";
        return false;
    }
    else
    {
        qDebug()<<"oracle database connnect successfuly!";
        return true;
    }
}	

bool ORCLDATA::ExcuteSQL(QString strsql)
{
//    if(!Connect())
//      return false;


    if(!pQuery->exec(strsql))
    {
        qDebug()<<pQuery->lastError().text();
        return false;
    }

    qDebug()<<"query ExcuteSQL run successfuly!";
    return true;
}


int ORCLDATA::GetRecordCount(void)
{

    return true;
}

bool ORCLDATA::ExecProc(void)
{

    return true;
}
