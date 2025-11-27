#include "mvpwidget.h"

#include <QVBoxLayout>
#include <QLabel>

MVPWidget::MVPWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel("MVP 示例", this);
    title->setStyleSheet("font-weight: bold; font-size: 16px;");
    auto* desc = new QLabel("Model-View-Presenter：Presenter 负责业务逻辑与View交互，提升可测试性。", this);
    desc->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(desc);
    setLayout(layout);
}

