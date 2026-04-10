#ifndef FRMVIEW1_H
#define FRMVIEW1_H

#include <QWidget>
#include "qcustomplot.h"
#include "MyApplication.h"

namespace Ui {
class frmView1;
}

class frmView1 : public QWidget
{
    Q_OBJECT

public:
    explicit frmView1(QWidget *parent = nullptr);
    ~frmView1();

private:
    Ui::frmView1 *ui;

public:
    HTuple winHandle_cam1_ori;
    HTuple winHandle_cam1_pro;

    HTuple winHandle_cam2_ori;
    HTuple winHandle_cam2_pro;

    MyCameraGigE *p_camera_front;
    MyCameraGigE *p_camera_back;

    QStandardItemModel *model_from_plc;
    QStandardItemModel *model_to_plc;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    void varInit();
    void logSlot(const QString &message, int level);
    void refreshDataDisp(void);

    void startDispRefresh();

public:
    void initForm();

private slots:
    void on_pushButton_camera_open_f_clicked();
    void on_pushButton_camera_close_f_clicked();
    void on_pushButton_camera_get_para_f_clicked();
    void on_pushButton_camera_set_para_f_clicked();

    void on_pushButton_camera_open_b_clicked();
    void on_pushButton_camera_close_b_clicked();
    void on_pushButton_camera_get_para_b_clicked();
    void on_pushButton_camera_set_para_b_clicked();

    void on_checkBox_test_img_clicked(bool checked);
    void on_checkBox_test_video_clicked(bool checked);
    void on_pushButton_load_para_clicked();
};

#endif // FRMVIEW1_H
