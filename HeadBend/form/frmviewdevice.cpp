#include "frmviewdevice.h"
#include "ui_frmviewdevice.h"
#include "head.h"

frmViewDevice::frmViewDevice(QWidget *parent) : QWidget(parent), ui(new Ui::frmViewDevice)
{
    ui->setupUi(this);
    //关联右键菜单全屏切换信号
    connect(ui->videoPanel, SIGNAL(fullScreen(bool)), AppEvent::Instance(), SIGNAL(fullScreen(bool)));
}

frmViewDevice::~frmViewDevice()
{
    delete ui;
}
