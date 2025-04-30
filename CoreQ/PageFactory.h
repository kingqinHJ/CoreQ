#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H
#include <QWidget>
#include "qsetting.h"
#include "qpromise.h"
#include "httpwidget.h"
#include "callbackdemowidget.h"
#include "jsonwidget.h"
#include "ModernCpp/moderncppwidget.h"

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
        return nullptr;
    }
};

#endif // PAGEFACTORY_H
