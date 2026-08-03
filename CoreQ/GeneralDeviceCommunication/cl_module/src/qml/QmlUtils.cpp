#include "QmlUtils.h"
#include <QtQml>
#include <QMutex>

static int callback_max_id = 0;
#ifndef Q_OS_WASM
static QMutex callback_mutex;
#endif
static QMap<int, QJSValue> callback_maps;

QJSEngine *QmlUtils::getQmlEngine(QObject *obj)
{
    QJSEngine *engine = NULL;

    QObject *__obj = obj;
    while (__obj) {
        engine = qmlEngine(__obj);
        if (engine)
            break;
        else
            __obj = obj->parent();
    }

    return engine;
}

QJSValueList QmlUtils::toJS(QObject *obj, QVariantList args)
{
    QJSEngine *engine = getQmlEngine(obj);
    if (!engine) {
        LOGD("invalid engine.");
        return QJSValueList();
    }

    QJSValueList js_args;
    for (auto &item: args)
        js_args.append(engine->toScriptValue<QVariant>(item));
    return js_args;
}

int QmlUtils::storeCallback(QJSValue callback)
{
    if (callback.isCallable()) {
#ifndef Q_OS_WASM
        QMutexLocker locker(&callback_mutex);
#endif
        int id = callback_max_id++;
        callback_maps.insert(id, callback);
        return id;
    }
    else {
        return -1;
    }
}

QJSValue QmlUtils::fetchCallback(int id, bool take)
{
#ifndef Q_OS_WASM
    QMutexLocker locker(&callback_mutex);
#endif
    if (callback_maps.contains(id)) {
        if (take) return callback_maps.take(id);
        else return callback_maps.value(id);
    }
    else
        return QJSValue();
}

bool QmlUtils::invokeMethod(QObject *obj, int callback_id,
                            QVariantList args, bool async, bool lock)
{
    if (callback_id == -1)
        return false;

    QMetaObject::invokeMethod(obj, [=]() {
        QJSValue callback = fetchCallback(callback_id, !lock);
        if (callback.isCallable()) {
            QJSEngine *engine = qmlEngine(obj);
#if QT_VERSION_MAJOR < 6
            if (!engine)
                engine = callback.engine();
#endif

            if (!engine) {
                LOGD("invalid engine.");
                return;
            }

            QJSValueList js_args;
            for (auto &item: args) {
                js_args.append(engine->toScriptValue<QVariant>(item));
            }

            callback.call(js_args);
        }
        else {
            LOGD("invalid callback: %d", callback_id);
        }

    }, async ? Qt::QueuedConnection:
               Qt::BlockingQueuedConnection);
    return true;
}

void QmlUtils::invokeMethod(QObject *obj, QJSValue callback, QVariantList args)
{
    if (callback.isCallable()) {
        QJSEngine *engine = qmlEngine(obj);
#if QT_VERSION_MAJOR < 6
        if (!engine)
            engine = callback.engine();
#endif

        if (!engine) {
            LOGD("invalid engine.");
            return;
        }

        QJSValueList js_args;
        for (auto &item: args) {
            js_args.append(engine->toScriptValue<QVariant>(item));
        }

        callback.call(js_args);
    }
    else {
        LOGD("invalid callback: %p", obj);
    }
}
