#include "designpatternswidget.h"
#include "TemplateMethod/templatemethodwidget.h"

#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>

DesignPatternsWidget::DesignPatternsWidget(QWidget* parent)
    : QWidget(parent)
    , tabs(new QTabWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs);

    tabs->addTab(new QLabel(QStringLiteral("设计模式示例（待补充）"), this),
                 QStringLiteral("概览"));
    tabs->addTab(new TemplateMethodWidget(this),
                 QStringLiteral("模板方法模式"));
}
