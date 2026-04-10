#ifndef APPNAV_H
#define APPNAV_H

#include "head.h"

class AppNav
{
public:
    //主导航按钮图标左侧
    static bool NavMainLeft;
    //子导航按钮图标左侧
    static bool NavSubLeft;

    //初始化导航布局
    static void initNavLayout(QWidget *widget, QLayout *layout, bool left);
    //初始化导航按钮
    static void initNavBtn(QToolButton *btn, const QString &name, const QString &text, bool left);
    //初始化导航按钮图标
    static void initNavBtnIcon(QAbstractButton *btn, int icon, bool left, int offset = 0);

    //设置图形字体图标
    static void setIconBtn(QAbstractButton *btn, int icon);
    static void setIconBtn(QAbstractButton *btn, int icon, int size, int width, int height);
};

#endif // APPNAV_H
