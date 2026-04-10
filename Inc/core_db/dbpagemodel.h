#ifndef DBPAGEMODEL_H
#define DBPAGEMODEL_H

#include "dbhead.h"

//自定义模型设置列居中和右对齐+鼠标整行换色+指定列替换字符
class DbPageModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit DbPageModel(QObject *parent = 0);

protected:
    QVariant data(const QModelIndex &index, int role) const;

private:
    //所有居中
    bool allCenter;
    //居中对齐列
    QList<int> alignCenterColumn;
    //右对齐列
    QList<int> alignRightColumn;

public:
    //设置所有列居中
    void setAllCenter(bool allCenter);
    //设置居中对齐列索引集合
    void setAlignCenterColumn(const QList<int> &alignCenterColumn);
    //设置右对齐列索引集合
    void setAlignRightColumn(const QList<int> &alignRightColumn);
};

#endif // DBPAGEMODEL_H
