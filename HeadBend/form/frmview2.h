#ifndef FRMVIEW2_H
#define FRMVIEW2_H

#include <qstandarditemmodel.h>
#include <qstringlistmodel.h>
#include <QWidget>

namespace Ui {
class frmView2;
}

class frmView2 : public QWidget
{
    Q_OBJECT

public:
    explicit frmView2(QWidget *parent = nullptr);
    ~frmView2();

private:
    Ui::frmView2 *ui;

public:
    QStandardItemModel *model_cali_para_front;
    QStandardItemModel *model_cal_para_front;

    QStandardItemModel *model_cali_para_back;
    QStandardItemModel *model_cal_para_back;

    QStringList msg_list;
    QStringListModel *msg_model;

    int x0_coord[32];
    int y0_coord[32];

    bool isGetPointOK;

    int widget_width,widget_height;
    int image_width,image_height;
    float width_scale_coff_front,height_scale_coff_front;
    float width_scale_coff_back,height_scale_coff_back;

public slots:
    void receivePoints(const QVector<QPoint> &points);
    void initForm();
    void loadCalibratePata();

    void readDataFromTableView();

public:
    void PrintEvent(QString str,bool isDispTime);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_pushButton_clear_draw_clicked();
    void on_pushButton_load_pic_front_clicked();
    void on_pushButton_load_pic_back_clicked();
    void on_pushButton_save_front_clicked();
    void on_pushButton_save_back_clicked();
    void on_pushButton_get_point_clicked();
};

#endif // FRMVIEW2_H
