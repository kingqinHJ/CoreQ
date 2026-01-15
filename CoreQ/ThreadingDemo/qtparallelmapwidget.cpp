#include "qtparallelmapwidget.h"

QtParallelMapWidget::QtParallelMapWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *info = new QLabel(
        "<h3>并行计算 / Map-Reduce</h3>"
        "<p>待实现功能：</p>"
        "<ul>"
        "<li>使用 <b>QtConcurrent::map</b> 处理大列表</li>"
        "<li>使用 <b>QtConcurrent::mappedReduced</b> 进行结果汇总</li>"
        "<li>结合 QFutureWatcher 显示实时进度</li>"
        "</ul>", 
        this);
    info->setAlignment(Qt::AlignCenter);
    info->setTextFormat(Qt::RichText);
    layout->addWidget(info);
}
