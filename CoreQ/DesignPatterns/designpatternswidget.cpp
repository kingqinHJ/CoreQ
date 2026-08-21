#include "designpatternswidget.h"

#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>

DesignPatternsWidget::DesignPatternsWidget(QWidget* parent)
    : QWidget(parent)
    , tabs(new QTabWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabs);

    // 后续各设计模式示例页在此通过 tabs->addTab(...) 添加
    tabs->addTab(new QLabel(QStringLiteral("设计模式示例（待补充）"), this),
                 QStringLiteral("概览"));
}
