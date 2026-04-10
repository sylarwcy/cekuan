#ifndef WORKERTRIGGER_H
#define WORKERTRIGGER_H

#include <QObject>
#include "workStationDataStructure.h"

class WorkerTrigger : public QObject
{
    Q_OBJECT
public:
    explicit WorkerTrigger(QObject *parent = nullptr);
    ~WorkerTrigger();
    void init(const WorkStation_DATA& paramData);

    QString m_location;       //工位位置
    int m_serialNumber;       //工位序号
    int m_loop_refresh_ms;    //刷新等待时间
    int m_mqtt_trigger_time;  //mqtt刷新时间
    QString m_bufferPath;//需要定期删除缓存的路径

signals:
    void sig_refreshDisplay();
    void sig_mqtt_trigger();

public slots:
    void on_doSomething();
    void timerMsByCPU(double mSleepTime);

};

void clearCache(const QString &path, int days);

#endif // WORKERTRIGGER_H
