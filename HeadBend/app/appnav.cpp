#include "appnav.h"
#include "quihelper.h"

//可以自行更改这两个值切换导航按钮样式
bool AppNav::NavMainLeft = false;
bool AppNav::NavSubLeft = false;

void AppNav::initNavLayout(QWidget *widget, QLayout *layout, bool left)
{
    //不同的样式边距和图标大小不一样,可以自行调整对应值看效果
    int topMargin, otherMargin;
    if (left) {

    } else {
        if (AppNav::NavMainLeft) {
            topMargin = widget->height() + 6;
            otherMargin = 9;
        } else {
            topMargin = 3;
            otherMargin = 3;
        }
    }

    layout->setContentsMargins(otherMargin, topMargin, otherMargin, otherMargin);
}

void AppNav::initNavBtn(QToolButton *btn, const QString &name, const QString &text, bool left)
{
    btn->setObjectName(name);
    btn->setText(text);
    btn->setCheckable(true);

    //顶部导航和左侧导航拉伸策略不一样
    if (left) {
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        if (AppNav::NavSubLeft) {
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setMinimumHeight(30);
        } else {
            btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            btn->setMinimumHeight(80);
        }

    } else {
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        if (AppNav::NavMainLeft) {
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setMinimumWidth(115);
        } else {
            btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            btn->setMinimumWidth(80);
        }
    }
}

void AppNav::initNavBtnIcon(QAbstractButton *btn, int icon, bool left, int offset)
{
    //可以自行调整尺寸
    int size = 20;
    int width = 25;
    int height = 20;

    //根据不同的图标位置设置不同的图标尺寸
    if (left) {
        if (!AppNav::NavSubLeft) {
            size = 35;
            width = 40;
            height = 35;
        }
    } else {
        if (!AppNav::NavMainLeft) {
            size = 35;
            width = 40;
            height = 35;
        }
    }

    //偏移值 可能部分图标要调整大小
    size += offset;
    setIconBtn(btn, icon, size, width, height);
}

void AppNav::setIconBtn(QAbstractButton *btn, int icon)
{
    //可以自行调整尺寸
    int size = 16;
    int width = 18;
    int height = 18;
    setIconBtn(btn, icon, size, width, height);
}

void AppNav::setIconBtn(QAbstractButton *btn, int icon, int size, int width, int height)
{
    QPixmap pix = IconHelper::getPixmap(QUIConfig::TextColor, icon, size, width, height);
    btn->setIconSize(QSize(width, height));
    btn->setIcon(QIcon(pix));
}
