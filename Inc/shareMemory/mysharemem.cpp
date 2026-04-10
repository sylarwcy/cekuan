#include "mysharemem.h"
#include <QDebug>

MyShareMem::MyShareMem()
{
    m_shareName="";
    m_shareSize=0;
}

MyShareMem::MyShareMem(QString name,int size)
{
    m_shareName=name;
    m_shareSize=size;

    m_shareMem.setKey(m_shareName);
    if(m_shareMem.create(m_shareSize))
        qDebug()<<"sharememory already create!";
    else
        qDebug()<<"sharememory create ok!";
}

MyShareMem::~MyShareMem()
{

}

void MyShareMem::SetShareMem(QString name,int size)
{
    m_shareName=name;
    m_shareSize=size;

    m_shareMem.setKey(m_shareName);
    if(m_shareMem.create(m_shareSize))
        qDebug()<<"sharememory already create!";
    else
        qDebug()<<"sharememory create ok!";
}

bool MyShareMem::ReadFromShareMem(void *pData)
{
    void *pShareData;

    if(m_shareSize==0)
    {
        qDebug()<<"sharememory not create!";
        return false;
    }

    if(!m_shareMem.isAttached())
    {
        if(m_shareMem.attach())
        {
            pShareData=m_shareMem.data();
            m_shareMem.lock();
            memcpy(pData,pShareData,m_shareSize);
            m_shareMem.unlock();
        }
        else {
            qDebug()<<"sharememory attach error!";
            return false;
        }
    }
    else
    {
        pShareData=m_shareMem.data();
        m_shareMem.lock();
        memcpy(pData,pShareData,m_shareSize);
        m_shareMem.unlock();
    }
    return true;
}
bool MyShareMem::WriteToShareMem(void *pData)
{
    void *pShareData;

    if(m_shareSize==0)
    {
        qDebug()<<"sharememory not create!";
        return false;
    }

    if(!m_shareMem.isAttached())
    {
        if(m_shareMem.attach())
        {
            pShareData=m_shareMem.data();
            m_shareMem.lock();
            memcpy(pShareData,pData,m_shareSize);
            m_shareMem.unlock();
        }
        else {
            qDebug()<<"sharememory attach error!";
            return false;
        }
    }
    else
    {
        pShareData=m_shareMem.data();
        m_shareMem.lock();
        memcpy(pShareData,pData,m_shareSize);
        m_shareMem.unlock();
    }
    return true;
}
