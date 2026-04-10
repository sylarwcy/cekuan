#ifndef FRMCONFIG_H
#define FRMCONFIG_H

#include <QWidget>
#include "frmconfig1.h"

class QAbstractButton;

namespace Ui {
class frmConfig;
}

class frmConfig : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfig(QWidget *parent = 0);
    ~frmConfig();

public:
    frmConfig1 *pfrmConfig1;

private:
    Ui::frmConfig *ui;

    //导航按钮+图标集合
    QList<int> icons;
    QList<QAbstractButton *> btns;

private slots:
    void initForm();        //初始化界面数据
    void fullScreen(bool full);

};

#endif // FRMCONFIG_H
