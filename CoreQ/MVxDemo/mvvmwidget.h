#pragma once

#include <QWidget>

class MVVMWidget : public QWidget {
    Q_OBJECT
public:
    explicit MVVMWidget(QWidget* parent = nullptr);
};

