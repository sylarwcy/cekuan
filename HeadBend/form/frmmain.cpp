#include "frmmain.h"
#include "ui_frmmain.h"
#include "quihelper.h"
#include "appinit.h"
#include "appconfig.h"
#include "framelesshelper.h"

#include "MyApplication.h"

frmMain::frmMain(QWidget *parent) : QWidget(parent), ui(new Ui::frmMain)
{
    ui->setupUi(this);

    MyApplication *pApp=(MyApplication *)qApp;
    NodeData *pNodeData=pApp->pNodeData;

    this->initProcess();

    this->initForm();
    this->initText();
    this->initNav();
    this->initIcon();
    this->initAction();


    QUIHelper::setFormInCenter(this);
    on_btnMenu_Max_clicked();

    //初始化日志
    initLogger();

    //显示环境变量
    QLOG_INFO()<<QString("环境变量 local_window_name=%1").arg(pNodeData->setting.local_window_name);
    QLOG_INFO()<<QString("环境变量 comm_window_name=%1").arg(pNodeData->setting.comm_window_name);

    QLOG_INFO()<<QString("环境变量 l2_plc_share_mem_name=%1").arg(pNodeData->setting.to_plc_share_mem_name);
    QLOG_INFO()<<QString("环境变量 plc_l2_share_mem_name=%1").arg(pNodeData->setting.from_plc_share_mem_name);

    QLOG_INFO()<<QString("环境变量 data_refresh_ms=%1").arg(pNodeData->setting.data_refresh_ms);
    QLOG_INFO()<<QString("环境变量 sample_period=%1").arg(pNodeData->setting.sample_period);

    QLOG_INFO()<<QString("环境变量 test_mode=%1").arg(pNodeData->setting.test_mode);
    QLOG_INFO()<<QString("环境变量 write_rot_video=%1").arg(pNodeData->setting.write_rot_video);

    QLOG_INFO()<<QString("环境变量 DbType1=%1").arg(pNodeData->setting.DbType1);
    QLOG_INFO()<<QString("环境变量 DbName1=%1").arg(pNodeData->setting.DbName1);
    QLOG_INFO()<<QString("环境变量 HostName1=%1").arg(pNodeData->setting.HostName1);
    QLOG_INFO()<<QString("环境变量 HostPort1=%1").arg(pNodeData->setting.HostPort1);
    QLOG_INFO()<<QString("环境变量 UserName1=%1").arg(pNodeData->setting.UserName1);
    QLOG_INFO()<<QString("环境变量 UserPwd1=%1").arg(pNodeData->setting.UserPwd1);

    QLOG_INFO()<<QString("环境变量 front_ip_addr=%1").arg(pNodeData->setting.front_ip_addr);
    QLOG_INFO()<<QString("环境变量 front_expTime=%1").arg(pNodeData->setting.front_expTime);
    QLOG_INFO()<<QString("环境变量 front_gain=%1").arg(pNodeData->setting.front_gain);
    QLOG_INFO()<<QString("环境变量 front_frameRate=%1").arg(pNodeData->setting.front_frameRate);
    QLOG_INFO()<<QString("环境变量 front_camera_res_width=%1").arg(pNodeData->setting.front_camera_res_width);
    QLOG_INFO()<<QString("环境变量 front_camera_res_height=%1").arg(pNodeData->setting.front_camera_res_height);

    QLOG_INFO()<<QString("环境变量 back_ip_addr=%1").arg(pNodeData->setting.back_ip_addr);
    QLOG_INFO()<<QString("环境变量 back_expTime=%1").arg(pNodeData->setting.back_expTime);
    QLOG_INFO()<<QString("环境变量 back_gain=%1").arg(pNodeData->setting.back_gain);
    QLOG_INFO()<<QString("环境变量 back_frameRate=%1").arg(pNodeData->setting.back_frameRate);
    QLOG_INFO()<<QString("环境变量 back_camera_res_width=%1").arg(pNodeData->setting.back_camera_res_width);
    QLOG_INFO()<<QString("环境变量 back_camera_res_height=%1").arg(pNodeData->setting.back_camera_res_height);

    QLOG_INFO()<<QString("环境变量 calibrate_path=%1").arg(pNodeData->setting.calibrate_path);
    QLOG_INFO()<<QString("环境变量 test_path=%1").arg(pNodeData->setting.test_path);
    QLOG_INFO()<<QString("环境变量 save_path=%1").arg(pNodeData->setting.save_path);

    QLOG_INFO()<<QString("---------------------------------------------------------");
    double x1,x2,x3,x4;

    x1=pApp->f_hv_x0[0].D();
    x2=pApp->f_hv_x0[1].D();
    x3=pApp->f_hv_x0[2].D();
    x4=pApp->f_hv_x0[3].D();
    QLOG_INFO()<<QString("机前投影变换参数:x0=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->f_hv_y0[0].D();
    x2=pApp->f_hv_y0[1].D();
    x3=pApp->f_hv_y0[2].D();
    x4=pApp->f_hv_y0[3].D();
    QLOG_INFO()<<QString("机前投影变换参数:y0=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->f_hv_x1[0].D();
    x2=pApp->f_hv_x1[1].D();
    x3=pApp->f_hv_x1[2].D();
    x4=pApp->f_hv_x1[3].D();
    QLOG_INFO()<<QString("机前投影变换参数:x1=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->f_hv_y1[0].D();
    x2=pApp->f_hv_y1[1].D();
    x3=pApp->f_hv_y1[2].D();
    x4=pApp->f_hv_y1[3].D();
    QLOG_INFO()<<QString("机前投影变换参数:y1=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);

    x1=pApp->b_hv_x0[0].D();
    x2=pApp->b_hv_x0[1].D();
    x3=pApp->b_hv_x0[2].D();
    x4=pApp->b_hv_x0[3].D();
    QLOG_INFO()<<QString("机后投影变换参数:x0=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->b_hv_y0[0].D();
    x2=pApp->b_hv_y0[1].D();
    x3=pApp->b_hv_y0[2].D();
    x4=pApp->b_hv_y0[3].D();
    QLOG_INFO()<<QString("机后投影变换参数:y0=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->b_hv_x1[0].D();
    x2=pApp->b_hv_x1[1].D();
    x3=pApp->b_hv_x1[2].D();
    x4=pApp->b_hv_x1[3].D();
    QLOG_INFO()<<QString("机后投影变换参数:x1=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);
    x1=pApp->b_hv_y1[0].D();
    x2=pApp->b_hv_y1[1].D();
    x3=pApp->b_hv_y1[2].D();
    x4=pApp->b_hv_y1[3].D();
    QLOG_INFO()<<QString("机后投影变换参数:y1=[%1,%2,%3,%4]").arg(x1).arg(x2).arg(x3).arg(x4);

    QLOG_INFO()<<QString("---------------------------------------------------------");

    //连接数据库
    emit sig_connect_db();

}

frmMain::~frmMain()
{
    MyApplication *pApp=(MyApplication *)qApp;

    delete ui;

    //删除日志
    destroyLogger();
}

void frmMain::initLogger()
{
    Logger& logger = Logger::instance();
    //这样和槽函数连接
    DestinationPtr sigsSlotDestination(DestinationFactory::MakeFunctorDestination(pfrmView->pfrmView1, SLOT(logSlot(QString,int))));
    logger.addDestination(sigsSlotDestination);
}

void frmMain::destroyLogger()
{
    QsLogging::Logger::destroyInstance();
}

bool frmMain::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        //双击标题栏最大化
        if (watched == ui->widgetTop) {
            on_btnMenu_Max_clicked();
        }
    }

    return QWidget::eventFilter(watched, event);
}

//防止误关闭程序
void frmMain::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void frmMain::initProcess()
{
    MyApplication *pApp=(MyApplication *)qApp;

    //标定参数
    //投影坐标x为row或height，y为col或width
    float f_scale_coff_width=AppConfig::f_calibrate_image_scale_width;
    float f_scale_coff_height=AppConfig::f_calibrate_image_scale_height;

    //获取投射变换参数HomMat2D
    //标定图像缩放0.5，恢复原始像素坐标
    pApp->f_hv_x0.Clear();
    pApp->f_hv_x0[0] = AppConfig::f_x0_1/f_scale_coff_height;
    pApp->f_hv_x0[1] = AppConfig::f_x0_2/f_scale_coff_height;
    pApp->f_hv_x0[2] = AppConfig::f_x0_3/f_scale_coff_height;
    pApp->f_hv_x0[3] = AppConfig::f_x0_4/f_scale_coff_height;

    pApp->f_hv_y0.Clear();
    pApp->f_hv_y0[0] = AppConfig::f_y0_1/f_scale_coff_width;
    pApp->f_hv_y0[1] = AppConfig::f_y0_2/f_scale_coff_width;
    pApp->f_hv_y0[2] = AppConfig::f_y0_3/f_scale_coff_width;
    pApp->f_hv_y0[3] = AppConfig::f_y0_4/f_scale_coff_width;

    //mm变为像素
    float f_each_pixel_equal_mm_col=AppConfig::f_each_pixel_equal_mm_width;
    float f_each_pixel_equal_mm_row=AppConfig::f_each_pixel_equal_mm_height;
    float f_row_offset=AppConfig::f_row_offset_mm/AppConfig::f_each_pixel_equal_mm_height;
    float f_col_offset=AppConfig::f_col_offset_mm/AppConfig::f_each_pixel_equal_mm_width;

    pApp->f_hv_x1.Clear();
    pApp->f_hv_x1[0] = AppConfig::f_x1_1/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[1] = AppConfig::f_x1_2/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[2] = AppConfig::f_x1_3/f_each_pixel_equal_mm_row+f_row_offset;
    pApp->f_hv_x1[3] = AppConfig::f_x1_4/f_each_pixel_equal_mm_row+f_row_offset;

    pApp->f_hv_y1.Clear();
    pApp->f_hv_y1[0] = AppConfig::f_y1_1/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[1] = AppConfig::f_y1_2/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[2] = AppConfig::f_y1_3/f_each_pixel_equal_mm_col+f_col_offset;
    pApp->f_hv_y1[3] = AppConfig::f_y1_4/f_each_pixel_equal_mm_col+f_col_offset;


    //投影坐标x为row或height，y为col或width
    float b_scale_coff_width=AppConfig::b_calibrate_image_scale_width;
    float b_scale_coff_height=AppConfig::b_calibrate_image_scale_height;

    //获取投射变换参数HomMat2D
    //标定图像缩放0.5，恢复原始像素坐标
    pApp->b_hv_x0.Clear();
    pApp->b_hv_x0[0] = AppConfig::b_x0_1/b_scale_coff_height;
    pApp->b_hv_x0[1] = AppConfig::b_x0_2/b_scale_coff_height;
    pApp->b_hv_x0[2] = AppConfig::b_x0_3/b_scale_coff_height;
    pApp->b_hv_x0[3] = AppConfig::b_x0_4/b_scale_coff_height;

    pApp->b_hv_y0.Clear();
    pApp->b_hv_y0[0] = AppConfig::b_y0_1/b_scale_coff_width;
    pApp->b_hv_y0[1] = AppConfig::b_y0_2/b_scale_coff_width;
    pApp->b_hv_y0[2] = AppConfig::b_y0_3/b_scale_coff_width;
    pApp->b_hv_y0[3] = AppConfig::b_y0_4/b_scale_coff_width;

    //mm变为像素
    float b_each_pixel_equal_mm_col=AppConfig::b_each_pixel_equal_mm_width;
    float b_each_pixel_equal_mm_row=AppConfig::b_each_pixel_equal_mm_height;
    float b_row_offset=AppConfig::b_row_offset_mm/AppConfig::b_each_pixel_equal_mm_height;
    float b_col_offset=AppConfig::b_col_offset_mm/AppConfig::b_each_pixel_equal_mm_width;

    pApp->b_hv_x1.Clear();
    pApp->b_hv_x1[0] = AppConfig::b_x1_1/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[1] = AppConfig::b_x1_2/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[2] = AppConfig::b_x1_3/b_each_pixel_equal_mm_row+b_row_offset;
    pApp->b_hv_x1[3] = AppConfig::b_x1_4/b_each_pixel_equal_mm_row+b_row_offset;

    pApp->b_hv_y1.Clear();
    pApp->b_hv_y1[0] = AppConfig::b_y1_1/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[1] = AppConfig::b_y1_2/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[2] = AppConfig::b_y1_3/b_each_pixel_equal_mm_col+b_col_offset;
    pApp->b_hv_y1[3] = AppConfig::b_y1_4/b_each_pixel_equal_mm_col+b_col_offset;

}

void frmMain::initForm()
{
    ui->widgetBtn->setProperty("flag", "btnNavTop");
    ui->widgetMain->setProperty("form", true);
    ui->widgetTop->setProperty("form", "title");
    ui->widgetTop->setFixedHeight(80);
    ui->bottomWidget->setFixedHeight(25);

    this->setProperty("form", true);
    //this->setProperty("canMove", true);
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->widgetTop->installEventFilter(this);
    ui->labLogoBg->installEventFilter(this);

    //设置右上角菜单,图形字体
    IconHelper::setIcon(ui->btnMenu_Min, QUIConfig::IconMin);
    IconHelper::setIcon(ui->btnMenu_Max, QUIConfig::IconNormal);
    IconHelper::setIcon(ui->btnMenu_Close, QUIConfig::IconClose, QUIConfig::FontSize + 2);

    //无边框可拉伸类
    frameless = new FramelessHelper(this);
    //激活当前窗体
    frameless->activateOn(this);
    //设置窗体的标题栏高度
    frameless->setTitleHeight(ui->widgetTop->height());
    //设置窗体可移动
    frameless->setWidgetMovable(true);
    //设置窗体可缩放
    frameless->setWidgetResizable(true);

    //设置底部状态栏
    ui->bottomWidget->setVersion("V20251117");
    ui->bottomWidget->setCurrentUserName("何纯玉");
    ui->bottomWidget->setCurrentUserType("管理员");
    ui->bottomWidget->setFontName(QUIConfig::FontName);
    int fontSize = (QUIConfig::FontSize > 12 ? QUIConfig::FontSize - 1 : QUIConfig::FontSize);
    ui->bottomWidget->setFontSize(fontSize);
    ui->bottomWidget->setLineFixedWidth(true);
    ui->bottomWidget->start();

}

void frmMain::initNav()
{
    //不同的样式边距和图标大小不一样,可以自行调整三个值看效果
    int topMargin, otherMargin;
    if (AppNav::NavMainLeft)
    {
        topMargin = ui->widgetMenu->height() + 6;
        otherMargin = 9;
    }
    else
    {
        topMargin = 3;
        otherMargin = 3;
    }

    ui->layoutNav->setContentsMargins(otherMargin, topMargin, otherMargin, otherMargin);

    pfrmView = new frmView(this);
    ui->stackedWidget->addWidget(pfrmView);

    pfrmData = new frmData(this);
    ui->stackedWidget->addWidget(pfrmData);

    pfrmConfig = new frmConfig(this);
    ui->stackedWidget->addWidget(pfrmConfig);

    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), pfrmData, SLOT(initIcon()));
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), pfrmView, SLOT(initIcon()));

    connect(AppEvent::Instance(), SIGNAL(fullScreen(bool)), this, SLOT(fullScreen(bool)));

    QList<QString> names, texts;
    names << "btnImgDetect" << "btnDataSearch" << "btnConfig" << "btnQuit";
    texts << "图像检测" << "数据处理" << "系统设置" << "退出系统";
    icons << 0xea00 << 0xe699 << 0xe706 << 0xe699;

    //根据设定实例化导航按钮对象
    for (int i = 0; i < texts.count(); i++)
    {
        QToolButton *btn = new QToolButton;
        if (AppNav::NavMainLeft)
        {
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setMinimumWidth(115);
        }
        else
        {
            btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            btn->setMinimumWidth(80);
        }

        btn->setObjectName(names.at(i));
        btn->setText(texts.at(i));
        btn->setCheckable(true);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(buttonClicked()));
        ui->layoutNav->addWidget(btn);
        btns << btn;
    }

    btns.at(0)->click();
}

void frmMain::initIcon()
{
    //如果logo图片不存在则隐藏标签,否则显示logo图片
    ui->labLogoBg->setAlignment(Qt::AlignCenter);
    QPixmap pixLogo(QUIHelper::appPath() + "/" + "bg.png");
    if (pixLogo.isNull())
    {
        //ui->labLogoBg->setVisible(false);
        //设置图形字体作为logo
        int size = 50;
        ui->labLogoBg->setFixedWidth(size + 20);
        IconHelper::setIcon(ui->labLogoBg, 0xe6f2, size);
    }
    else
    {
        int size = 150;
        ui->labLogoBg->setFixedWidth(size + 20);
        pixLogo = pixLogo.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->labLogoBg->setPixmap(pixLogo);
    }

    int count = btns.count();
    for (int i = 0; i < count; ++i)
    {
        AppNav::initNavBtnIcon(btns.at(i), icons.at(i), false);
    }
}

void frmMain::buttonClicked()
{
    QAbstractButton *btn = (QAbstractButton *)sender();
    if (btn == btns.at(0))
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else if (btn == btns.at(1))
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
    else if (btn == btns.at(2))
    {
        ui->stackedWidget->setCurrentIndex(2);
    }
    else if(btn == btns.at(3))
    {
        QMessageBox::StandardButton btn= QMessageBox::warning(NULL, "警告", "是否要关闭图像检测系统！", \
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(btn==QMessageBox::Yes)
        {
            Sleep(500);
            exit(0);
        }
    }

    //取消其他按钮选中
    foreach (QAbstractButton *b, btns) {
        b->setChecked(b == btn);
    }
}

void frmMain::initText()
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeData *pNodeData=pApp->pNodeData;

    //从配置文件读取logo名称
    ui->labLogoCn->setText(u8"板号识别系统");
    ui->labLogoEn->setText("OCR System for Plate");

    this->setWindowTitle(pNodeData->setting.local_window_name);

    //设置标题+版本+版权
    ui->bottomWidget->setTitle(ui->labLogoCn->text());
    ui->bottomWidget->setCopyright("东北大学 轧制技术及连轧自动化国家重点实验室");
}

void frmMain::initAction()
{
    //设置右键菜单模式
    this->setContextMenuPolicy(Qt::ActionsContextMenu);

    //添加换肤菜单
    QStringList styleNames, styleFiles;
    QUIStyle::getStyle(styleNames, styleFiles);

    //放入动作组产生互斥效果
    QActionGroup *actionGroup = new QActionGroup(this);
    for (int i = 0; i < styleNames.count(); i++) {
        QAction *action = new QAction(this);
        action->setCheckable(true);
        action->setText(styleNames.at(i));
        action->setData(styleFiles.at(i));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(doAction()));
        this->addAction(action);
        actionGroup->addAction(action);

        //默认样式
        if (styleFiles.at(i) == ":/qss/blackvideo.css") {
            action->setChecked(true);
        }
    }
}

void frmMain::doAction()
{
    QAction *action = (QAction *)sender();
    QString qssFile = action->data().toString();
    AppInit::Instance()->initStyle(qssFile);
}

void frmMain::fullScreen(bool full)
{
    if (full) {
        ui->widgetTop->setVisible(false);
        ui->bottomWidget->setVisible(false);
        this->showFullScreen();
        //this->showMaximized();
    } else {
        ui->widgetTop->setVisible(true);
        ui->bottomWidget->setVisible(true);
        this->showNormal();
    }
}

void frmMain::on_btnMenu_Min_clicked()
{
    this->showMinimized();
}

void frmMain::on_btnMenu_Max_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(QUIHelper::getScreenRect());
    }

    IconHelper::setIcon(ui->btnMenu_Max, max ? QUIConfig::IconNormal : QUIConfig::IconMax);
    this->setProperty("canMove", max);
    max = !max;
    frameless->setWidgetMovable(!max);
    frameless->setWidgetResizable(!max);
}

void frmMain::on_btnMenu_Close_clicked()
{
    this->close();
}

