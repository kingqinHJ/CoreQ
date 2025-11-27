#pragma once

#include <QWidget>

class QTabWidget;

class MVxDemoWidget : public QWidget {
    Q_OBJECT
public:
    explicit MVxDemoWidget(QWidget* parent = nullptr);

private:
    QTabWidget* tabs;
};

