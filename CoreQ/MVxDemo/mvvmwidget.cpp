#include "mvvmwidget.h"

#include <QVBoxLayout>
#include <QLabel>

MVVMWidget::MVVMWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel("MVVM 示例", this);
    title->setStyleSheet("font-weight: bold; font-size: 16px;");
    auto* desc = new QLabel("Model-View-ViewModel：通过数据绑定/观察者减少样板代码，ViewModel承载状态。", this);
    desc->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(desc);
    setLayout(layout);
}

