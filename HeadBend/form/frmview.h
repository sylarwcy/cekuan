#ifndef FRMVIEW_H
#define FRMVIEW_H

#include <QWidget>
#include "frmview1.h"
#include "frmview2.h"

namespace Ui {
class frmView;
}

class frmView : public QWidget
{
    Q_OBJECT

public:
    explicit frmView(QWidget *parent = 0);
    ~frmView();

private:
    Ui::frmView *ui;

    //导航按钮+图标集合
    QList<int> icons;
    QList<QAbstractButton *> btns;

public:
    frmView1 *pfrmView1;
    frmView2 *pfrmView2;

private slots:
    void initForm();
    void initNav();
    void initIcon();
    void buttonClicked();

};

#endif // FRMVIEW_H
