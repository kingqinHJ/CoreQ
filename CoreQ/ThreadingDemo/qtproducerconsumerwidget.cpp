#include "qtproducerconsumerwidget.h"

QtProducerConsumerWidget::QtProducerConsumerWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_infoLabel = new QLabel(
        "<h3>生产者-消费者模型 (Qt版)</h3>"
        "<p>待实现功能：</p>"
        "<ul>"
        "<li>使用 <b>QWaitCondition</b> 实现线程等待与唤醒</li>"
        "<li>使用 <b>QMutex</b> 保护共享缓冲区</li>"
        "<li>演示 <b>QSemaphore</b> 作为一个高级替代方案</li>"
        "</ul>"
        "<p><i>TODO: 请参考 ConditionVariableWidget 实现 Qt 原生版本</i></p>", 
        this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setTextFormat(Qt::RichText);
    layout->addWidget(m_infoLabel);
}
