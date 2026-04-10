#include "frmdata2.h"
#include "ui_frmdata2.h"

frmData2::frmData2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmData2)
{
    ui->setupUi(this);
}

frmData2::~frmData2()
{
    delete ui;
}
