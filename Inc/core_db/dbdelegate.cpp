#pragma execution_character_set("utf-8")

#include "dbdelegate.h"
#include "qapplication.h"
#include "qdatetime.h"
#include "qpainter.h"
#include "qevent.h"
#include "qlineedit.h"
#include "qcombobox.h"
#include "qspinbox.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qdatetimeedit.h"
#include "qcalendarwidget.h"
#include "qprogressbar.h"
#include "qtimer.h"
#include "qdebug.h"

#define TIMEMS QTime::currentTime().toString("hh:mm:ss zzz")
DbDelegate::DbDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    delegatePwd = false;
    delegateEdit = false;
    delegateEnable = true;

    delegateMin = 0;
    delegateMax = 100;
    delegateColumn = -1;
    delegateLevel = 0;
    delegateType = "QLineEdit";

    checkBoxChecked = "true";
    checkBoxUnchecked = "false";

    checkColumn = -1;
    checkType = "contains";
    checkValue = "报警";
    checkOkImage = QPixmap(":/image/msg_error.png");
    checkNoImage = QPixmap(":/image/msg_info.png");
    checkImageWidth = 15;
    checkImageHeight = 15;

    //用来设定了特殊背景颜色而又使用了委托的情况
    checkText = false;
    textColor = "#000000";
    selectBgColor = "#F0F0F0";
    checkTextColor = "#FFFFFF";
    checkBgColor = "#FF6B6B";

    //取配色板子默认的颜色
    QWidget *w = (QWidget *)parent;
    if (w != 0) {
        textColor = w->palette().color(QPalette::WindowText).name();
        selectBgColor = w->palette().color(QPalette::Window).name();
    }
}

bool DbDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (qApp->mouseButtons() == Qt::RightButton) {
            emit mousePress(true);
        } else {
            emit mousePress(false);
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

int DbDelegate::getLevel(const QModelIndex &index) const
{
    //找到当前数据模型所在层级
    int level = 0;
    QModelIndex tempIndex = index;
    while (tempIndex.parent().isValid()) {
        level++;
        tempIndex = tempIndex.parent();
    }
    return level;
}

QWidget *DbDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    //设置应用委托最小的层级,比如QTreeWidget
    if (getLevel(index) < delegateLevel) {
        return 0;
    }

    //如果是复选框或者图标则返回0不用处理
    if (!delegateEnable || delegateType == "QCheckBox" || checkColumn >= 0) {
        return 0;
    }

    //在进入编辑状态的时候触发,根据不同的委托类型实例化不同的控件
    QWidget *editor = new QWidget(parent);
    if (delegateType == "QLineEdit") {
        QLineEdit *txt = new QLineEdit(parent);
        connect(txt, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged(QString)));
        txt->setEchoMode(delegatePwd ? QLineEdit::Password : QLineEdit::Normal);
        editor = txt;
    } else if (delegateType == "QComboBox") {
        QComboBox *cbox = new QComboBox(parent);
        connect(cbox, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(valueChanged(QString)));
        cbox->setEditable(delegateEdit);
        cbox->addItems(delegateValue);
        editor = cbox;
    } else if (delegateType == "QSpinBox") {
        QSpinBox *spinbox = new QSpinBox(parent);
        spinbox->setRange(delegateMin, delegateMax);
        connect(spinbox, SIGNAL(valueChanged(QString)), this, SIGNAL(valueChanged(QString)));
        editor = spinbox;
    } else if (delegateType == "QDateTimeEdit") {
        QDateTimeEdit *dateTime = new QDateTimeEdit(parent);
        dateTime->setCalendarPopup(true);
        dateTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
        dateTime->calendarWidget()->setLocale(QLocale::Chinese);
        editor = dateTime;
    } else if (delegateType == "QDateEdit") {
        QDateEdit *date = new QDateEdit(parent);
        date->setCalendarPopup(true);
        date->setDisplayFormat("yyyy-MM-dd");
        date->calendarWidget()->setLocale(QLocale::Chinese);
        editor = date;
    } else if (delegateType == "QTimeEdit") {
        QTimeEdit *time = new QTimeEdit(parent);
        time->setDisplayFormat("HH:mm:ss");
        editor = time;
    }

    return editor;
}

void DbDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //设置应用委托最小的层级,比如QTreeWidget
    if (getLevel(index) < delegateLevel) {
        return;
    }

    //在进入编辑状态的时候触发,将单元格数据赋值给当前控件
    QString data = index.data(Qt::DisplayRole).toString();
    //2020-12-25 增加了关键字对照表
    QString value = getKeyValue(data, true);
    data = value.isEmpty() ? data : value;

    if (delegateType == "QLineEdit") {
        QLineEdit *txt = static_cast<QLineEdit *>(editor);
        txt->setText(data);
    } else if (delegateType == "QComboBox") {
        QComboBox *cbox = static_cast<QComboBox *>(editor);
        cbox->setCurrentIndex(cbox->findText(data));
    } else if (delegateType == "QSpinBox") {
        QSpinBox *spinbox = static_cast<QSpinBox *>(editor);
        spinbox->setValue(data.toInt());
    } else if (delegateType == "QDateTimeEdit") {
        QDateTimeEdit *dateTime = static_cast<QDateTimeEdit *>(editor);
        dateTime->setDateTime(QDateTime::fromString(data, "yyyy-MM-dd HH:mm:ss"));
    } else if (delegateType == "QDateEdit") {
        QDateEdit *date = static_cast<QDateEdit *>(editor);
        date->setDate(QDate::fromString(data, "yyyy-MM-dd"));
    } else if (delegateType == "QTimeEdit") {
        QTimeEdit *time = static_cast<QTimeEdit *>(editor);
        time->setTime(QTime::fromString(data, "HH:mm:ss"));
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void DbDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index)const
{
    //设置应用委托最小的层级,比如QTreeWidget
    if (getLevel(index) < delegateLevel) {
        return;
    }

    //在编辑状态结束的时候触发,将当前控件的数据重新设置到单元格
    QString data = index.data(Qt::DisplayRole).toString();
    if (delegateType == "QLineEdit") {
        QLineEdit *txt = static_cast<QLineEdit *>(editor);
        data = txt->text();
    } else if (delegateType == "QComboBox") {
        QComboBox *cbox = static_cast<QComboBox *>(editor);
        data = cbox->currentText();
    } else if (delegateType == "QSpinBox") {
        QSpinBox *spinbox = static_cast<QSpinBox *>(editor);
        data = QString::number(spinbox->value());
    } else if (delegateType == "QDateTimeEdit") {
        QDateTimeEdit *dateTime = static_cast<QDateTimeEdit *>(editor);
        data = dateTime->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    } else if (delegateType == "QDateEdit") {
        QDateEdit *date = static_cast<QDateEdit *>(editor);
        data = date->date().toString("yyyy-MM-dd");
    } else if (delegateType == "QTimeEdit") {
        QTimeEdit *time = static_cast<QTimeEdit *>(editor);
        data = time->time().toString("HH:mm:ss");
    }

    //2020-12-25 增加了关键字对照表
    QString key = getKeyValue(data, false);
    data = key.isEmpty() ? data : key;
    //将数据设置回数据模型中
    model->setData(index, data);
}

void DbDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //不执行这个生成的控件在坐标原点
    editor->setGeometry(option.rect);
}

bool DbDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    //设置应用委托最小的层级,比如QTreeWidget
    if (getLevel(index) < delegateLevel) {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    //处理编辑数据
    QString data = index.data(Qt::DisplayRole).toString();
    if (delegateType == "QCheckBox") {
        //鼠标松开不用处理,不然会重复设置
        if (event->type() == QEvent::MouseButtonRelease) {
            return false;
        }

        //如果当前的文字和设置的选中文字一致则说明是选中
        checkedText = (data == checkBoxChecked ? checkBoxUnchecked : checkBoxChecked);
        model->setData(index, checkedText, Qt::EditRole);
        //改成了延时发送不然不准确
        //emit valueChanged(checkedText);
        QTimer::singleShot(10, this, SLOT(slot_valueChanged()));
    } else if (delegateType == "QPushButton") {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        mousePoint = mouseEvent->pos();
        mousePressed = false;

        //没有按钮对应的值则不处理
        QStringList list = data.split("|");
        int count = list.count();
        if (data.isEmpty() || count == 0) {
            return false;
        }

        //根据按钮数量自动计算按钮对应的区域
        int x = option.rect.x();
        int y = option.rect.y();
        int width = option.rect.width() / count;
        int height = option.rect.height();

        QList<QRect> btnRect;
        for (int i = 0; i < count; i++) {
            btnRect << QRect(x + i * width, y, width, height);
        }

        if (event->type() == QEvent::MouseButtonPress) {
            //鼠标按下更新对应按钮区域是否按下,并判断按下的区域是哪个按钮,发送对应的按钮索引和index
            for (int i = 0; i < count; i++) {
                if (btnRect.at(i).contains(mousePoint)) {
                    mousePressed = true;
                    emit buttonClicked(i, index);
                    break;
                }
            }
        }

        //需要返回真表示需要继续绘制
        return true;
    }

    //和事件过滤器类似,也需要继续返回执行
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void DbDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //先绘制背景,不然不会应用原有的背景色样式
    drawBg(painter, option, index);

    //设置应用委托最小的层级,比如QTreeWidget
    if (getLevel(index) < delegateLevel) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    //分别处理设置了委托列的
    if (delegateColumn >= 0 && index.column() == delegateColumn) {
        if (delegateType == "QCheckBox") {
            drawCheckBox(painter, option, index);
        } else if (delegateType == "QPushButton") {
            drawPushButton(painter, option, index);
        } else if (delegateType == "QProgressBar") {
            drawProgressBar(painter, option, index);
        } else if (delegateType == "QColor") {
            drawColor(painter, option, index);
        } else if (delegateType == "QLineEdit" && delegatePwd) {
            drawLineEdit(painter, option, index);
        } else if (delegateType == "QPixmap" && checkColumn >= 0) {
            drawPixmap(painter, option, index);
        }

        //绘制关键字对照表
        drawKey(painter, option, index);
        //这个校验数据可以和委托同时设置
        drawText(painter, option, index);
        return;
    }

    //继续父类的绘制
    QStyledItemDelegate::paint(painter, option, index);
}

void DbDelegate::drawBg(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    if ((option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver)) {
        painter->fillRect(option.rect, selectBgColor);
    }
}

void DbDelegate::drawCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //处理复选框委托,判断是否选中,设置选中状态
    QString data = index.data(Qt::EditRole).toString();
    bool checked = (data == checkBoxChecked);
    QStyleOptionButton styleOption;
    styleOption.initFrom(checkBox);
    styleOption.state = checked ? QStyle::State_On : QStyle::State_Off;
    styleOption.state |= QStyle::State_Enabled;

    //设置居中正方形区域
    int width = option.rect.width();
    int height = option.rect.height();
    int size = qMin(width, height);
    //奇怪这里理论上是 /2 结果发现是 /3 更居中 不知道啥情况
    int x = option.rect.center().x() - size / 3;
    int y = option.rect.y();
    styleOption.rect = QRect(x, y, size, size);
    //styleOption.rect = option.rect;

    //第四个参数也可以为空,为空的话就不能应用样式表
    //一般采用 drawControl 方法, drawPrimitive 会拉伸填充
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &styleOption, painter, checkBox);
    //QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &styleOption, painter, checkBox);
}

void DbDelegate::drawPushButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //没有按钮对应的值则不处理
    QString data = index.data(Qt::DisplayRole).toString();
    QStringList list = data.split("|");
    int count = list.count();
    if (data.isEmpty() || count == 0) {
        return;
    }

    //处理按钮集合委托,自动计算分配宽度
    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width() / count;
    int height = option.rect.height();

    //分别绘制按钮集合
    for (int i = 0; i < count; i++) {
        QStyleOptionButton styleOption;
        styleOption.initFrom(pushButton);
        styleOption.rect = QRect(x + i * width, y, width, height);
        styleOption.text = list.at(i);
        styleOption.state |= QStyle::State_Enabled;

        //鼠标按下更新对应的样式
        if (mousePressed) {
            if (styleOption.rect.contains(mousePoint)) {
                styleOption.state |= QStyle::State_Sunken;
            }
        }

        //第四个参数也可以为空,为空的话就不能应用样式表
        QApplication::style()->drawControl(QStyle::CE_PushButton, &styleOption, painter, pushButton);
    }
}

void DbDelegate::drawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //处理进度条委托
    int data = index.model()->data(index, Qt::DisplayRole).toInt();
    QStyleOptionProgressBar styleOption;
    styleOption.initFrom(progressBar);
    styleOption.minimum = 0;
    styleOption.maximum = 100;
    styleOption.progress = data;
    styleOption.state |= QStyle::State_Enabled;
    styleOption.rect = option.rect;
    styleOption.textVisible = true;
    styleOption.text = QString("%1%").arg(data);
    styleOption.textAlignment = Qt::AlignCenter;

    //第四个参数也可以为空,为空的话就不能应用样式表
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &styleOption, painter, progressBar);
}

void DbDelegate::drawColor(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //取出颜色值进行绘制
    QString data = index.model()->data(index, Qt::DisplayRole).toString();
    data = (data == "transparent") ? "black" : data;
    QColor color(data);
    if (color.isValid()) {
        //根据背景色自动计算合适的前景色
        double gray = (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
        QColor textColor = gray > 0.5 ? Qt::black : Qt::white;

        //如果设置过颜色则需要先save后restore,否则会改变全局的painter
        painter->save();
        painter->fillRect(option.rect, color);
        painter->setPen(textColor);
        painter->drawText(option.rect, Qt::AlignCenter, color.name().toUpper());
        painter->restore();
    }
}

void DbDelegate::drawLineEdit(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //处理密码委托,将明文转为密文显示
    QString data = index.model()->data(index, Qt::DisplayRole).toString();
    int len = data.length();
    QString text;
    for (int i = 0; i < len; i++) {
        text += "●";
    }

    QRect rect = option.rect;
    rect.setX(rect.x() + 2);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
}

void DbDelegate::drawPixmap(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //处理图标委托,根据不同的过滤条件显示不同的图标
    //获取参照列的字符串数据,根据过滤规则判断,符合关键字则错误图标,否则正确图标
    QString data = index.model()->index(index.row(), checkColumn).data().toString();
    bool exist;
    checkData(data, &exist);

    //图标自动缩放到指定大小
    QPixmap pixmap = (exist ? checkOkImage : checkNoImage);
    pixmap = pixmap.scaled(QSize(checkImageWidth, checkImageHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignCenter, pixmap);
}

QString DbDelegate::getKeyValue(const QString &data, bool key) const
{
    //根据关键字对照表分别绘制成设定的文本
    //比如原文是 0 1 分别要绘制成 禁用 启用 则 delegateKey = QStringList() << "0|禁用" << "1|启用"
    QString result;
    int count = delegateKey.count();
    for (int i = 0; i < count; ++i) {
        QStringList list = delegateKey.at(i).split("|");
        if (key) {
            if (list.at(0) == data) {
                result = list.at(1);
                break;
            }
        } else {
            if (list.at(1) == data) {
                result = list.at(0);
                break;
            }
        }
    }
    return result;
}

void DbDelegate::drawKey(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString data = index.data(Qt::DisplayRole).toString();
    QString value = getKeyValue(data, true);
    if (value.isEmpty()) {
        return;
    }

    QFont font = index.data(Qt::FontRole).value<QFont>();
    int align = index.data(Qt::TextAlignmentRole).toInt();
    if (align == 0) {
        align = (Qt::AlignLeft | Qt::AlignVCenter);
    }

    QRect rect = option.rect;
    rect.setX(rect.x() + 2);

    painter->save();
    painter->setFont(font);
    painter->drawText(rect, align, value);
    painter->restore();
}

void DbDelegate::checkData(const QString &data, bool *exist) const
{
    *exist = false;
    if (checkType == "==") {
        if (data == checkValue) {
            *exist = true;
        }
    } else if (checkType == ">") {
        if (data > checkValue) {
            *exist = true;
        }
    } else if (checkType == ">=") {
        if (data >= checkValue) {
            *exist = true;
        }
    } else if (checkType == "<") {
        if (data < checkValue) {
            *exist = true;
        }
    } else if (checkType == "<=") {
        if (data <= checkValue) {
            *exist = true;
        }
    } else if (checkType == "!=") {
        if (data != checkValue) {
            *exist = true;
        }
    } else if (checkType == "contains") {
        QStringList list = checkValue.split("|");
        foreach (QString value, list) {
            if (data.contains(value)) {
                *exist = true;
                break;
            }
        }
    }
}

void DbDelegate::drawText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!checkText) {
        return;
    }

    QString data = index.data(Qt::DisplayRole).toString();
    QFont font = index.data(Qt::FontRole).value<QFont>();
    int align = index.data(Qt::TextAlignmentRole).toInt();
    if (align == 0) {
        align = (Qt::AlignLeft | Qt::AlignVCenter);
    }

    QBrush brush = Qt::NoBrush;
    if ((option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver)) {
        brush = QBrush(selectBgColor);
    }

    QColor color = textColor;
    bool exist;
    checkData(data, &exist);

    //过滤到关键字则设置对应的颜色和字体
    if (exist) {
        brush = QBrush(checkBgColor);
        color = checkTextColor;

        //字体加粗以及字号+1
        font.setBold(true);
        if (font.pixelSize() > 0) {
            font.setPixelSize(font.pixelSize() + 1);
        } else {
            font.setPointSize(font.pointSize() + 1);
        }
    }

    QRect rect = option.rect;
    rect.setX(rect.x() + 2);

    //如果设置过颜色则需要先save后restore,否则会改变全局的painter
    painter->save();
    painter->setPen(color);
    painter->setFont(font);
    painter->fillRect(option.rect, brush);
    painter->drawText(rect, align, data);
    painter->restore();
}

void DbDelegate::slot_valueChanged()
{
    emit valueChanged(checkedText);
}

void DbDelegate::setDelegatePwd(bool delegatePwd)
{
    this->delegatePwd = delegatePwd;
}

void DbDelegate::setDelegateEdit(bool delegateEdit)
{
    this->delegateEdit = delegateEdit;
}

void DbDelegate::setDelegateEnable(bool delegateEnable)
{
    this->delegateEnable = delegateEnable;
}

void DbDelegate::setDelegateMin(int delegateMin)
{
    this->delegateMin = delegateMin;
}

void DbDelegate::setDelegateMax(int delegateMax)
{
    this->delegateMax = delegateMax;
}

void DbDelegate::setDelegateColumn(int delegateColumn)
{
    this->delegateColumn = delegateColumn;
}

void DbDelegate::setDelegateLevel(int delegateLevel)
{
    this->delegateLevel = delegateLevel;
}

void DbDelegate::setDelegateType(const QString &delegateType)
{
    //设置了对象名称方便设置全局样式
    //比如一般其他地方按钮有圆角而在委托中不需要圆角会更合适这就需要单独设置样式
    //qApp->setStyleSheet("#DbDelegate_pushButton{border-radius:0px;margin:1px;padding:1px;}");
    this->delegateType = delegateType;
    if (delegateType == "QCheckBox") {
        checkBox = new QCheckBox;
        checkBox->setObjectName("DbDelegate_checkBox");
    } else if (delegateType == "QPushButton") {
        pushButton = new QPushButton;
        pushButton->setObjectName("DbDelegate_pushButton");
    } else if (delegateType == "QProgressBar") {
        progressBar = new QProgressBar;
        progressBar->setObjectName("DbDelegate_progressBar");
    }
}

void DbDelegate::setDelegateKey(const QStringList &delegateKey)
{
    this->delegateKey = delegateKey;
}

void DbDelegate::setDelegateValue(const QStringList &delegateValue)
{
    this->delegateValue = delegateValue;
}

void DbDelegate::setCheckBoxText(const QString &checkBoxChecked, const QString &checkBoxUnchecked)
{
    this->checkBoxChecked = checkBoxChecked;
    this->checkBoxUnchecked = checkBoxUnchecked;
}

void DbDelegate::setCheckColumn(int checkColumn)
{
    this->checkColumn = checkColumn;
}

void DbDelegate::setCheckType(const QString &checkType)
{
    this->checkType = checkType;
}

void DbDelegate::setCheckValue(const QString &checkValue)
{
    this->checkValue = checkValue;
}

void DbDelegate::setCheckOkImage(const QPixmap &checkOkImage)
{
    this->checkOkImage = checkOkImage;
}

void DbDelegate::setCheckNoImage(const QPixmap &checkNoImage)
{
    this->checkNoImage = checkNoImage;
}

void DbDelegate::setCheckImageWidth(int checkImageWidth)
{
    this->checkImageWidth = checkImageWidth;
}

void DbDelegate::setCheckImageHeight(int checkImageHeight)
{
    this->checkImageHeight = checkImageHeight;
}

void DbDelegate::setCheckText(bool checkText)
{
    this->checkText = checkText;
}

void DbDelegate::setTextColor(const QColor &textColor)
{
    this->textColor = textColor;
}

void DbDelegate::setSelectBgColor(const QColor &selectBgColor)
{
    this->selectBgColor = selectBgColor;
}

void DbDelegate::setCheckTextColor(const QColor &checkTextColor)
{
    this->checkTextColor = checkTextColor;
}

void DbDelegate::setCheckBgColor(const QColor &checkBgColor)
{
    this->checkBgColor = checkBgColor;
}
