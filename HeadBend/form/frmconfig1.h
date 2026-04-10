#ifndef FRMCONFIG1_H
#define FRMCONFIG1_H

#include <QWidget>

namespace Ui {
class frmConfig1;
}

class frmConfig1 : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfig1(QWidget *parent = nullptr);
    ~frmConfig1();

private:
    Ui::frmConfig1 *ui;
};

#endif // FRMCONFIG1_H
