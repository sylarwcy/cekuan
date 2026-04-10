#ifndef FRMDATA_H
#define FRMDATA_H

#include <QWidget>
#include "frmdata1.h"
#include "frmdata2.h"

class QAbstractButton;

namespace Ui {
class frmData;
}

class frmData : public QWidget
{
    Q_OBJECT

public:
    explicit frmData(QWidget *parent = 0);
    ~frmData();

private:
    Ui::frmData *ui;

    //导航按钮+图标集合
    QList<int> icons;
    QList<QAbstractButton *> btns;

public:
    frmData1 *pfrmData1;
    frmData2 *pfrmData2;

private slots:
    void initForm();        //初始化界面数据
    void initNav();         //初始化导航窗体
    void initIcon();        //初始化按钮图标
    void buttonClicked();   //按钮单击事件
};

#endif // FRMDATA_H
