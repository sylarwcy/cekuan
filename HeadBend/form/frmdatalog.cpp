#include "frmdatalog.h"
#include "ui_frmdatalog.h"

frmDataLog::frmDataLog(QWidget *parent) : QWidget(parent), ui(new Ui::frmDataLog)
{
    ui->setupUi(this);
}

frmDataLog::~frmDataLog()
{
    delete ui;
}
