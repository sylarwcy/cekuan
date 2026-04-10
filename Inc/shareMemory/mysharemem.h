#ifndef MYSHAREMEM_H
#define MYSHAREMEM_H

#include <QSharedMemory>

class MyShareMem
{
public:
    MyShareMem();
    MyShareMem(QString name,int size);
    ~MyShareMem();

public:
    QSharedMemory m_shareMem;
    QString m_shareName;
    int m_shareSize;

public:
    void SetShareMem(QString name,int size);
    bool ReadFromShareMem(void *pData);
    bool WriteToShareMem(void *pData);

signals:

};

#endif // MYSHAREMEM_H
