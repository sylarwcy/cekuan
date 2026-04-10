#include "frmconfig1.h"
#include "ui_frmconfig1.h"

frmConfig1::frmConfig1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmConfig1)
{
    ui->setupUi(this);
}

frmConfig1::~frmConfig1()
{
    delete ui;
}
