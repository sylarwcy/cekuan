#ifndef FRMSHOWCURVE_H
#define FRMSHOWCURVE_H

#include <QDialog>
#include "qcustomplot.h"

class DbConnThread;

namespace Ui {
class frmShowCurve;
}

class frmShowCurve : public QDialog
{
    Q_OBJECT

public:
    explicit frmShowCurve(QWidget *parent = nullptr);
    ~frmShowCurve();

private:
    Ui::frmShowCurve *ui;

public slots:
    void DrawCurve(QString);
private slots:
    void on_pushButton_close_clicked();
};

#endif // FRMSHOWCURVE_H
