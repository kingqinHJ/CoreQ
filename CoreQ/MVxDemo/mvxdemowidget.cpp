#include "mvxdemowidget.h"
#include "mvcwidget.h"
#include "mvpwidget.h"
#include "mvvmwidget.h"

#include <QTabWidget>
#include <QVBoxLayout>

MVxDemoWidget::MVxDemoWidget(QWidget* parent)
    : QWidget(parent)
    , tabs(new QTabWidget(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    setLayout(layout);

    tabs->addTab(new MVCWidget(this), QString::fromUtf8("MVC"));
    tabs->addTab(new MVPWidget(this), QString::fromUtf8("MVP"));
    tabs->addTab(new MVVMWidget(this), QString::fromUtf8("MVVM"));
}

