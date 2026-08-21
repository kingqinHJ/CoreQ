#pragma once

#include <QWidget>

class QTabWidget;

// 设计模式示例入口页：以 QTabWidget 组织各设计模式的示例页
class DesignPatternsWidget : public QWidget {
    Q_OBJECT
public:
    explicit DesignPatternsWidget(QWidget* parent = nullptr);

private:
    QTabWidget* tabs;
};
