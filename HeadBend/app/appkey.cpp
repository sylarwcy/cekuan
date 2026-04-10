#pragma execution_character_set("utf-8")
#include "appkey.h"
#include "qmutex.h"
#include "qfile.h"
#include "qapplication.h"
#include "qmessagebox.h"
#include "qprocess.h"
#include "qcryptographichash.h"
#include "qclipboard.h"
#include "qdebug.h"

AppKey *AppKey::appKey = 0;
void AppKey::initClass()
{
    //没有实例化过则先实例化
    if (appKey == 0) {
        appKey = new AppKey;
    }
}

QString AppKey::getXorEncryptDecrypt(const QString &keyData, char keyCode)
{
    //采用异或加密,也可以自行更改算法
    QByteArray buffer = keyData.toLatin1();
    int size = buffer.size();
    for (int i = 0; i < size; i++) {
        buffer[i] = buffer.at(i) ^ keyCode;
    }

    return QLatin1String(buffer);
}

void AppKey::writeFile(const QString &fileName, const QByteArray &data)
{
    QFile file(fileName);
    file.open(QFile::WriteOnly | QIODevice::Text);
    file.write(data);
    file.close();
}

QString AppKey::writeKey(const QString &fileName, char keyCode,
                         bool keyUseDate, const QString &keyDate,
                         bool keyUseRun, int keyRun,
                         bool keyUseCount, int keyCount)
{
    QString keyData = QString("%1|%2|%3|%4|%5|%6")
                      .arg(keyUseDate).arg(keyDate)
                      .arg(keyUseRun).arg(keyRun)
                      .arg(keyUseCount).arg(keyCount);
    keyData = getXorEncryptDecrypt(keyData, keyCode);

    QByteArray data = keyData.toLatin1();
    writeFile(fileName, data);
    return QString(data);
}

QString AppKey::writeLicense(const QString &fileName, const QString &machineCode)
{
    //MD5加密,也可以选择其他加密方式
    QByteArray data = QCryptographicHash::hash(machineCode.toLatin1(), QCryptographicHash::Md5);
    data = data.toHex().toUpper();
    writeFile(fileName, data);
    return QString(data);
}

void AppKey::getCpuId(int cpuInfo[], int infoType)
{
#if 0
#ifdef Q_OS_WIN
    //__cpuid貌似针对的是因特尔的CPU
    //此方法有些低版本的编译器没有 需要引入头文件 #include "intrin.h"
    //__cpuid(cpuInfo, infoType);return;
#if _MSC_VER
    int excValue = 0;
    _asm {
        //读取参数到寄存器
        mov edi, cpuInfo;
        mov eax, infoType;
        mov ecx, excValue;
        cpuid;
        //将寄存器保存
        mov [edi], eax;
        mov [edi+4], ebx;
        mov [edi+8], ecx;
        mov [edi+12], edx;
    }
#else
    //下面这段代码就是从 __cpuid 函数抠出来的 因为早期的mingw没有这个函数
    asm volatile
    (
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(infoType)
    );
#endif
#endif
#endif
}

QString AppKey::getCpuId2()
{
    QString cpu_id;
    int cpuInfo[4] = {0};
    getCpuId(cpuInfo, 1);

    //必须转换成 quint32 否则64位的不准确
    QString str0 = QString::number((quint32)cpuInfo[3], 16).toUpper();
    str0 = str0.rightJustified(8, '0');
    QString str1 = QString::number((quint32)cpuInfo[0], 16).toUpper();
    str1 = str1.rightJustified(8, '0');
    cpu_id = str0 + str1;
    return cpu_id;
}

QString AppKey::getWMIC(const QString &cmd, int timeout)
{
    //获取cpu名称：wmic cpu get Name
    //获取cpu核心数：wmic cpu get NumberOfCores
    //获取cpu线程数：wmic cpu get NumberOfLogicalProcessors
    //查询cpu序列号：wmic cpu get processorid
    //查询主板序列号：wmic baseboard get serialnumber
    //查询BIOS序列号：wmic bios get serialnumber
    //查看硬盘编号：wmic diskdrive get serialnumber

    QProcess p;
    p.start(cmd);
    //等待完成带超时时间
    p.waitForFinished(timeout);
    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());
    QStringList list = cmd.split(" ");
    QString result = output.remove(list.last(), Qt::CaseInsensitive);
    result.replace("\r", "");
    result.replace("\n", "");
    result = result.simplified();
    result = result.trimmed();
    qDebug() << cmd << output << result;
    return result;
}

QString AppKey::getMachineCode()
{
    //获取CPU编号
    QString cpuid = getCpuId();
    if (cpuid.length() <= 4) {
        cpuid = getCpuId2();
    }

    if (cpuid.length() <= 4) {
        QMessageBox::critical(0, "错误", "获取CPU编号失败!");
        return "";
    }

    //获取硬盘编号
    QString number = getBoardNum();
    //硬盘编号可能在虚拟机中拿到的是None
    //改成取CPU编号倒过来
    if (number.toLower().length() <= 4) {
        QStringList list;
        for (int i = cpuid.length() - 1; i >= 0; --i) {
            list << cpuid.at(i);
        }
        number = list.join("");
    }

    QStringList keys;
    int count1 = cpuid.count();
    int count2 = number.count();
    int count = qMin(count1, count2);
    for (int i = 0; i < count; i++) {
        keys << QString("%1%2").arg(cpuid.at(i)).arg(number.at(i));
    }

    //拿到key大写并去除空格
    QString key = keys.join("").toUpper();
    if (key.contains(" ")) {
        QStringList list;
        for (int i = 0; i < key.count(); i++) {
            if (key.at(i) != ' ') {
                list << key.at(i);
            }
        }

        key = list.join("");
    }

    return key;
}

QString AppKey::getCpuName()
{
    return getWMIC("wmic cpu get name");
}

QString AppKey::getCpuId()
{
    return getWMIC("wmic cpu get processorid");
}

QString AppKey::getBoardNum()
{
    return getWMIC("wmic baseboard get serialnumber");
}

QString AppKey::getDiskNum()
{
    return getWMIC("wmic diskdrive where index=0 get serialnumber");
}

void AppKey::checkTime()
{
    initClass();
    appKey->checkTime1();
}

bool AppKey::checkCount(int count)
{
    initClass();
    return appKey->checkCount1(count);
}

bool AppKey::checkKey(const QString &fileName, char keyCode)
{
    initClass();
    return appKey->checkKey1(fileName, keyCode);
}

bool AppKey::checkLicense(const QString &fileName)
{
    initClass();
    return appKey->checkLicense1(fileName);
}


AppKey::AppKey(QObject *parent) : QObject(parent)
{
    keyData = "";
    keyUseDate = false;
    keyDate = "1970-01-01";
    keyUseRun = false;
    keyRun = 1;
    keyUseCount = false;
    keyCount = 10;

    //定时器判断运行时间
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkTime1()));
    startTime = QDateTime::currentDateTime();
}

AppKey::~AppKey()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

void AppKey::checkTime1()
{
    //找出当前时间与首次启动时间比较
    QDateTime now = QDateTime::currentDateTime();
    if (startTime.secsTo(now) >= (keyRun * 60)) {
        timer->stop();
        QString msg = QString("试运行时间已到, 共运行 [ %1 ] 分钟.\n请联系供应商更新秘钥文件!").arg(keyRun);
        QMessageBox::critical(0, "错误", msg);
        //直接不讲情面的暴力退出
        exit(0);
    }
}

bool AppKey::checkCount1(int count)
{
    if (keyUseCount && count >= keyCount) {
        QString msg = QString("数量超过限制, 允许最大数量 [ %1 ].\n请联系供应商更新秘钥文件!").arg(keyCount);
        QMessageBox::critical(0, "错误", msg);
        return false;
    }

    return true;
}

bool AppKey::checkKey1(const QString &fileName, char keyCode)
{
    //判断密钥文件是否存在,不存在则从资源文件复制出来,同时需要设置文件写权限
    QFile keyFile(fileName);
    if (!keyFile.exists() || keyFile.size() == 0) {
        QString msg = QString("秘钥文件 %1 丢失,请联系供应商!").arg(fileName);
        QMessageBox::critical(0, "错误", msg);
        return false;
    }

    //读取密钥文件
    keyFile.open(QFile::ReadOnly);
    keyData = keyFile.readLine();
    keyFile.close();

    //将从注册码文件中的密文解密
    keyData = getXorEncryptDecrypt(keyData, keyCode);
    QStringList data = keyData.split("|");
    if (data.count() != 6) {
        QMessageBox::critical(0, "错误", "秘钥文件已损坏,程序将自动关闭!");
        return false;
    }

    //挨个取出对应项的内容
    keyUseDate = (data.at(0) == "1");
    keyDate = data.at(1);
    keyUseRun = (data.at(2) == "1");
    keyRun = data.at(3).toInt();
    keyUseCount = (data.at(4) == "1");
    keyCount = data.at(5).toInt();

    //如果启用了时间限制
    if (keyUseDate) {
        QString nowDate = QDate::currentDate().toString("yyyy-MM-dd");
        if (nowDate > keyDate) {
            QString msg = QString("软件已到期, 到期时间 [ %1 ].\n请联系供应商更新秘钥文件!").arg(keyDate);
            QMessageBox::critical(0, "错误", msg);
            return false;
        }
    }

    //如果启用了运行时间显示
    timer->stop();
    if (keyUseRun) {
        timer->start();
    }

    return true;
}

bool AppKey::checkLicense1(const QString &fileName)
{
    QFile keyFile(fileName);
    if (!keyFile.exists() || keyFile.size() == 0) {
        QString msg = QString("秘钥文件 %1 丢失,请联系供应商!").arg(fileName);
        QMessageBox::critical(0, "错误", msg);
        return false;
    }

    //读取密钥文件
    keyFile.open(QFile::ReadOnly);
    keyData = keyFile.readLine();
    keyFile.close();

    //获取机器码
    QString key = getMachineCode();
    //MD5加密,也可以选择其他加密方式
    QByteArray result = QCryptographicHash::hash(key.toLatin1(), QCryptographicHash::Md5).toHex().toUpper();
    if (result != keyData) {
        //自动复制一下等待粘贴到秘钥工具生成秘钥
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(key);
        QString msg = QString("秘钥文件错误,请联系供应商! 已自动复制机器码!\n机器码: %1").arg(key);
        QMessageBox::critical(0, "错误", msg);
        return false;
    }

    return true;
}
