#ifndef JSONWIDGET_H
#define JSONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class JsonWidget : public QWidget
{
    Q_OBJECT

public:
    JsonWidget(QWidget *parent = nullptr);
    ~JsonWidget();

private slots:
    void onGenerateJsonClicked();
    void onWriteJsonFileClicked();

private:
    void setupUI();

    QVBoxLayout *mainLayout;
    QPushButton *generateJsonBtn;
    QPushButton *writeJsonFileBtn;
    QLabel *resultLabel;
};

#endif // JSONWIDGET_H
