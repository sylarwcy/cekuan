#ifndef APPKEY_H
#define APPKEY_H

#include <QObject>
#include <QDateTime>
#include <QTimer>

//1. 目前生成的是两种秘钥文件选其一即可。
//2. 机器码对应的秘钥目前只能在windows上。
//3. 等到其他系统研究好了如何获取唯一机器码再改进。
//4. 后面统一将两种秘钥逻辑合并，即可控制详细参数也能识别设备唯一编码。
//5. 统一合并生成一个秘钥文件字符串加密形式。
class AppKey : public QObject
{
    Q_OBJECT
public:
    //全局打印数据类
    static AppKey *appKey;
    static void initClass();

    //异或加密解密
    static QString getXorEncryptDecrypt(const QString &keyData, char keyCode);
    //秘钥内容写入到秘钥文件
    static void writeFile(const QString &fileName, const QByteArray &data);

    //生成异或加密秘钥文件
    //fileName      秘钥文件
    //keyCode       运算秘钥 必须和解密秘钥一致
    //keyUseDate    是否启用运行日期时间限制
    //keyDate       到期时间字符串
    //keyUseRun     是否启用可运行时间限制
    //keyRun        可运行时间 单位分钟
    //keyUseCount   是否启用设备数量限制
    //keyCount      设备限制数量
    static QString writeKey(const QString &fileName, char keyCode,
                            bool keyUseDate, const QString &keyDate,
                            bool keyUseRun, int keyRun,
                            bool keyUseCount, int keyCount);

    //生成机器码文件
    static QString writeLicense(const QString &fileName, const QString &machineCode);

    //通过编译器函数或者汇编获取CPU编号
    static void getCpuId(int cpuInfo[], int infoType);
    static QString getCpuId2();

    //执行wmic查询命令
    static QString getWMIC(const QString &cmd, int timeout = 1000);
    //获取机器码
    static QString getMachineCode();

    //获取cpu名称
    static QString getCpuName();
    //获取cpu编号
    static QString getCpuId();
    //获取主板编号
    static QString getBoardNum();
    //获取硬盘编号
    static QString getDiskNum();

    //定时器检查运行时间
    static void checkTime();
    //校验数量 一般在添加设备的地方调用此方法
    static bool checkCount(int count);
    //校验秘钥文件 传入秘钥文件和运算秘钥
    static bool checkKey(const QString &fileName, char keyCode);
    //校验机器码是否正确 另外一种方式秘钥校验机制
    static bool checkLicense(const QString &fileName);

    explicit AppKey(QObject *parent = 0);
    ~AppKey();

private:
    QTimer *timer;          //定时器判断是否运行超时
    QDateTime startTime;    //程序启动时间

public:
    QString keyData;        //注册码密文
    bool keyUseDate;        //是否启用运行日期时间限制
    QString keyDate;        //到期时间字符串
    bool keyUseRun;         //是否启用可运行时间限制
    int keyRun;             //可运行时间 单位分钟
    bool keyUseCount;       //是否启用设备数量限制
    int keyCount;           //设备限制数量

private slots:
    //定时器检查运行时间
    void checkTime1();
    //校验数量 一般在添加设备的地方调用此方法
    bool checkCount1(int count);
    //校验秘钥文件 传入秘钥文件和运算秘钥
    bool checkKey1(const QString &fileName, char keyCode);
    //校验机器码是否正确 另外一种方式秘钥校验机制
    bool checkLicense1(const QString &fileName);
};

#endif // APPKEY_H
