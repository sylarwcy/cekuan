#include "frmdata1.h"
#include "ui_frmdata1.h"

//////////////////////////////////////////////////////////////////////////////////////////
frmData1::frmData1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmData1)
{
    ui->setupUi(this);

}

frmData1::~frmData1()
{
    delete ui;
}



