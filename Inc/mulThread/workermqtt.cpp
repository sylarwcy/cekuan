#include "workermqtt.h"
#include "MyApplication.h"
#include "appconfig.h"
///////////////////////////////////////////////////////////////
bool isHObjectEmpty(const HObject &ho_Obj) {
    //halcon判断HObject是否为空
    HObject ho_Null;

    try {
        GenEmptyObj(&ho_Null);
    } catch (HException &except) {
        //HALCON error #2036: could not find license file
        QLOG_INFO() << except.ErrorMessage().Text();
        return true;
    }

    //IsInitialized只能用来判断HObject有无初始化,不能判断HObject空不空或有没有
    if (!ho_Obj.IsInitialized()) {
        return true;
    }

    HTuple hv_n;
    TestEqualObj(ho_Obj, ho_Null, &hv_n);
    //两个Obj相等
    if (hv_n == 1) {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////
WorkerMQTT::WorkerMQTT(QObject *parent)
    : QObject{parent} {
    p_mqtt_client = NULL;
}

WorkerMQTT::~WorkerMQTT() {
    delete p_mqtt_client;
    p_mqtt_client = NULL;
}

void WorkerMQTT::init(const WorkStation_DATA &paramData) {
    m_mqttUse = paramData.mqttUse; // 是否启用MQTT（0-禁用，1-启用）
    m_mqttTriggerTime = paramData.mqttTriggerTime; // MQTT触发时间（单位：ms）
    m_mqttHostname = paramData.mqttHostname; // MQTT服务器主机名
    m_mqttPort = paramData.mqttPort; // MQTT服务器端口
    m_mqttSaveImagePath = paramData.mqttSaveImagePath; // MQTT图像保存路径
    m_mqttImageCycleNum = paramData.mqttImageCycleNum; // MQTT图像循环数量
    m_mqttPublicMsg = paramData.mqttPublicMsg; // 【接收】MQTT消息主题-识别错误的板号
}

void WorkerMQTT::startConnectMqtt() {
    //创建MQTT客户端
    p_mqtt_client = new Client();

    connect(p_mqtt_client,SIGNAL(connected()), this,SLOT(connected()));
    connect(p_mqtt_client,SIGNAL(disconnected()), this,SLOT(disconnected()));
    connect(p_mqtt_client,SIGNAL(subscribed(const QString&, const quint8)), this,
            SLOT(subscribed(const QString&, const quint8)));
    connect(p_mqtt_client,SIGNAL(published(const QMQTT::Message&, const quint16)), this,
            SLOT(published(const QMQTT::Message&, const quint16)));
    connect(p_mqtt_client,SIGNAL(received(const QMQTT::Message& )), this,
            SLOT(received(const QMQTT::Message&)));
    connect(p_mqtt_client,SIGNAL(error(const QMQTT::ClientError)), this,
            SLOT(error(const QMQTT::ClientError)));

    //mqtt连接
    if (m_mqttUse == 1) {
        //延时连接mqtt
        QTimer::singleShot(5000, this, &WorkerMQTT::mqtt_connect);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
void WorkerMQTT::connected() {
    QLOG_INFO() << QString("mqtt connected...,host=%1,port=%2").arg(m_mqttHostname).arg(
                   m_mqttPort);

    //连接成功，开始订阅
    p_mqtt_client->subscribe(m_mqttPublicMsg, 1);
    //mqtt_subscribe();
}

void WorkerMQTT::disconnected() {
    QLOG_INFO() << "mqtt disconnect...";
}

void WorkerMQTT::subscribed(const QString &topic, const quint8 qos) {
    QLOG_INFO() << QString("mqtt subscribed...,top=%1,Qos=%2").arg(topic).arg(qos);
}

void WorkerMQTT::published(const QMQTT::Message &message, quint16 msgid) {
    ;
    //qDebug()<< QString("mqtt published...,msgid=%1").arg(msgid);
}

void WorkerMQTT::parseMqttDataTest(QString data) {
    // 1. 解析 JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON 解析错误：" << parseError.errorString();
        return;
    }

    // 4. 获取 JSON 对象
    QJsonObject jsonObj = jsonDoc.object();

    // 5. 提取字段
    QString plcId = jsonObj["picId"].toString();
    QString actualId = jsonObj["actualId"].toString();

    // 6. 打印解析结果
    qDebug() << "PiC ID:" << plcId;
    qDebug() << "Actual ID:" << actualId;
    QLOG_INFO() << "PiC ID:" << plcId;
    QLOG_INFO()  << "Actual ID:" << actualId;
}

void WorkerMQTT::received(const QMQTT::Message &message) {
    QString mes = QString("received msg:id=%1,Qos=%2").arg(message.id()).arg(message.qos()) + "," + message.topic() +
                  "," + message.payload();
    qDebug() << mes;

    if (message.topic() == m_mqttPublicMsg) {
        ;//todo:调用需要处理mqtt纠错数据的线程里的槽
        parseMqttDataTest(message.payload());
    }


    /*
        QByteArray payload = message.payload();
        char* pArrayData = payload.data();

        int *pIntValue = (int *)pArrayData;

        int iChannel = (*pIntValue);
        pIntValue++;
        int iElementSize1   = (*pIntValue);

        pIntValue++;
        int iWidth  = (*pIntValue);

        pIntValue++;
        int iHeight = (*pIntValue);
    */
}

void WorkerMQTT::error(const QMQTT::ClientError error) {
    QString strError;

    switch (error) {
        case QMQTT::SocketConnectionRefusedError:
            strError = QStringLiteral("Socket connection refused");
            break;
        case QMQTT::SocketRemoteHostClosedError:
            strError = QStringLiteral("Socket remote host closed");
            break;
        case QMQTT::SocketHostNotFoundError:
            strError = QStringLiteral("Socket host not found");
            break;
        case QMQTT::SocketAccessError:
            strError = QStringLiteral("Socket access error");
            break;
        case QMQTT::SocketResourceError:
            strError = QStringLiteral("Socket resource error");
            break;
        case QMQTT::SocketTimeoutError:
            strError = QStringLiteral("Socket timeout error");
            break;
        case QMQTT::SocketDatagramTooLargeError:
            strError = QStringLiteral("Socket datagram too large error");
            break;
        case QMQTT::SocketNetworkError:
            strError = QStringLiteral("Socket network error");
            break;
        case QMQTT::SocketAddressInUseError:
            strError = QStringLiteral("Socket address in use error");
            break;
        case QMQTT::SocketUnsupportedSocketOperationError:
            strError = QStringLiteral("Socket unsupported socket operation error");
            break;
        case QMQTT::SocketProxyAuthenticationRequiredError:
            strError = QStringLiteral("Socket proxy authentication required error");
            break;
        case QMQTT::SocketSslHandshakeFailedError:
            strError = QStringLiteral("Socket SSL handshake failed error");
            break;
        default:
            strError = QStringLiteral("Unknown error");
    }

    QLOG_INFO() << strError;
}

void WorkerMQTT::mqtt_connect() {
    //    STATE_INIT = 0,
    //    STATE_CONNECTING,
    //    STATE_CONNECTED,
    //    STATE_DISCONNECTED
    QMQTT::ConnectionState MQttState = p_mqtt_client->connectionState();
    //未连接服务器则连接
    if (MQttState == QMQTT::STATE_DISCONNECTED || MQttState == QMQTT::STATE_INIT) {
        p_mqtt_client->setHostName(m_mqttHostname);
        p_mqtt_client->setPort(m_mqttPort);
        //设置自动重新连接
        p_mqtt_client->setAutoReconnect(true);
        p_mqtt_client->setAutoReconnectInterval(10000);
        //开始连接
        p_mqtt_client->connectToHost();
    }
}

void WorkerMQTT::mqtt_subscribe() {
    ;

    //    MyApplication *pApp=(MyApplication *)qApp;
    //    GlobalData &myData=pApp->globalData;

    //    QMQTT::ConnectionState MQttState = pApp->p_mqtt_client->connectionState();
    //    if (MQttState == QMQTT::STATE_CONNECTED)
    //    {
    //        pApp->p_mqtt_client->subscribe(myData.setting.mqtt_subscribe,0);
    //    }
}

void WorkerMQTT::mqtt_disconnect(void) {
    QMQTT::ConnectionState MQttState = p_mqtt_client->connectionState();
    if (MQttState == QMQTT::STATE_CONNECTED) {
        p_mqtt_client->disconnectFromHost();
    }
}

QString WorkerMQTT::SaveImgToDisk(HObject &ho_image, QString fName, int num) {
    static unsigned int saveProcessNum = 0;
    static QString processSavePath;
    static int last_track_size = 0;
    QString rtn_name;

    bool isOK;
    QString pathName = QString("%1/%2_%3").arg(m_mqttSaveImagePath).arg(fName).arg(num);

    rtn_name = QString("%1_%2").arg(fName).arg(num);

    //不存在就创建
    isOK = DirExistEx(m_mqttSaveImagePath);
    if (isOK)
        WriteImage(ho_image, "jpeg", 0, pathName.toStdString().c_str());

    return rtn_name;
}

//单级文件夹是否存储，不存在则创建
bool WorkerMQTT::DirExist(QString fullPath) {
    QDir dir(fullPath);
    if (dir.exists()) {
        //存在当前文件夹
        return true;
    } else {
        //不存在则创建
        bool ok = dir.mkdir(fullPath); //只创建一级子目录，即必须保证上级目录存在
        return ok;
    }
}

//文件夹是否存储，不存在则创建，支持多级文件夹
bool WorkerMQTT::DirExistEx(QString fullPath) {
    QDir dir(fullPath);
    if (dir.exists()) {
        return true;
    } else {
        //不存在当前目录，创建，可创建多级目录
        bool ok = dir.mkpath(fullPath);
        return ok;
    }
}

void WorkerMQTT::mqtt_public_mydata(void) {
    MyApplication *pApp = (MyApplication *) qApp;
    NodeData *pData = pApp->pNodeData;
    int width, height;
    bool isOK;
    QString f_img_ori_name, f_img_pro_name;
    QString b_img_ori_name, b_img_pro_name;

    HObject img_front_pro, img_back_pro;

    static int img_save_num = 1;

    try {
        QMQTT::ConnectionState MQttState = p_mqtt_client->connectionState();
        if (m_mqttUse == 1 && MQttState == QMQTT::STATE_CONNECTED) {
            //判断图像是否为空，如果为空则按分辨率产生黑图像
            isOK = isHObjectEmpty(pData->ho_image_front_ori);
            if (isOK)
                GenImageConst(&pData->ho_image_front_ori, "byte", pData->setting.front_camera_res_width,
                              pData->setting.front_camera_res_height);
            //存储机前原始
            f_img_ori_name = SaveImgToDisk(pData->ho_image_front_ori, "head_front_ori", img_save_num);

            isOK = isHObjectEmpty(pData->ho_image_front_pro);
            width = AppConfig::f_crop_image_width_mm / AppConfig::f_each_pixel_equal_mm_width;
            height = AppConfig::f_crop_image_height_mm / AppConfig::f_each_pixel_equal_mm_height;
            if (isOK)
                GenImageConst(&pData->ho_image_front_pro, "byte", width, height);

            //存储机前处理
            f_img_pro_name = SaveImgToDisk(pData->ho_image_front_pro, "head_front_pro", img_save_num);

            isOK = isHObjectEmpty(pData->ho_image_back_ori);
            if (isOK)
                GenImageConst(&pData->ho_image_back_ori, "byte", pData->setting.back_camera_res_width,
                              pData->setting.back_camera_res_height);

            //存储机后原始
            b_img_ori_name = SaveImgToDisk(pData->ho_image_back_ori, "head_back_ori", img_save_num);

            isOK = isHObjectEmpty(pData->ho_image_back_pro);
            width = AppConfig::b_crop_image_width_mm / AppConfig::b_each_pixel_equal_mm_width;
            height = AppConfig::b_crop_image_height_mm / AppConfig::b_each_pixel_equal_mm_height;
            if (isOK)
                GenImageConst(&pData->ho_image_back_pro, "byte", width, height);
            //存储机后处理
            b_img_pro_name = SaveImgToDisk(pData->ho_image_back_pro, "camber_back_pro", img_save_num);

            QString str;
            //增加其它发送数据
            str = QString("%1,%2,%3,%4")
                    .arg(f_img_ori_name)
                    .arg(f_img_pro_name)
                    .arg(b_img_ori_name)
                    .arg(b_img_pro_name);

            QMQTT::ConnectionState MQttState = p_mqtt_client->connectionState();
            if (m_mqttUse == 1 && MQttState == QMQTT::STATE_CONNECTED) {
                QMQTT::Message msg = QMQTT::Message(0, pData->setting.mqtt_public_data, str.toStdString().c_str(), 0);
                int rtn_code = p_mqtt_client->publish(msg);
            }
        }

        //图像存储计数
        img_save_num++;
        if (img_save_num > m_mqttImageCycleNum)
            img_save_num = 1;

        //mqtt处理完成，重置标记
        pApp->pNodeData->isMQTTProcessing = false;
    } catch (HException &except) {
        HString str;
        str = except.ErrorMessage();
        QLOG_ERROR() << "mqtt_public_mydata->ErrorMessage:" << str.ToUtf8() << ",ErrorCode:" << except.ErrorCode();

        pApp->pNodeData->isMQTTProcessing = false;
    }
}
