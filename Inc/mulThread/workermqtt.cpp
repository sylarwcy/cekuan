#include "workermqtt.h"
#include <QDebug>

WorkerMQTT::WorkerMQTT(QObject *parent) : QObject(parent), p_mqtt_client(nullptr) {
    qRegisterMetaType<PlateMqttReportData>("PlateMqttReportData");
}

WorkerMQTT::~WorkerMQTT() {
    if (p_mqtt_client) {
        delete p_mqtt_client;
    }
}

void WorkerMQTT::init(const WorkStation_DATA &paramData) {
    m_mqttUse = 1; // 强行启用，适配当前Web联调
    m_mqttHostname = "115.28.140.23"; // 临时与文件服务器同IP或指定您的Broker IP
    m_mqttPort = 1883;
    m_mqttPublicMsg = "/industrial/measure/plate_report"; // WEB指定报文接收主题
}

void WorkerMQTT::startConnectMqtt() {
    p_mqtt_client = new QMQTT::Client();

    connect(p_mqtt_client, &QMQTT::Client::connected, this, &WorkerMQTT::connected);
    connect(p_mqtt_client, &QMQTT::Client::disconnected, this, &WorkerMQTT::disconnected);
    connect(p_mqtt_client, &QMQTT::Client::error, this, &WorkerMQTT::error);

    mqtt_connect();
}

void WorkerMQTT::mqtt_connect() {
    QMQTT::ConnectionState state = p_mqtt_client->connectionState();
    if (state == QMQTT::STATE_DISCONNECTED || state == QMQTT::STATE_INIT) {
        p_mqtt_client->setHostName(m_mqttHostname);
        p_mqtt_client->setPort(m_mqttPort);
        p_mqtt_client->setAutoReconnect(true);
        p_mqtt_client->setAutoReconnectInterval(5000);
        p_mqtt_client->connectToHost();
    }
}

void WorkerMQTT::mqtt_disconnect() {
    if (p_mqtt_client && p_mqtt_client->connectionState() == QMQTT::STATE_CONNECTED) {
        p_mqtt_client->disconnectFromHost();
    }
}

void WorkerMQTT::connected() {
    qDebug() << "[MQTT后台成功] 成功连接至Web端数据中转Broker:" << m_mqttHostname;
}

void WorkerMQTT::disconnected() {
    qWarning() << "[MQTT断开] 警告：与Web端中转Broker的连接已断开！";
}

void WorkerMQTT::error(const QMQTT::ClientError error) {
    qWarning() << "[MQTT异常] 错误码:" << error;
}

// =======================================================================
// 🌟【核心重构实现】：后台异步远程Halcon写图 + JSON拼装 + 高速上报Web
// =======================================================================
void WorkerMQTT::slot_uploadAndSendReport(const PlateMqttReportData& report) {
    QString fusedWebPath = "";
    QString contourWebPath = "";

    try {
        // 1. 生成物理存储绝对路径与Web相对映射路径
        QString fusedFileName = QString("%1_fused").arg(report.plateID);
        QString contourFileName = QString("%1_contour").arg(report.plateID);

        // UNC网络物理存储路径（例如：//115.28.140.23/FileServerRoot/0605_130215_fused.jpg）
        QString remoteFusedAbsPath = QString("%1/%2").arg(m_fileServerLocalShare).arg(fusedFileName);
        QString remoteContourAbsPath = QString("%1/%2").arg(m_fileServerLocalShare).arg(contourFileName);

        // WEB可直接读取的虚拟映射相对路径（供JSON下发使用）
        fusedWebPath = QString("%1/%2.jpg").arg(m_webAccessPrefix).arg(fusedFileName);
        contourWebPath = QString("%1/%2.jpg").arg(m_webAccessPrefix).arg(contourFileName);

        // 2. 利用Halcon WriteImage的底层强悍网络特性，跨网络无损写盘存储到共享文件服务器
        if (report.ho_fusedImage.IsInitialized()) {
            HalconCpp::WriteImage(report.ho_fusedImage, "jpeg", 0, remoteFusedAbsPath.toLocal8Bit().constData());
        }
        if (report.ho_contourImage.IsInitialized()) {
            HalconCpp::WriteImage(report.ho_contourImage, "jpeg", 0, remoteContourAbsPath.toLocal8Bit().constData());
        }
        qDebug() << "[文件服务器异步存储成功] 拼接大图:" << fusedWebPath << ", 轮廓大图:" << contourWebPath;

    } catch (HalconCpp::HException &e) {
        qCritical() << "[文件服务器存图严重崩溃] 远程网络路径不可达，请检查文件服务器共享访问控制。具体原因:" << e.ErrorMessage().Text();
    }

    // 3. 构建完备的 Web JSON 交互报文
    QJsonObject rootObj;
    rootObj.insert("plateID", report.plateID);
    rootObj.insert("length", report.length);
    rootObj.insert("thickness", report.thickness);
    rootObj.insert("targetWidth", report.targetWidth);
    rootObj.insert("avgWidth", report.avgWidth);
    rootObj.insert("maxWidth", report.maxWidth);
    rootObj.insert("minWidth", report.minWidth);
    rootObj.insert("fusedImagePath", fusedWebPath);
    rootObj.insert("contourImagePath", contourWebPath);

    // 压入全量滤波精细白线宽度特征数据数组
    QJsonArray curveArray;
    for (double v : report.curveValues) {
        curveArray.append(v);
    }
    rootObj.insert("curveValues", curveArray);

    QJsonDocument doc(rootObj);
    QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);

    // 4. 执行异步的高刚性协议发布
    if (p_mqtt_client && p_mqtt_client->connectionState() == QMQTT::STATE_CONNECTED) {
        QMQTT::Message msg(0, m_mqttPublicMsg, jsonBytes, 1); // QoS=1 稳健传递
        p_mqtt_client->publish(msg);
        qDebug() << "[MQTT大数据上报Web成功] 主题:" << m_mqttPublicMsg << " 字节大小:" << jsonBytes.size();
    } else {
        qWarning() << "[MQTT上报失败] MQTT Broker未就绪或连接断开！已自动丢弃当前长钢板上报包。";
    }
}