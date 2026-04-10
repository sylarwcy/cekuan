#include "dbhttpthread.h"

QMap<QString, QString> DbHttpThread::getTables1()
{
    QMap<QString, QString> tables;
    QStringList list;

    //初始化表名+字段名集合 方便用于解析数据
    //如果有自定义的表和字段只需要在这里增加即可
    tables.insert("t_1_1_mold_prod_total", "name,prod_1,prod_2");
    tables.insert("t_1_2_mold_prod_monthly", "year,month,prod_1,prod_2,prod_3");
    tables.insert("t_1_3_wp_prod_total", "name,prod_1,prod_2");
    tables.insert("t_1_4_wp_prod_monthly", "year,month,prod_1,prod_2,prod_3");

    tables.insert("t_2_1_mold_achie_rate", "plan,achieved");
    tables.insert("t_2_2_wp_achie_rate", "name,plan,achieved");
    tables.insert("t_2_3_wp_achie_number", "wp_achie_number");
    tables.insert("t_2_4_process_achie_number", "day,green,blue,red");

    tables.insert("t_3_1_device_runtime", "group_name,no_id,name,text_1,text_2,status");
    tables.insert("t_3_2_oee", "cnc,edm,wedm");

    tables.insert("t_4_1_mold_progress", "internal_id,mold_no,tn_no,type,status,product_name,plan_date,green,yellow,red");
    tables.insert("t_4_2_mold_status_parcent", "finished,processing,delay");
    tables.insert("t_4_3_mold_processing_num", "mold_processing_number");

    list.clear();
    list << "process_name";
    for (int i = 1; i <= 7; i++) {
        list << QString("work_load_%1").arg(i);
    }
    tables.insert("t_5_1_work_load", list.join(","));

    list.clear();
    for (int i = 1; i <= 7; i++) {
        list << QString("date_%1").arg(i);
    }
    tables.insert("t_5_1_work_load_table_head", list.join(","));

    list.clear();
    list << "group_name";
    for (int i = 1; i <= 7; i++) {
        list << QString("day_%1").arg(i);
    }
    tables.insert("t_5_3_work_load_percent", list.join(","));

    list.clear();
    for (int i = 1; i <= 15; i++) {
        list << QString("date_%1").arg(i);
    }

    tables.insert("t_6_1_wp_qual_rate", list.join(","));
    tables.insert("t_6_2_ele_qual_rate", list.join(","));
    tables.insert("t_6_3_qual_rate_today", "qual_rate");
    tables.insert("t_6_4_mold_qual_rate", "mold_name,qual_rate");

    tables.insert("t_7_1_qual_percent", "green,blue,yellow,red");
    tables.insert("t_7_2_group_qual_rate", "group_name,qual_rate");

    list.clear();
    list << "group_name";
    for (int i = 1; i <= 31; i++) {
        list << QString("day_%1").arg(i);
    }
    tables.insert("t_7_3_qual_rate_daily", list.join(","));

    tables.insert("t_8_1_key_invt", "name,upper_bound,current");
    tables.insert("t_8_2_invt_table", "internal_id,invt_no,name,spec,mat,size,target,current");
    return tables;
}

QMap<QString, QString> DbHttpThread::getTables2()
{
    QMap<QString, QString> tables;
    tables.insert("UserInfo", "UserName,UserPwd,UserType,Permission1,Permission2,Permission3,Permission4,Permission5,Permission6,Permission7,UserMark");
    tables.insert("MaterialInfo", "MaterialID,MaterialNumber,MaterialName,MaterialPara1,MaterialPara2,MaterialPara3,MaterialPara4,MaterialPara5,MaterialPara6,MaterialPara7,MaterialPara8,MaterialPara9,MaterialPara10,MaterialMark");
    tables.insert("MaterialData1", "DataID,MaterialFlag,DataX,DataY");
    tables.insert("MaterialData2", "DataID,MaterialFlag,DataX,DataY");
    tables.insert("PartInfo", "PartID,PartNumber,PartName,MaterialName,PartSize,PartImage,PartImagex,PartModel,PartModelx,PartMark");
    tables.insert("MoldInfo", "MoldID,MoldType,MoldName,MoldState,MoldImage,MoldImagex,MoldModel,MoldModelx,MoldMark");
    tables.insert("QualityInfo", "QualityID,PartName,DefectType,DefectReason,QualityImage,QualityImagex,QualityModel,QualityModelx,QualityMark");
    tables.insert("ProcessInfo", "ProcessID,ProcessName,ProcessImage,ProcessImagex,ProcessMark");
    tables.insert("ProcessPlanInfo", "ProcessPlanID,ProcessPlanName,ProcessPlanType,PartName,ProcessPlanMark");
    tables.insert("ProcedureInfo", "ProcedureID,ProcessPlanName,ProcedureIndex,ProcedureName,ProcedureImage,ProcedureImagex,ProcedureModel,ProcedureModelx,ProcessName,MoldName,ProcedureRequire,ProcedurePara,ProcedureMark");
    return tables;
}

DbHttpThread::DbHttpThread(QObject *parent) : QObject(parent)
{
    url = "http://127.0.0.1:6000";

    //实例化网络请求通信对象
    manager = new QNetworkAccessManager(this);
    //绑定数据返回信号
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finished(QNetworkReply *)));

    //启动定时器排队处理队列中的请求
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(takeCmd()));
    timer->start(100);

    setTables(getTables1());
}

DbHttpThread::~DbHttpThread()
{

}

QString DbHttpThread::getUseTime()
{
    return QString::number((double)time.elapsed() / 1000, 'f', 3);
}

void DbHttpThread::finished(QNetworkReply *reply)
{
    //请求失败则发出错误信号
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::InternalServerError) {
#else
    if (reply->error() != QNetworkReply::NoError) {
#endif
        emit error(reply->errorString());
    }

    if (reply->bytesAvailable() > 0) {
        //统计用时
        QElapsedTimer time;
        time.start();

        QNetworkAccessManager::Operation type = reply->operation();
        QString url = reply->url().toString();
        QByteArray buffer = reply->readAll();
        //释放对象
        reply->deleteLater();
        //qDebug() << TIMEMS << url << buffer;
        //发送接收数据信号
        emit debug("收到数据返回");

        //判断是post还是get返回的数据
        if (type == QNetworkAccessManager::GetOperation) {

        } else if (type == QNetworkAccessManager::PostOperation) {
            QString tag;
            QStringList data;
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
            //初始化脚本引擎
            QScriptEngine engine;
            //构建解析对象
            QScriptValue script = engine.evaluate("value=" + buffer);
            if (!script.isObject()) {
                emit error(QString("未知数据( %1 )").arg(QString(buffer)));
                return;
            }

            //取出tag标识符用来传出去
            tag = script.property("tag").toString();
            //取出数据结果数组
            QScriptValue result = script.property("result");
            if (!result.isArray()) {
                emit error(QString("未知数据( %1 )").arg(QString(buffer)));
                return;
            }

            //创建迭代器逐个解析具体值
            QScriptValueIterator it(result);
            while (it.hasNext()) {
                it.next();
                if (it.flags() & QScriptValue::SkipInEnumeration) {
                    continue;
                }

                //用标识符tag取出当前回复数据对应字段名
                QString value = tables.value(tag);
                if (value.isEmpty()) {
                    continue;
                }

                QStringList columnNames = value.split(",");
                foreach (QString columnName, columnNames) {
                    data << it.value().property(columnName).toString();
                }
            }
#else
            //采用qt内置的json方法解析
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(buffer, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                emit error(QString("未知数据( %1 )").arg(QString(buffer)));
                return;
            }

            QJsonObject rootObj = jsonDoc.object();
            //qDebug() << TIMEMS << rootObj.keys();
            tag = rootObj.value("tag").toString();
            if (!rootObj.contains("result")) {
                emit error(QString("未知数据( %1 )").arg(QString(buffer)));
                return;
            }

            QJsonArray result = rootObj.value("result").toArray();
            for (int i = 0; i < result.count(); i++) {
                QJsonObject subObj = result.at(i).toObject();
                //用标识符tag取出当前回复数据对应字段名
                QString value = tables.value(tag);
                if (value.isEmpty()) {
                    continue;
                }

                QStringList columnNames = value.split(",");
                //qDebug() << TIMEMS << columnNames << subObj;
                foreach (QString columnName, columnNames) {
                    //需要进行格式判断再进行转换
                    QJsonValue jsonValue = subObj.value(columnName);
                    if (jsonValue.isDouble()) {
                        data <<  QString::number(jsonValue.toDouble());
                    } else {
                        data <<  jsonValue.toString();
                    }
                }
            }
#endif
            //qDebug() << TIMEMS << tag << data;
            if (data.count() > 0) {
                emit receiveData(tag, data, time.elapsed());
            }
        }
    }
}

void DbHttpThread::setUrl(const QString &url)
{
    if (!url.isEmpty() && url.startsWith("http")) {
        this->url = url;
    }
}

QString DbHttpThread::getUrl() const
{
    return this->url;
}

void DbHttpThread::setTables(const QMap<QString, QString> &tables)
{
    this->tables = tables;
}

void DbHttpThread::takeCmd()
{
    //取出队列sql语句执行
    if (tags.count() > 0) {
        //从最前面开始取
        mutex.lock();
        QString tag = tags.takeFirst();
        QString cmd = cmds.takeFirst();
        mutex.unlock();
        //qDebug() << TIMEMS << tag << cmd;

        //地址可以自行更改
        QNetworkRequest request;
        request.setUrl(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        //发送请求
        QString buffer = QString("tag=%1&%2").arg(tag).arg(cmd);
        manager->post(request, buffer.toUtf8());
        emit debug(QString("请求数据( %1 )").arg(buffer));
    }
}

void DbHttpThread::select(const QString &tableName, const QString &columnName, bool append)
{
    //超过队列中最大数量限制则无需处理
    if (tags.count() > 100) {
        return;
    }

    //可以自行调整cmd格式比如增加时间或者数量等条件
    QString cmd = QString("tableName=%1&columnName=%2").arg(tableName).arg(columnName);

    //如果是append表示追加在队列末尾,否则插到最前面优先级最高
    mutex.lock();
    if (append) {
        tags << tableName;
        cmds << cmd;
    } else {
        tags.insert(0, tableName);
        cmds.insert(0, cmd);
    }
    mutex.unlock();
}

void DbHttpThread::post(const QString &data, const QString &userName, const QString &userPwd, const QString &userKey)
{
    //地址可以自行更改
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //设置用户认证信息
    request.setRawHeader("username", userName.toUtf8());
    request.setRawHeader("userpwd", userPwd.toUtf8());
    request.setRawHeader("userkey", userKey.toUtf8());

    //发送请求
    manager->post(request, data.toUtf8());
    emit debug(QString("请求数据( %1 )").arg(data));
}
