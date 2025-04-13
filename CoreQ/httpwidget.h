#ifndef HTTPWIDGET_H
#define HTTPWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>
#include "httpmanager.h"

class HttpWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HttpWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    void initConnections();

private:
    HttpManager *httpManager;
    QLineEdit *getUrlEdit;
    QLineEdit *postUrlEdit;
    QLineEdit *formUrlEdit;
    QPushButton *getBtn;
    QPushButton *postBtn;
    QPushButton *formBtn;
    QVBoxLayout *mainLayout;
    QTextEdit *getResultEdit;
    QTextEdit *postResultEdit;
    QTextEdit *formResultEdit;
};

#endif // HTTPWIDGET_H
