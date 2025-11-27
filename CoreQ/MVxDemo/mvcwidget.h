#pragma once

#include <QWidget>

class MVCWidget : public QWidget {
    Q_OBJECT
public:
    explicit MVCWidget(QWidget* parent = nullptr);
};

