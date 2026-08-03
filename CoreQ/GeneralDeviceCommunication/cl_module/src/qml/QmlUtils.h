#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QJSValue>

class QmlUtils
{
public:
    static QJSEngine *getQmlEngine(QObject *obj);
    static QJSValueList toJS(QObject *obj, QVariantList args);

    // 这里主要避免跨线程的拷贝，出现不可预知的BUG，包括但不限于程序崩溃
    static int storeCallback(QJSValue callback);
    static QJSValue fetchCallback(int id, bool take = false);

    static bool invokeMethod(QObject *obj, int callback_id,
                             QVariantList args, bool async = true, bool lock = false);
    static void invokeMethod(QObject *obj, QJSValue callback, QVariantList args);
};

#endif // QMLUTILS_H
