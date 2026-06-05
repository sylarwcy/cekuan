#ifndef WORKERMQTT_H
#define WORKERMQTT_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QTimer>
#include "workStationDataStructure.h"
#include "qmqtt.h"
#include "QMqtt_Client.h"
#include "halconcpp/HalconCpp.h"

class WorkerMQTT : public QObject
{
    Q_OBJECT
public:
    explicit WorkerMQTT(QObject *parent = nullptr);
    ~WorkerMQTT();

    void mqtt_connect(void);
    void mqtt_disconnect(void);

public:
    QMQTT::Client *p_mqtt_client;

    int m_mqttUse;
    QString m_mqttHostname;
    int m_mqttPort;
    QString m_mqttPublicMsg; // 发布主题

    // 🌟 临时硬编码的文件服务器配置（后续调试完成可以移入配置文件）
    QString m_fileServerIP{"115.28.140.23"};
    QString m_fileServerLocalShare{"//115.28.140.23/FileServerRoot"}; // Windows共享挂载绝对物理路径
    QString m_webAccessPrefix{"/dist/images"}; // 映射出来的给WEB端访问的相对根路径

public slots:
    void init(const WorkStation_DATA& paramData);
    void startConnectMqtt();

    // 🌟 核心新增：在后台线程中异步执行Halcon远程存图、JSON打包、并实时发布给Web端
    void slot_uploadAndSendReport(const PlateMqttReportData& report);

private slots:
    void connected();
    void disconnected();
    void error(const QMQTT::ClientError error);
};

#endif // WORKERMQTT_H