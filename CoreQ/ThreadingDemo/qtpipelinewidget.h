#ifndef QTPIPELINEWIDGET_H
#define QTPIPELINEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class QtPipelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtPipelineWidget(QWidget *parent = nullptr);
};

#endif // QTPIPELINEWIDGET_H
