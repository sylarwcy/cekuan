#include "appevent.h"
#include "quihelper.h"

QScopedPointer<AppEvent> AppEvent::self;
AppEvent *AppEvent::Instance()
{
    if (self.isNull()) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (self.isNull()) {
            self.reset(new AppEvent);
        }
    }

    return self.data();
}

AppEvent::AppEvent(QObject *parent) : QObject(parent)
{

}

void AppEvent::slot_exitAll()
{
    //退出前保存配置文件


    //发送退出信号
    emit exitAll();
    //延时一点时间给处理
    QUIHelper::sleep(100);
}

void AppEvent::slot_changeStyle()
{
    emit changeStyle();
}

void AppEvent::slot_fullScreen(bool full)
{
    emit fullScreen(full);
}
