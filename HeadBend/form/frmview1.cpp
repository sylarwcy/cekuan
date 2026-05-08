#include "frmview1.h"
#include "ui_frmview1.h"
#include "MyApplication.h"

frmView1::frmView1(QWidget *parent) : QWidget(parent),
                                      ui(new Ui::frmView1) {
    MyApplication *pApp = (MyApplication *) qApp;

    ui->setupUi(this);
    initForm();

    //from plc
    //颜色设置
    ui->tableView_from_plc->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background-color:rgb(245, 245, 245);}");

    //单选
    ui->tableView_from_plc->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_from_plc->setSelectionBehavior(QAbstractItemView::SelectRows);

    //tableView_para
    model_from_plc = new QStandardItemModel(this);
    model_from_plc->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    for (int i = 0; i < pApp->m_workstationList.size(); ++i)
        model_from_plc->setHorizontalHeaderItem(
            i + 1, new QStandardItem(pApp->m_workstationList[i]->m_location.right(5)));

    ui->tableView_from_plc->setModel(model_from_plc);
    ui->tableView_from_plc->setColumnWidth(0, 100);
    for (int i = 1; i <= pApp->m_workstationList.size(); ++i)
        ui->tableView_from_plc->setColumnWidth(i, 50);

    model_from_plc->setItem(0, 0, new QStandardItem("心跳"));

    model_from_plc->setItem(1, 0, new QStandardItem("推钢电机运转"));
    model_from_plc->setItem(2, 0, new QStandardItem("推钢转动方向"));

    model_from_plc->setItem(3, 0, new QStandardItem("加热炉冷检"));

    //单选
    // ui->tableView_to_plc->setSelectionMode(QAbstractItemView::SingleSelection);
    // ui->tableView_to_plc->setSelectionBehavior(QAbstractItemView::SelectRows);

    //tableView_para
    model_to_plc = new QStandardItemModel(this);
    model_to_plc->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr(u8"属性")));
    for (int i = 0; i < pApp->m_workstationList.size(); ++i)
        model_to_plc->setHorizontalHeaderItem(
            i + 1, new QStandardItem(pApp->m_workstationList[i]->m_location.right(5)));
    // ui->tableView_to_plc->setModel(model_to_plc);

    // ui->tableView_to_plc->setColumnWidth(0, 100);
    // for (int i = 1; i <= pApp->m_workstationList.size(); ++i)
    //     ui->tableView_to_plc->setColumnWidth(i, 50);

    // model_to_plc->setItem(0, 0, new QStandardItem("心跳"));
    // model_to_plc->setItem(1, 0, new QStandardItem("是否有钢板"));
    // model_to_plc->setItem(2, 0, new QStandardItem("头部位置"));
    // model_to_plc->setItem(3, 0, new QStandardItem("板号"));
    // model_to_plc->setItem(4, 0, new QStandardItem("图片名"));
    // model_to_plc->setItem(5, 0, new QStandardItem("触发信号"));
    // model_to_plc->setItem(6, 0, new QStandardItem("相机故障"));
    // model_to_plc->setItem(7, 0, new QStandardItem("备用1"));
    // model_to_plc->setItem(8, 0, new QStandardItem("备用2"));
    // model_to_plc->setItem(9, 0, new QStandardItem("备用3"));

    //变量初始化
    QTimer::singleShot(1000, this,SLOT(varInit()));
}

frmView1::~frmView1() {
    MyApplication *pApp = (MyApplication *) qApp;
    delete ui;

    //关闭相机
    on_pushButton_camera_close_f_clicked();
    on_pushButton_camera_close_b_clicked();
    Sleep(10);

    delete p_camera_front;
    delete p_camera_back;
}

void frmView1::resizeEvent(QResizeEvent *event) {
    static unsigned long tri_num;
    int width, height;

    tri_num++;

    //第一次触发会出错
    if (tri_num > 1) {
        width = ui->gView_front_ori->width();
        height = ui->gView_front_ori->height();
        SetWindowExtents(winHandle_cam1_ori, 0, 0, width, height);

        width = ui->gView_front_pro->width();
        height = ui->gView_front_pro->height();
        SetWindowExtents(winHandle_cam2_pro, 0, 0, width, height);

        width = ui->gView_back_ori->width();
        height = ui->gView_back_ori->height();
        SetWindowExtents(winHandle_cam1_pro, 0, 0, width, height);

        // width = ui->gView_back_pro->width();
        // height = ui->gView_back_pro->height();
        // SetWindowExtents(winHandle_cam2_pro, 0, 0, width, height);
    }
}

void frmView1::logSlot(const QString &message, int level) {
    ui->textBrowser->append(qUtf8Printable(message));
}

void frmView1::varInit() {
    MyApplication *pApp = (MyApplication *) qApp;
    QString front_ipstr = pApp->pNodeData->setting.front_ip_addr;
    QString back_ipstr = pApp->pNodeData->setting.back_ip_addr;

    //处理后数据由halcon显示
    Hlong winId_front_ori = (Hlong) ui->gView_front_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height(), winId_front_ori, "visible", "",
               &winHandle_cam1_ori);
    HDevWindowStack::Push(winHandle_cam1_ori);

    Hlong winId_front_pro = (Hlong) ui->gView_front_pro->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height(), winId_front_pro, "visible", "",
               &winHandle_cam1_pro);
    HDevWindowStack::Push(winHandle_cam1_pro);

    Hlong winId_back_ori = (Hlong) ui->gView_back_ori->winId();
    SetWindowAttr("background_color", "gray");
    OpenWindow(0, 0, ui->gView_front_ori->width(), ui->gView_front_ori->height(), winId_back_ori, "visible", "",
               &winHandle_cam2_ori);
    HDevWindowStack::Push(winHandle_cam2_ori);

    // Hlong winId_back_pro = (Hlong) ui->gView_back_pro->winId();
    // SetWindowAttr("background_color", "gray");
    // OpenWindow(0, 0, ui->gView_front_pro->width(), ui->gView_front_pro->height(), winId_back_pro, "visible", "",
    //            &winHandle_cam2_pro);
    // HDevWindowStack::Push(winHandle_cam2_pro);

    //相机与界面显示控件绑定
    // pApp->pNodeData->camera_1_para.hwnd = (HWND) ui->gView_front_ori->winId();
    pApp->pNodeData->camera_1_para.winHandle_ori = winHandle_cam1_ori;
    pApp->pNodeData->camera_2_para.winHandle_ori = winHandle_cam2_ori;
    // pApp->pNodeData->camera_1_para.winHandle_pro = winHandle_cam1_pro;
    // pApp->m_workstationList[0]->m_pWorkerCamera->SetHandle(winHandle_cam1_ori, winHandle_cam2_ori);

    connect(pApp->m_workstationList[0], &Workstation::sigForwardToView,
                this, &frmView1::onUpdateRawImage);

    QLOG_INFO() << u8"相机创建完成!";

    //防止控件失去焦点时闪烁
    ui->gView_front_ori->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_front_pro->setAttribute(Qt::WA_OpaquePaintEvent);

    ui->gView_back_ori->setAttribute(Qt::WA_OpaquePaintEvent);
    ui->gView_back_ori->setAttribute(Qt::WA_OpaquePaintEvent);

    //全局变量赋值，便于其它界面访问
    // pApp->p_camera_front = p_camera_front;
    // pApp->p_camera_back = p_camera_back;

    // pApp->winHandle_front_ori = winHandle_cam1_ori;
    // pApp->winHandle_front_pro = winHandle_cam1_pro;
    //
    // pApp->winHandle_back_ori = winHandle_cam2_ori;
    // pApp->winHandle_back_pro = winHandle_cam2_pro;


    //是否测试模式
    ui->checkBox_test_mode->setEnabled(false);
    ui->checkBox_test_mode->setChecked(pApp->pNodeData->setting.test_mode);

    //启动界面刷新
    startDispRefresh();
}

void frmView1::initForm() {
    MyApplication *pApp = (MyApplication *) qApp;

    // if (pApp->m_workstationList.size() > 0) {

    //     Workstation* station = pApp->m_workstationList[0];

    //     // =========================================================================
    //     // 1. 绑定：原始相机实时流 -> 显示到左右两个屏幕
    //     // =========================================================================
    //     if (station->m_pWorkerCamera) {
    //         // 注意：这里连接的是 WorkerCamera.h 里面定义的 sigDisplayRawImage 信号
    //         connect(station->m_pWorkerCamera, &WorkerCamera::sigDisplayRawImage,
    //                 this, [=](const DualCameraChunk &chunk) {

    //                     // ⚠️ 这里的 chunk.imgLeft 和 chunk.imgRight 是我猜的名字。
    //                     // 如果报错，请去 workStationDataStructure.h 里面看一下 DualCameraChunk 结构体里
    //                     // 存放左右图像的变量叫什么名字，替换一下即可！(可能是 leftMat, rightMat 或者是左相机QImage)

    //                     // 显示左相机
    //                     ui->gView_front_ori->ReceiveImage(chunk.imgLeft);

    //                     // 显示右相机
    //                     ui->gView_back_ori->ReceiveImage(chunk.imgRight);
    //                 });
    //     }

    //     // =========================================================================
    //     // 2. 绑定：算法处理后的拼接图 -> 显示到中间大屏
    //     // =========================================================================
    //     // 假设 Workstation 中有发送最终结果的信号 sigSendDataToUI，里面包含 WidthResult
    //     connect(station, &Workstation::sigSendDataToUI,
    //             this, [=](const WidthResult& result) {

    //                 // ⚠️ 这里的 result.stitchedImage 也是我猜的。
    //                 // 请去 workStationDataStructure.h 里面看一下 WidthResult 结构体里那张拼接图叫啥
    //                 ui->gView_front_pro->ReceiveImage(result.stitchedImage);
    //             });
    // }
    //不换行
    ui->textBrowser->setWordWrapMode(QTextOption::NoWrap);
    //设置最大行数
    ui->textBrowser->document()->setMaximumBlockCount(100);
}

void frmView1::onUpdateRawImage(const DualCameraChunk &chunk) {
    try {
        // 1. 渲染左图 (Master)
        if (chunk.imgLeft.IsInitialized()) {
            HTuple widthL, heightL;
            // 获取当前左图的真实宽高
            HalconCpp::GetImageSize(chunk.imgLeft, &widthL, &heightL);

            // 【核心修复】：重置左窗视口，确保图像完整显示且自适应控件大小
            HalconCpp::SetPart(winHandle_cam1_ori, 0, 0, heightL - 1, widthL - 1);

            // 执行显示
            HalconCpp::DispObj(chunk.imgLeft, winHandle_cam1_ori);
        }

        // 2. 渲染右图 (Slave)
        if (chunk.imgRight.IsInitialized()) {
            HTuple widthR, heightS;
            // 获取当前右图的真实宽高
            HalconCpp::GetImageSize(chunk.imgRight, &widthR, &heightS);

            // 【核心修复】：重置右窗视口
            HalconCpp::SetPart(winHandle_cam2_ori, 0, 0, heightS - 1, widthR - 1);

            // 执行显示
            HalconCpp::DispObj(chunk.imgRight, winHandle_cam2_ori);
        }
    } catch (const HalconCpp::HException& e) {
        // 增加更详细的错误日志
        QLOG_ERROR() << "UI界面刷新原始图像失败, 原因:" << e.ErrorMessage().Text();
    }
}

//用于数据刷新
void frmView1::refreshDataDisp(void) {
    MyApplication *pApp = (MyApplication *) qApp;
    QString mystr;
    static long disp_num;

    //监控线程循环时间
    for (int i = 0; i < pApp->m_workstationList[0]->m_pWorkerCamera->m_camList.size(); ++i)
        ui->lineEdit_camera_num_front->setText(
            QString("%1").arg(pApp->m_workstationList[0]->m_pWorkerCamera->m_thread_cycle_ms));

    ui->lineEdit_disp_num->setText(QString("%1").arg(disp_num++));

    for (int i = 1; i <= pApp->m_workstationList.size(); ++i) {
        model_from_plc->setItem(
            0, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetFromPLCData(i).beat_num)));
        model_from_plc->setItem(
            1, i, new QStandardItem(
                QString("%1").arg(pApp->m_mem_manager->GetFromPLCData(i).motor_signal)));
        model_from_plc->setItem(
            2, i, new QStandardItem(
                QString("%1").arg(pApp->m_mem_manager->GetFromPLCData(i).motor_direction)));
        model_from_plc->setItem(
            3, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetFromPLCData(i).cold_inspection)));

        //设定数据刷新
        model_to_plc->setItem(
            0, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).beat_num)));
        model_to_plc->setItem(
            1, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).has_steel_plate)));
        model_to_plc->setItem(
            2, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).gundao_X_left)));
        model_to_plc->setItem(
            3, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).ID)));
        model_to_plc->setItem(
            4, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).img_name)));
        model_to_plc->setItem(
            5, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).recog_finished_flag)));
        model_to_plc->setItem(
            6, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).has_cam_broken)));
        model_to_plc->setItem(
            7, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).bak1)));
        model_to_plc->setItem(
            8, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).bak2)));
        model_to_plc->setItem(
            9, i, new QStandardItem(QString("%1").arg(pApp->m_mem_manager->GetToPLCData(i).bak3)));
    }
    //界面表格数据刷新

    pApp->pNodeData->isDispProcessing = false;
}

//启动触发循环
void frmView1::startDispRefresh() {
    MyApplication *pApp = (MyApplication *) qApp;
    for (int i = 0; i < pApp->m_workstationList.size(); ++i)
        pApp->m_workstationList[i]->StartTrigger();

    QLOG_INFO() << "触发进程启动，开始刷新界面...";
}

void frmView1::on_pushButton_camera_open_f_clicked() {
    MyApplication *pApp = (MyApplication *) qApp;
    int ret_int;
    bool ret_bool;

    QLOG_INFO() << "机前相机启动中...";
    ret_int = p_camera_front->FindCameraGigE();
    ret_bool = p_camera_front->OpenCamera();
    ret_bool = p_camera_front->StartGrabbing();
}

void frmView1::on_pushButton_camera_close_f_clicked() {
    int ret_int;
    bool ret_bool;

    QLOG_INFO() << u8"机前相机关闭中...";
    p_camera_front->StopGrabbing();
    ret_int = p_camera_front->CloseCamera();
}

void frmView1::on_pushButton_camera_get_para_f_clicked() {
    bool isWidthMirror, isHeightMirror;

    QLOG_INFO() << u8"机前相机参数获取";
    p_camera_front->GetParameter();
    p_camera_front->GetMirror(isWidthMirror, isHeightMirror);

    QLOG_INFO() << "宽度方向镜像:" << isWidthMirror << ",高度方向镜像:" << isHeightMirror;
}

void frmView1::on_pushButton_camera_set_para_f_clicked() {
    MyApplication *pApp = (MyApplication *) qApp;

    QLOG_INFO() << u8"机前相机参数设定";
    p_camera_front->SetParameter(pApp->pNodeData->setting.front_expTime,
                                 pApp->pNodeData->setting.front_gain,
                                 pApp->pNodeData->setting.front_frameRate);
}

void frmView1::on_pushButton_camera_open_b_clicked() {
    MyApplication *pApp = (MyApplication *) qApp;
    int ret_int;
    bool ret_bool;

    QLOG_INFO() << "机后相机启动中...";
    ret_int = p_camera_back->FindCameraGigE();
    ret_bool = p_camera_back->OpenCamera();
    ret_bool = p_camera_back->StartGrabbing();
}


void frmView1::on_pushButton_camera_close_b_clicked() {
    int ret_int;
    bool ret_bool;

    QLOG_INFO() << u8"机后相机关闭中...";
    p_camera_back->StopGrabbing();
    ret_int = p_camera_back->CloseCamera();
}

void frmView1::on_pushButton_camera_get_para_b_clicked() {
    bool isWidthMirror, isHeightMirror;

    QLOG_INFO() << u8"机后相机参数获取";
    p_camera_back->GetParameter();

    p_camera_back->GetMirror(isWidthMirror, isHeightMirror);
    QLOG_INFO() << "宽度方向镜像:" << isWidthMirror << ",高度方向镜像:" << isHeightMirror;
}

void frmView1::on_pushButton_camera_set_para_b_clicked() {
    MyApplication *pApp = (MyApplication *) qApp;

    QLOG_INFO() << u8"机后相机参数设定";
    p_camera_front->SetParameter(pApp->pNodeData->setting.back_expTime,
                                 pApp->pNodeData->setting.back_gain,
                                 pApp->pNodeData->setting.back_frameRate);
}

void frmView1::on_checkBox_test_img_clicked(bool checked) {
    MyApplication *pApp = (MyApplication *) qApp;
    pApp->isTestImage = checked;
}


void frmView1::on_checkBox_test_video_clicked(bool checked) {
    MyApplication *pApp = (MyApplication *) qApp;
    HTuple hv_Value_0, hv_Value_1;

    QString strName_front = QCoreApplication::applicationDirPath() + "/Image_test/video_test_front.avi";
    QString strName_back = QCoreApplication::applicationDirPath() + "/Image_test/video_test_back.avi";

    //打开
    if (!pApp->isTestVideo && checked) {
        //机前测试
        OpenFramegrabber("DirectFile", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
                         strName_front.toStdString().c_str(), "default", -1, -1, &pApp->hv_AcqHandle_front);
        SetFramegrabberParam(pApp->hv_AcqHandle_front, "grab_timeout", 500);

        GetFramegrabberParam(pApp->hv_AcqHandle_front, "first_frame", &hv_Value_0);
        GetFramegrabberParam(pApp->hv_AcqHandle_front, "last_frame", &hv_Value_1);

        pApp->frame_start_front = hv_Value_0[0];
        pApp->frame_end_front = hv_Value_1[0];

        GrabImageStart(pApp->hv_AcqHandle_front, -1);

        //机后测试
        OpenFramegrabber("DirectFile", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
                         strName_back.toStdString().c_str(), "default", -1, -1, &pApp->hv_AcqHandle_back);
        SetFramegrabberParam(pApp->hv_AcqHandle_back, "grab_timeout", 500);

        GetFramegrabberParam(pApp->hv_AcqHandle_back, "first_frame", &hv_Value_0);
        GetFramegrabberParam(pApp->hv_AcqHandle_back, "last_frame", &hv_Value_1);

        pApp->frame_start_back = hv_Value_0[0];
        pApp->frame_end_back = hv_Value_1[0];

        GrabImageStart(pApp->hv_AcqHandle_back, -1);
    }

    //关闭
    if (pApp->isTestVideo && !checked) {
        CloseFramegrabber(pApp->hv_AcqHandle_front);
        CloseFramegrabber(pApp->hv_AcqHandle_back);
    }

    pApp->isTestVideo = checked;
}

void frmView1::on_pushButton_load_para_clicked() {
    MyApplication *pApp = (MyApplication *) qApp;
    NodeData *pNodeData = pApp->pNodeData;
    int i;

    //读ini文件
    int rtn;
    QLOG_INFO() << u8"开始加载环境变量...";
    rtn = pNodeData->ReadSetting();
    if (rtn == 0)
        QLOG_INFO() << u8"加载环境变量成功！";
    else {
        QLOG_ERROR() << u8"加载环境变量失败！";
        return;
    }

    //显示环境变量
    QLOG_INFO() << QString("环境变量 local_window_name=%1").arg(pNodeData->setting.local_window_name);
    QLOG_INFO() << QString("环境变量 comm_window_name=%1").arg(pNodeData->setting.comm_window_name);

    QLOG_INFO() << QString("环境变量 l2_plc_share_mem_name=%1").arg(pNodeData->setting.to_plc_share_mem_name);
    QLOG_INFO() << QString("环境变量 plc_l2_share_mem_name=%1").arg(pNodeData->setting.from_plc_share_mem_name);

    QLOG_INFO() << QString("环境变量 data_refresh_ms=%1").arg(pNodeData->setting.data_refresh_ms);
    QLOG_INFO() << QString("环境变量 sample_period=%1").arg(pNodeData->setting.sample_period);

    // QLOG_INFO() << QString("环境变量 test_mode=%1").arg(pNodeData->setting.test_mode);
    // QLOG_INFO() << QString("环境变量 write_rot_video=%1").arg(pNodeData->setting.write_rot_video);
    //
    // QLOG_INFO() << QString("环境变量 DbType1=%1").arg(pNodeData->setting.DbType1);
    // QLOG_INFO() << QString("环境变量 DbName1=%1").arg(pNodeData->setting.DbName1);
    // QLOG_INFO() << QString("环境变量 HostName1=%1").arg(pNodeData->setting.HostName1);
    // QLOG_INFO() << QString("环境变量 HostPort1=%1").arg(pNodeData->setting.HostPort1);
    // QLOG_INFO() << QString("环境变量 UserName1=%1").arg(pNodeData->setting.UserName1);
    // QLOG_INFO() << QString("环境变量 UserPwd1=%1").arg(pNodeData->setting.UserPwd1);
    //
    // QLOG_INFO() << QString("环境变量 front_ip_addr=%1").arg(pNodeData->setting.front_ip_addr);
    // QLOG_INFO() << QString("环境变量 front_expTime=%1").arg(pNodeData->setting.front_expTime);
    // QLOG_INFO() << QString("环境变量 front_gain=%1").arg(pNodeData->setting.front_gain);
    // QLOG_INFO() << QString("环境变量 front_frameRate=%1").arg(pNodeData->setting.front_frameRate);
    // QLOG_INFO() << QString("环境变量 front_camera_res_width=%1").arg(pNodeData->setting.front_camera_res_width);
    // QLOG_INFO() << QString("环境变量 front_camera_res_height=%1").arg(pNodeData->setting.front_camera_res_height);
    //
    // QLOG_INFO() << QString("环境变量 back_ip_addr=%1").arg(pNodeData->setting.back_ip_addr);
    // QLOG_INFO() << QString("环境变量 back_expTime=%1").arg(pNodeData->setting.back_expTime);
    // QLOG_INFO() << QString("环境变量 back_gain=%1").arg(pNodeData->setting.back_gain);
    // QLOG_INFO() << QString("环境变量 back_frameRate=%1").arg(pNodeData->setting.back_frameRate);
    // QLOG_INFO() << QString("环境变量 back_camera_res_width=%1").arg(pNodeData->setting.back_camera_res_width);
    // QLOG_INFO() << QString("环境变量 back_camera_res_height=%1").arg(pNodeData->setting.back_camera_res_height);
    //
    // QLOG_INFO() << QString("环境变量 calibrate_path=%1").arg(pNodeData->setting.calibrate_path);
    // QLOG_INFO() << QString("环境变量 test_path=%1").arg(pNodeData->setting.test_path);
    // QLOG_INFO() << QString("环境变量 save_path=%1").arg(pNodeData->setting.save_path);
}

void frmView1::onMeasureReady(const WidthResult &res)
{
    qDebug() << "[Pipeline-4] UI 界面成功拆包 -> 准备刷新数值和画面! 图像状态:" << res.dispImage.IsInitialized();
    if (res.isValid) {
        // 保留两位小数
        ui->lineEdit_width->setText(QString::number(res.widthValue, 'f', 2));
    } else {
        ui->lineEdit_width->setText("Error");
    }

// 2. 渲染融合图像到窗口
    if (res.dispImage.IsInitialized()) {
        try {
            // 使用界面绑定的 Halcon 窗口句柄
            HTuple currentWin = this->winHandle_cam1_pro;

            // =========================================================
            // 【核心修复】：动态自适应图像尺寸，解决画面不完整的问题
            // =========================================================
            HTuple imgW, imgH;
            HalconCpp::GetImageSize(res.dispImage, &imgW, &imgH);

            // 强制将 Halcon 窗口的显示视口设定为图像的完整宽高
            // 这句代码会让图像自动缩放，完美塞进 UI 控件中，绝不被裁切！
            HalconCpp::SetPart(currentWin, 0, 0, imgH - 1, imgW - 1);

            // A. 显示拼接好的大图
            HalconCpp::DispObj(res.dispImage, currentWin);
            set_display_font(currentWin, 30, "mono", "true", "false");

            // 如果测量有效，开始叠加渲染文字和线条
            if (res.isValid && res.renderLeftX != -1) {

                // B. 绘制边缘线条 (绿色/青色)
                HalconCpp::SetLineWidth(currentWin, 2);

                // 【顺手优化】：把之前写死的 2048 长度改成了动态自适应的高度 (imgH)
                HalconCpp::SetColor(currentWin, "green");
                HalconCpp::DispLine(currentWin, 0, res.renderLeftX, imgH - 1, res.renderLeftX);

                HalconCpp::SetColor(currentWin, "cyan");
                HalconCpp::DispLine(currentWin, 0, res.renderRightX, imgH - 1, res.renderRightX);

                // C. 绘制测量十字星 (黄色)
                HalconCpp::SetColor(currentWin, "yellow");
                HalconCpp::DispCross(currentWin, res.renderY, res.renderLeftX, 60, 0);
                HalconCpp::DispCross(currentWin, res.renderY, res.renderRightX, 60, 0);

                // D. 渲染结果文字
                QString widthStr = QString("Physical Width: %1 mm").arg(res.widthValue, 0, 'f', 3);
                HalconCpp::DispText(currentWin, widthStr.toLocal8Bit().constData(),
                                    "window", 20, 20, "red", HTuple(), HTuple());

                QString yawStr = QString("Yaw Angle: %1 deg").arg(res.yawAngle, 0, 'f', 2);
                HalconCpp::DispText(currentWin, yawStr.toLocal8Bit().constData(),
                                    "window", 50, 20, "yellow", HTuple(), HTuple());

            } else {
                // 如果无效，在图上正中间写个大大的红色警告
                HalconCpp::DispText(currentWin, "NO PLATE DETECTED",
                                    "window", "center", "center", "red", HTuple(), HTuple());
            }

        } catch (HalconCpp::HException &e) {
            qWarning() << "[UI渲染报错] " << e.ErrorMessage().Text();
        }
    }
}
// #include "qcustomplot.h"

// // 假设这段代码写在 frmshowcurve.cpp 或 frmview1.cpp 的初始化函数中
// void frmView1::initCurve()
// {
//     // 1. 添加一条图线 (Graph)
//     ui->widget_curve->addGraph();

//     // 2. 设置曲线的颜色和粗细
//     QPen bluePen;
//     bluePen.setColor(QColor(0, 114, 219)); // 科技蓝
//     bluePen.setWidth(2);
//     ui->widget_curve->graph(0)->setPen(bluePen);

//     // 3. 设置 X 轴和 Y 轴的文字标签
//     ui->widget_curve->xAxis->setLabel("测量点位/时间");
//     ui->widget_curve->yAxis->setLabel("测量宽度 (mm)");

//     // 4. 设置初始的坐标轴显示范围
//     // 假设你的目标宽度是 1250mm，上下公差 5mm
//     ui->widget_curve->yAxis->setRange(1240, 1260);

//     // 5. 让 X 轴的刻度以时间格式显示 (可选，如果你想用真实时间作为X轴)
//     QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//     timeTicker->setTimeFormat("%h:%m:%s");
//     ui->widget_curve->xAxis->setTicker(timeTicker);
// }