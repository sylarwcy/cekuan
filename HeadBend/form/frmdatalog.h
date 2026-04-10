#ifndef FRMDATALOG_H
#define FRMDATALOG_H

#include <QWidget>

namespace Ui {
class frmDataLog;
}

class frmDataLog : public QWidget
{
    Q_OBJECT

public:
    explicit frmDataLog(QWidget *parent = 0);
    ~frmDataLog();

private:
    Ui::frmDataLog *ui;
};

#endif // FRMDATALOG_H
