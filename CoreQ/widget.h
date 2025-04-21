#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets>
#include "qsetting.h"
#include "qpromise.h"
#include "httpwidget.h"
#include "callbackdemowidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onNavigationItemClicked(int row);

private:
    void setupUI();
    void initConnections();
    void createPages();

private:
    Ui::Widget *ui;
    QListWidget *navigationList;
    QStackedWidget *stackedWidget;
    QHBoxLayout *mainLayout;
    QMap<QString, QWidget*> mpages;
    // 功能页面
    QSetting *settingPage;
    QPromise *promisePage;
    HttpWidget *httpPage;
    CallbackDemoWidget *callbackDemoWidget;
};
#endif // WIDGET_H
