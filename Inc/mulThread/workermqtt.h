#ifndef WORKERMQTT_H
#define WORKERMQTT_H

#include <QObject>
///////////////////////////////////////////////////////
#include "workStationDataStructure.h"
#include "mqttDataStructure.h"
#include "qmqtt.h"
#include "QMqtt_Client.h"
using namespace  QMQTT;

//Halcon包含
#include "halcon_inc.h"
///////////////////////////////////////////////////////

class WorkerMQTT : public QObject
{
    Q_OBJECT
public:
    explicit WorkerMQTT(QObject *parent = nullptr);
    ~WorkerMQTT();

public:
    void mqtt_connect(void);
    void mqtt_subscribe(void);
    void mqtt_disconnect(void);

public:
    //mqtt client指针
    Client *p_mqtt_client;

    int m_mqttUse;                 // 是否启用MQTT（0-禁用，1-启用）
    int m_mqttTriggerTime;         // MQTT触发时间（单位：ms）
    QString m_mqttHostname;        // MQTT服务器主机名
    int m_mqttPort;                // MQTT服务器端口
    QString m_mqttSaveImagePath;   // MQTT图像保存路径
    int m_mqttImageCycleNum;       // MQTT图像循环数量
    QString m_mqttPublicMsg;       // MQTT消息主题-【错误识别的消息】

signals:

public slots:
    void init(const WorkStation_DATA& paramData);

    void startConnectMqtt();
    void connected();
    void disconnected();
    void subscribed(const QString& topic, const quint8 qos = 0);
    void published(const QMQTT::Message& message, quint16);
    void received(const QMQTT::Message& message);
    void error(const QMQTT::ClientError error);

    void parseMqttDataTest(QString data);
    QString SaveImgToDisk(HObject &ho_image,QString fName,int num);
    bool DirExist(QString fullPath);
    bool DirExistEx(QString fullPath);

    void mqtt_public_mydata(void);
};

#endif // WORKERMQTT_H
