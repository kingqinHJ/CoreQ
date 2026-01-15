#include "qtdiningphilosopherswidget.h"

QtDiningPhilosophersWidget::QtDiningPhilosophersWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *info = new QLabel(
        "<h3>哲学家就餐 / 死锁演示</h3>"
        "<p>待实现功能：</p>"
        "<ul>"
        "<li>模拟多个线程竞争有限资源</li>"
        "<li>演示死锁的产生 (循环等待)</li>"
        "<li>演示解决方案 (资源层级法 / tryLock)</li>"
        "</ul>", 
        this);
    info->setAlignment(Qt::AlignCenter);
    info->setTextFormat(Qt::RichText);
    layout->addWidget(info);
}
