#ifndef DBPAGE_H
#define DBPAGE_H

/**
 * 数据库通用翻页类 作者:feiyangqingyun(QQ:517216493) 2017-1-15
 * 1. 可设置每页多少行记录,自动按照设定的值进行分页
 * 2. 可设置要查询的表名/字段集合/条件语句/排序语句/
 * 3. 可设置第一页/上一页/下一页/末一页翻页按钮
 * 4. 可设置当前页/总页数/总记录数/每页记录数/查询用时标签页
 * 5. 多线程查询总记录数,数据量巨大时候不会卡主界面
 * 6. 建议条件字段用整型类型的主键,速度极快
 * 7. 提供查询结果返回信号,包括当前页/总页数/总记录数/查询用时等信息
 * 8. 可设置所有列或者某一列对齐样式例如居中或者右对齐
 * 9. 可增加列用于标识该条记录,设定列的位置/标题/宽度
 * 10. 提供函数直接执行第一页/上一页/下一页/末一页
 * 11. 提供函数直接跳转到指定页
 * 12. 根据是否第一页/末一页自动禁用对应的按钮
 * 13. 本控件是翻页功能类,和翻页控件navpage完美搭配形成超级牛逼的翻页控件
 */

#include "dbhelper.h"
#include "dbpagemodel.h"

class DbPage : public QObject
{
    Q_OBJECT
public:
    explicit DbPage(QObject *parent = 0);    

private:
    int startIndex;             //分页开始索引,每次翻页都变动
    DbPageModel *queryModel;    //查询模型

    QLabel *labPageTotal;       //总页数标签
    QLabel *labPageCurrent;     //当前页标签
    QLabel *labRecordsTotal;    //总记录数标签
    QLabel *labRecordsPerpage;  //每页记录数标签
    QLabel *labSelectTime;      //显示查询用时标签
    QLabel *labSelectInfo;      //总页数当前页总记录数每页记录数

    QTableView *tableView;      //显示数据的表格对象
    QAbstractButton *btnFirst;  //第一页按钮对象
    QAbstractButton *btnPrevious;//上一页按钮对象
    QAbstractButton *btnNext;   //下一页按钮对象
    QAbstractButton *btnLast;   //末一页按钮对象

    QString connName;           //所使用的数据库连接名
    DbType dbType;              //数据库类型

    quint32 pageCurrent;        //当前第几页
    quint32 pageTotal;          //总页数
    quint32 recordsTotal;       //总记录数
    quint32 recordsPerpage;     //每页显示记录数

    QString tableName;          //表名
    QString selectColumn;       //要查询的字段集合
    QString orderSql;           //排序语句
    QString whereSql;           //条件语句
    QList<QString> columnNames; //列名集合
    QList<int> columnWidths;    //列宽集合

    int insertColumnIndex;      //插入的列的索引位置
    QString insertColumnName;   //插入的列的标题
    int insertColumnWidth;      //插入的列的宽度

public:
    quint32 getPageCurrent()    const;
    quint32 getPageTotal()      const;
    quint32 getRecordsTotal()   const;
    quint32 getRecordsPerpage() const;

private slots:
    //绑定sql语句到表格
    void bindData(const QString &sql);
    //生成分页sql语句
    QString getPageSql();

    //查询到记录行数完成
    void receiveCount(const QString &tag, int count, int msec);

signals:
    //将翻页后的页码信息发出去可能其他地方要用到
    void receivePage(quint32 pageCurrent, quint32 pageTotal, quint32 recordsTotal, quint32 recordsPerpage);

public slots:
    //设置需要显示数据的表格,数据翻页对应的按钮
    void setControl(QTableView *tableView,
                    QLabel *labPageTotal, QLabel *labPageCurrent,
                    QLabel *labRecordsTotal, QLabel *labRecordsPerpage,
                    QLabel *labSelectTime, QLabel *labSelectInfo,
                    QAbstractButton *btnFirst, QAbstractButton *btnPrevious,
                    QAbstractButton *btnNext, QAbstractButton *btnLast);
    void setControl(QTableView *tableView,
                    QLabel *labPageTotal, QLabel *labPageCurrent,
                    QLabel *labRecordsTotal, QLabel *labRecordsPerpage,
                    QLabel *labSelectTime, QLabel *labSelectInfo);
    void setControl(QTableView *tableView,
                    QAbstractButton *btnFirst, QAbstractButton *btnPrevious,
                    QAbstractButton *btnNext, QAbstractButton *btnLast);
    void setControl(QTableView *tableView);


    //设置数据库连接名称+数据库类型
    void setConnName(const QString &connName);
    void setDbType(const QString &dbType);
    void setDbType(const DbType &dbType);

    //设置要查询的表名+字段列名集合
    void setTableName(const QString &tableName);
    void setSelectColumn(const QString &selectColumn);

    //设置排序sql+条件sql
    void setOrderSql(const QString &orderSql);
    void setWhereSql(const QString &whereSql);

    //设置每页显示多少行数据
    void setRecordsPerpage(int recordsPerpage);
    //设置列名称集合
    void setColumnNames(const QList<QString> &columnNames);
    //设置列宽度集合
    void setColumnWidths(const QList<int> &columnWidths);

    //设置所有列居中
    void setAllCenter(bool allCenter);
    //设置居中对齐列索引集合
    void setAlignCenterColumn(const QList<int> &alignCenterColumn);
    //设置右对齐列索引集合
    void setAlignRightColumn(const QList<int> &alignRightColumn);

    //设置插入的列的索引
    void setInsertColumnIndex(int insertColumnIndex);
    //设置插入的列的标题
    void setInsertColumnName(const QString &insertColumnName);
    //设置插入的列的宽度
    void setInsertColumnWidth(int insertColumnWidth);

    //根据首页末页禁用按钮
    void changeBtnEnable();
    //清空数据
    void clear();
    //执行查询
    void select();
    //指定页跳转
    void selectPage(int page);

    //翻页 第一页+上一页+下一页+末一页
    void first();
    void previous();
    void next();
    void last();
};

#endif // DBPAGE_H
