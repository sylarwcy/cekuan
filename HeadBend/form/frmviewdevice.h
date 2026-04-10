#ifndef FRMVIEWDEVICE_H
#define FRMVIEWDEVICE_H

#include <QWidget>

namespace Ui {
class frmViewDevice;
}

class frmViewDevice : public QWidget
{
    Q_OBJECT

public:
    explicit frmViewDevice(QWidget *parent = 0);
    ~frmViewDevice();

private:
    Ui::frmViewDevice *ui;
};

#endif // FRMVIEWDEVICE_H
