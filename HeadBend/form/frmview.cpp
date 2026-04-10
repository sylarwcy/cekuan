#include "frmview.h"
#include "ui_frmview.h"
#include "quihelper.h"
#include "appinit.h"

frmView::frmView(QWidget *parent) : QWidget(parent), ui(new Ui::frmView)
{
    ui->setupUi(this);

    this->initForm();
    this->initNav();
    this->initIcon();
}

frmView::~frmView()
{
    delete ui;
}

void frmView::initForm()
{
    ui->widgetLeft->setProperty("flag", "btnNavLeft");
    ui->widgetLeft->setFixedWidth(AppNav::NavSubLeft ? 150 : 120);
}

void frmView::initNav()
{
    MyApplication *pApp=(MyApplication *)qApp;

    pfrmView1=new frmView1(this);
    pfrmView2=new frmView2(this);

    ui->stackedWidget->addWidget(pfrmView1);
    ui->stackedWidget->addWidget(pfrmView2);

    QList<QString> names, texts;
    names << "btnImageProcess" << "btnImageCali";
    texts << "图像处理" << "图像标定";
    icons << 0xe671 << 0xe9e8;

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

    //增加刷新信号
    for (int i = 0; i < pApp->m_workstationList.size(); ++i)
        connect(pApp->m_workstationList[i]->p_worker_trigger,SIGNAL(sig_refreshDisplay()), pfrmView1, SLOT(refreshDataDisp()));

}

void frmView::initIcon()
{
    int count = btns.count();
    for (int i = 0; i < count; ++i) {
        AppNav::initNavBtnIcon(btns.at(i), icons.at(i), true);
    }
}

void frmView::buttonClicked()
{
    //切换到当前窗体
    QAbstractButton *btn = (QAbstractButton *)sender();
    ui->stackedWidget->setCurrentIndex(btns.indexOf(btn));

    //取消其他按钮选中
    foreach (QAbstractButton *b, btns) {
        b->setChecked(b == btn);
    }
}
