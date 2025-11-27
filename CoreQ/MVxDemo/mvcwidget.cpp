#include "mvcwidget.h"

#include <QVBoxLayout>
#include <QLabel>

MVCWidget::MVCWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel("MVC 示例", this);
    title->setStyleSheet("font-weight: bold; font-size: 16px;");
    auto* desc = new QLabel("Model-View-Controller：将数据(Model)、视图(View)、控制逻辑(Controller)分离。", this);
    desc->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(desc);
    setLayout(layout);
}

