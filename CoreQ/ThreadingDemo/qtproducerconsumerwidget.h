#ifndef QTPRODUCERCONSUMERWIDGET_H
#define QTPRODUCERCONSUMERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class QtProducerConsumerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtProducerConsumerWidget(QWidget *parent = nullptr);

private:
    QLabel *m_infoLabel;
};

#endif // QTPRODUCERCONSUMERWIDGET_H
