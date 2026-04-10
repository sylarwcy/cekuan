#ifndef FRMDATA1_H
#define FRMDATA1_H

#include <QSqlQueryModel>
#include <QWidget>

namespace Ui {
class frmData1;
}

class frmData1 : public QWidget
{
    Q_OBJECT

public:
    explicit frmData1(QWidget *parent = nullptr);
    ~frmData1();


private:
    Ui::frmData1 *ui;
};

#endif // FRMDATA1_H
