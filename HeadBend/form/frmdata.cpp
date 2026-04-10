#include "frmdata.h"
#include "ui_frmdata.h"
#include "quihelper.h"
#include "MyApplication.h"

frmData::frmData(QWidget *parent) : QWidget(parent), ui(new Ui::frmData)
{
    ui->setupUi(this);
    this->initForm();
    this->initNav();
    this->initIcon();
}

frmData::~frmData()
{
    delete ui;
}

void frmData::initForm()
{
    ui->widgetLeft->setProperty("flag", "btnNavLeft");
    ui->widgetLeft->setFixedWidth(AppNav::NavSubLeft ? 150 : 120);
}

void frmData::initNav()
{
    MyApplication *pApp=(MyApplication *)qApp;

    pfrmData1=new frmData1(this);
    pfrmData2=new frmData2(this);

    ui->stackedWidget->addWidget(pfrmData1);
    ui->stackedWidget->addWidget(pfrmData2);

    QList<QString> names, texts;
    names << "btnSearch" << "btnRunLog";
    texts << "工艺查询" << "运行记录";
    icons << 0xea44 << 0xea5b;

    //根据设定实例化导航按钮对象
    for (int i = 0; i < texts.count(); i++) {
        QToolButton *btn = new QToolButton;
        if (AppNav::NavSubLeft) {
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setMinimumHeight(30);
        } else {
            btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            btn->setMinimumHeight(80);
        }

        btn->setObjectName(names.at(i));
        btn->setText(" " + texts.at(i));
        btn->setCheckable(true);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(buttonClicked()));
        ui->layoutNav->addWidget(btn);
        btns << btn;
    }

    //底部加上弹簧顶上去
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->layoutNav->addItem(verticalSpacer);

    btns.at(0)->click();
}

void frmData::initIcon()
{
    int count = btns.count();
    for (int i = 0; i < count; ++i) {
        AppNav::initNavBtnIcon(btns.at(i), icons.at(i), true);
    }
}

void frmData::buttonClicked()
{
    //切换到当前窗体
    QAbstractButton *btn = (QAbstractButton *)sender();
    ui->stackedWidget->setCurrentIndex(btns.indexOf(btn));

    //取消其他按钮选中
    foreach (QAbstractButton *b, btns) {
        b->setChecked(b == btn);
    }
}
