#ifndef STABLE_H
#define STABLE_H
 
// Add C includes here
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <math.h>
 
#if defined __cplusplus
//Add C++ includes here
#include <iostream>
#include <algorithm>
#include <string.h>
#include <vector>
using namespace std;
 
//Qt includes
#include <QApplication>
#include <QThread>
#include <QPushButton>
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>
 
//最大最小值函数
#ifndef myMax
#define myMax(x, y) ((x) < (y) ? (y) : (x))
#define myMin(x, y) ((x) < (y) ? (x) : (y))
#endif
 
//解决UTF-8编码中文乱码的问题
#ifdef _MSC_VER
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#pragma warning(disable : 4819)
#endif // _MSC_VER >= 1600
#endif // _MSC_VER
 
//扩展qDebug以文件行列记录信息
#define QLOG_DEBUG(msg) qDebug() << QString("[%1][%2][%3][%4]%5")                                               \
                                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))  \
                                        .arg("DEBUG")                                                           \
                                        .arg(QString("%1:%2:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)) \
                                        .arg(QThread::currentThread()->objectName())                            \
                                        .arg(msg);
 
#define FIRLOG //FIRLOG::日志开关,注释关闭打印日志
#ifdef FIRLOG
#define firecat_log qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") \
                             << "[DEBUG]" << __FILE__ << __LINE__ << __FUNCTION__ << "msg:"
#else
#define firecat_log qDebug()
#endif
 
#endif //__cplusplus
 
#endif // STABLE_H
