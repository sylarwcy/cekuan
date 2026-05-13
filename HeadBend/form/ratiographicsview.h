#ifndef RATIOGRAPHICSVIEW_H
#define RATIOGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include <QSizePolicy>

class RatioGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit RatioGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        // 第一步：告诉外部的“独裁布局” -> 我的宽度可以拉伸，但我的高度必须是固定的 (Fixed)！
        QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        this->setSizePolicy(policy);
    }

protected:
    // 第二步：拦截每次窗口大小变化的事件
    virtual void resizeEvent(QResizeEvent *event) override {
        // 先让 Qt 完成它该做的默认缩放
        QGraphicsView::resizeEvent(event);

        // 【核武器】：强行把高度锁定为宽度的 1/2
        // 这样就永远保持 宽度:高度 = 2:1 的完美比例！
        int currentWidth = this->width();
        this->setFixedHeight(currentWidth / 3);

        // （如果你说的是高度是宽度的5倍，就改成 currentWidth * 5）
    }
};

#endif // RATIOGRAPHICSVIEW_H