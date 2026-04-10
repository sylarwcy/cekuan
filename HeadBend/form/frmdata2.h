#ifndef FRMDATA2_H
#define FRMDATA2_H

#include <QWidget>

namespace Ui {
class frmData2;
}

class frmData2 : public QWidget
{
    Q_OBJECT

public:
    explicit frmData2(QWidget *parent = nullptr);
    ~frmData2();

private:
    Ui::frmData2 *ui;
};

#endif // FRMDATA2_H
