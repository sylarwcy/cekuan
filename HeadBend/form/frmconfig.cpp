#include "frmconfig.h"
#include "ui_frmconfig.h"
#include "quihelper.h"

frmConfig::frmConfig(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfig)
{
    ui->setupUi(this);
    this->initForm();

}

frmConfig::~frmConfig()
{
    delete ui;
}

void frmConfig::initForm()
{
    pfrmConfig1=new frmConfig1(this);
    ui->stackedWidget->addWidget(pfrmConfig1);

    connect(AppEvent::Instance(), SIGNAL(fullScreen(bool)), this, SLOT(fullScreen(bool)));
}

void frmConfig::fullScreen(bool full)
{

}



