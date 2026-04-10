#include "frmshowcurve.h"
#include "ui_frmshowcurve.h"
#include "quihelper.h"
#include "dbconnthread.h"
#include "appconfig.h"

#include "MyApplication.h"

frmShowCurve::frmShowCurve(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmShowCurve)
{
    ui->setupUi(this);

    setWindowTitle("轧件头尾曲线共101个点");
}

frmShowCurve::~frmShowCurve()
{
    delete ui;
}

void frmShowCurve::DrawCurve(QString strID)
{
    MyApplication *pApp=(MyApplication *)qApp;

    QString strSql;
    QString strX1,strY1,strX2,strY2;
    int num1,num2;
    QStringList strListX,strListY;
    int i;

    ui->lineEdit_ID->setText(strID);

    //头尾曲线
    //删除所有图
    while(ui->myCustomPlot->graphCount()>0)
    {
        ui->myCustomPlot->removeGraph(0);
    }
    ui->myCustomPlot->replot();

    QSqlQuery query(pApp->dbConn_local->getDatabase());

    strSql=QString("select count(*) as num from ppc_image where ID='%1'").arg(strID);
    bool isOK=query.exec(strSql);
    if(!isOK)
        return;

    query.next();
    int recNum=query.value(0).toInt();
    if(recNum==0)
        return;

    strSql=QString("select head_coord_x,head_coord_y,tail_coord_x,tail_coord_y from ppc_image where ID='%1'").arg(strID);
    isOK=query.exec(strSql);
    if(!isOK)
        return;

    if(query.next())
    {
        strX1=query.value(0).toString().trimmed();
        strY1=query.value(1).toString().trimmed();

        strX2=query.value(2).toString().trimmed();
        strY2=query.value(3).toString().trimmed();
    }
    else
        return;

    ui->myCustomPlot->legend->setVisible(true);
    ui->myCustomPlot->legend->setFont(QFont("Helvetica", 9));
    ui->myCustomPlot->legend->setRowSpacing(-3);
    ui->myCustomPlot->xAxis->setLabelFont(QFont("Helvetica", 9));
    ui->myCustomPlot->yAxis->setLabelFont(QFont("Helvetica", 9));
    ui->myCustomPlot->xAxis->setLabel("钢板宽度");
    ui->myCustomPlot->yAxis->setLabel("头尾形状");

    QVector<QCPScatterStyle::ScatterShape> shapes;
    shapes << QCPScatterStyle::ssDisc;
    shapes << QCPScatterStyle::ssTriangle;

    double head_y_max,tail_y_max;
    //第一条曲线
    num1=100;
    if(num1>1)
    {
        QVector<double> x1(num1+1),y1(num1+1);
        strListX= strX1.split(",");
        strListY= strY1.split(",");

        qDebug()<<strListX.count()<<strListY.count();
        if((strListX.count()!=num1+1) || (strListY.count()!=num1+1))
            return;

        head_y_max=0.0;
        for(int i=0;i<num1+1;i++)
        {
            x1[i]=strListX[i].toDouble()*AppConfig::each_pixel_equal_mm;
            y1[i]=strListY[i].toDouble()*AppConfig::each_pixel_equal_mm;
            if(y1[i]>head_y_max)
                head_y_max=y1[i];
        }

        ui->myCustomPlot->addGraph();
        ui->myCustomPlot->graph()->setPen(QPen(Qt::black));
        ui->myCustomPlot->graph()->setLineStyle(QCPGraph::lsLine);
        ui->myCustomPlot->graph()->setName("头部形状");
        ui->myCustomPlot->graph()->setData(x1, y1);
        ui->myCustomPlot->graph()->setScatterStyle(QCPScatterStyle(shapes.at(0), 5));

    }

    //第二条曲线
    num2=100;
    if(num2>1)
    {
        QVector<double> x2(num2+1),y2(num2+1);
        strListX= strX2.split(",");
        strListY= strY2.split(",");

        qDebug()<<strListX.count()<<strListY.count();
        if((strListX.count()!=num2+1) || (strListY.count()!=num2+1))
            return;

        tail_y_max=0;
        for(int i=0;i<num2+1;i++)
        {
            x2[i]=strListX[i].toDouble()*AppConfig::each_pixel_equal_mm;
            y2[i]=strListY[i].toDouble()*AppConfig::each_pixel_equal_mm;

            if(y2[i]>tail_y_max)
                tail_y_max=y2[i];
        }

        ui->myCustomPlot->addGraph();
        ui->myCustomPlot->graph()->setPen(QPen(Qt::red));
        ui->myCustomPlot->graph()->setLineStyle(QCPGraph::lsLine);
        ui->myCustomPlot->graph()->setName("尾部形状");

        ui->myCustomPlot->graph()->setData(x2, y2);
        ui->myCustomPlot->graph()->setScatterStyle(QCPScatterStyle(shapes.at(1), 5));

    }

    if(head_y_max>tail_y_max)
        ui->myCustomPlot->graph(0)->rescaleAxes(true);
    else
        ui->myCustomPlot->graph(1)->rescaleAxes(true);


    //最后处理
    //ui->myCustomPlot->xAxis->setRange(0.0,3000.0);
    //ui->myCustomPlot->yAxis->setRange(-0.5,0.5);
    ui->myCustomPlot->axisRect()->setupFullAxesBox(true);
    ui->myCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->myCustomPlot->replot();
}

void frmShowCurve::on_pushButton_close_clicked()
{
    close();
}

