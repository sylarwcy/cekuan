#ifndef APPEVENT_H
#define APPEVENT_H

#include <QObject>

class AppEvent : public QObject
{
    Q_OBJECT
public:
    static AppEvent *Instance();
    explicit AppEvent(QObject *parent = 0);

private:
    static QScopedPointer<AppEvent> self;

signals:
    void exitAll();
    void changeStyle();
    void fullScreen(bool full);

public slots:
    //软件退出
    void slot_exitAll();
    //全局样式改变
    void slot_changeStyle();

    //鼠标右键全屏切换信号
    void slot_fullScreen(bool full);
};

#endif // APPEVENT_H
