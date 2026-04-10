#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>

#include "frmview.h"
#include "frmdata.h"
#include "frmconfig.h"

class FramelessHelper;
class QAbstractButton;

namespace Ui
{
class frmMain;
}

class frmMain : public QWidget
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void closeEvent(QCloseEvent * event);

public:
    void initLogger();
    void destroyLogger();

private:
    Ui::frmMain *ui;

    //无边框拉伸类
    FramelessHelper *frameless;

    //导航按钮+图标集合
    QList<int> icons;
    QList<QAbstractButton *> btns;

public:
    frmConfig *pfrmConfig;
    frmData *pfrmData;
    frmView *pfrmView;

private slots:
    void initForm();        //初始化界面数据
    void initNav();         //初始化导航窗体
    void initIcon();        //初始化按钮图标
    void buttonClicked();   //按钮单击事件

    void initText();        //初始化标题等
    void initAction();      //增加换肤菜单
    void doAction();        //执行菜单操作

    void initProcess();     //初始化线程

    //全屏切换
    void fullScreen(bool full);


private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();

signals:
    void changeStyle();
    void sig_connect_db();
};

#endif // FRMMAIN_H
