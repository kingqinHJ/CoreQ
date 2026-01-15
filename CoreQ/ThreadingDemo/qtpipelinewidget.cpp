#include "qtpipelinewidget.h"

QtPipelineWidget::QtPipelineWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *info = new QLabel(
        "<h3>流水线模型 (Pipeline)</h3>"
        "<p>待实现功能：</p>"
        "<ul>"
        "<li>设计多个 Stage (Worker) 串联</li>"
        "<li>Stage A (采集) -> Stage B (处理) -> Stage C (显示)</li>"
        "<li>使用信号槽或线程安全队列传递数据块</li>"
        "</ul>", 
        this);
    info->setAlignment(Qt::AlignCenter);
    info->setTextFormat(Qt::RichText);
    layout->addWidget(info);
}
