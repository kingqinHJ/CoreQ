#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H
#include <QWidget>
#include "qsetting.h"
#include "qpromise.h"
#include "httpwidget.h"
#include "callbackdemowidget.h"
#include "DataProcess/jsonwidget.h"
#include "ModernCpp/moderncppwidget.h"
#include "ThreadingDemo/threadingdemowidget.h"
#include "MVxDemo/mvcwidget.h"
#include "MVxDemo/mvpwidget.h"
#include "MVxDemo/mvvmwidget.h"
#include "MVxDemo/mvxdemowidget.h"

class PageFactory{
public:
    static QWidget* CreateWidget(const QString &type,QWidget *parent)
    {
        if(type=="Settings")  return new QSetting(parent);
        if (type == "Promise") return new QPromise(parent);
        if (type == "Http") return new HttpWidget(parent);
        if (type == "Callback") return new CallbackDemoWidget(parent);
        if (type == "Json") return new JsonWidget(parent);
        if (type == "ModernCpp") return new ModernCppWidget(parent);
        if (type == "ThreadingDemo") return new ThreadingDemoWidget(parent);
        if (type == "MVxDemo") return new MVxDemoWidget(parent);
        return nullptr;
    }
};

#endif // PAGEFACTORY_H
