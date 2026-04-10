#include "dbpagemodel.h"

DbPageModel::DbPageModel(QObject *parent) : QSqlQueryModel(parent)
{
    allCenter = false;
    alignCenterColumn.clear();
    alignRightColumn.clear();
}

QVariant DbPageModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);
    if (allCenter) {
        if (role == Qt::TextAlignmentRole) {
            value = Qt::AlignCenter;
        }
    } else {
        //逐个从列索引中查找是否当前列在其中
        int column = index.column();
        bool existCenter = alignCenterColumn.contains(column);
        bool existRight = alignRightColumn.contains(column);

        if (role == Qt::TextAlignmentRole) {
            if (existCenter) {
                value = Qt::AlignCenter;
            }
            if (existRight) {
                value = (QVariant)(Qt::AlignVCenter | Qt::AlignRight);
            }
        }
    }

    //实现鼠标经过整行换色,如果设置了hoverRow才需要处理
    if (property("hoverRow").isValid()) {
        int row = property("hoverRow").toInt();
        if (row == index.row()) {
            if (role == Qt::BackgroundRole) {
                value = QColor(property("hoverBgColor").toString());
            } else if (role == Qt::ForegroundRole) {
                value = QColor(property("hoverTextColor").toString());
            }
        }
    }

    //实现隐藏部分显示,指定列替换字符
    if (property("hideColumn").isValid()) {
        int column = property("hideColumn").toInt();
        if (column == index.column()) {
            if (role == Qt::DisplayRole) {
                QString letter = property("hideLetter").toString();
                int start = property("hideStart").toInt();
                int end = property("hideEnd").toInt();
                QString str = value.toString();

                QStringList list;
                for (int i = 0; i < str.length(); i++) {
                    if (i >= start && i <= end) {
                        list << letter;
                    } else {
                        list << str.at(i);
                    }
                }

                value = list.join("");
            }
        }
    }

    return value;
}

void DbPageModel::setAllCenter(bool allCenter)
{
    this->allCenter = allCenter;
}

void DbPageModel::setAlignCenterColumn(const QList<int> &alignCenterColumn)
{
    this->alignCenterColumn = alignCenterColumn;
}

void DbPageModel::setAlignRightColumn(const QList<int> &alignRightColumn)
{
    this->alignRightColumn = alignRightColumn;
}
