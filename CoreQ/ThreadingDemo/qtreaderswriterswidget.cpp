#include "qtreaderswriterswidget.h"

QtReadersWritersWidget::QtReadersWritersWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *info = new QLabel(
        "<h3>读写者模型 (Readers-Writers)</h3>"
        "<p>待实现功能：</p>"
        "<ul>"
        "<li>使用 <b>QReadWriteLock</b></li>"
        "<li>对比 QReadLocker (多线程并发读) 与 QWriteLocker (独占写)</li>"
        "<li>演示读多写少场景下的性能优势</li>"
        "</ul>", 
        this);
    info->setAlignment(Qt::AlignCenter);
    info->setTextFormat(Qt::RichText);
    layout->addWidget(info);
}
