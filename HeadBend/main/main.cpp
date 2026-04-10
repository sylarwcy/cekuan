#include "frmmain.h"
#include "appinit.h"
#include "appconfig.h"
#include "quihelper.h"

#include "MyApplication.h"

#include <QSystemSemaphore>
#include <QSharedMemory>

#include "imgAI.h"
#include "myCameraGigE.h"

#define display true

int main(int argc, char* argv[])
{

    //****************************************************************************************************
    // HObject img;
    // ReadImage(&img,"test.jpg");
    // WriteImage(img,"jpeg",0,"\\\\192.168.0.195/Temp/staging_test.jpg");
    // _sleep(2000);
    // return 0;
    //****************************************************************************************************

    QString exeDir = QCoreApplication::applicationDirPath();
    QByteArray exeDirBytes = QDir::toNativeSeparators(exeDir).toLocal8Bit();
    bool success = qputenv("HALCONROOT", exeDirBytes);

#if defined(_WIN32)
    SetSystem("use_window_thread", "true");
#endif

    // file was stored with local-8-bit encoding
    //   -> set the interface encoding accordingly
    SetHcppInterfaceStringEncodingIsUtf8(false);

    // Default settings used in HDevelop (can be omitted)
    SetSystem("width", 2000);
    SetSystem("height", 900);
    //
    // try {
    //     HObject imgPart;
    //     ReadImage(&imgPart,"test_bug.jpg");
    //     QString imgNameLocalCrop = QString("%1/test_%2.jpg").arg("\\\\192.168.0.196/Temp/").arg("test");
    //     WriteImage(imgPart, "jpeg", 0, imgNameLocalCrop.toStdString().c_str());
    // }catch (HException &exception){
    //     QLOG_INFO("图片传不出去了");
    // }


    // 注册 Halcon 类型
    qRegisterMetaType<HalconCpp::HObject>("HalconCpp::HObject");
    qRegisterMetaType<HalconCpp::HTuple>("HalconCpp::HTuple");
    //设置不应用操作系统设置比如字体
    QApplication::setDesktopSettingsAware(false);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);

    //应用实例
    MyApplication a(argc, argv);

    //运行一次
    QSystemSemaphore sema("QT_AppFrame", 1, QSystemSemaphore::Open);
    //在临界区操作共享内存SharedMemory
    sema.acquire();
    //全局对象名
    QSharedMemory mem("QT_AppFrameObject");
    //如果全局对象以存在则退出
    if (!mem.create(1))
    {
        qDebug() << mem.errorString();
        sema.release();

        return 0;
    }
    sema.release();

    //应用设定
    AppInit::Instance()->start();
    AppInit::Instance()->initStyle(":/qss/silvery.css");

    //读取相机标定设置
    AppConfig::readConfig();

    frmMain w;
    w.show();


    return a.exec();
}

