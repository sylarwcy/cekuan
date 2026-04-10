#ifndef WORKER_SAMPLE_H
#define WORKER_SAMPLE_H

#include <QObject>
#include <QThread>

class WorkerSample : public QObject
{
    Q_OBJECT
public:
    explicit WorkerSample(QObject *parent = nullptr);
    ~WorkerSample();

signals:
    void sig_finish_front_image();
    void sig_finish_back_image();

    void sig_save_front_data(QString id,QString fName);
    void sig_save_back_data(QString id,QString fName);

public:
    bool DirExist(QString fullPath);
    bool DirExistEx(QString fullPath);
    QString SaveImgToDisk(bool isHead,QString ID,QString strFlag);

public slots:
    void processFrontImage(int row1,int row2);
    void processBackImage(int row1,int row2);

};

#endif
