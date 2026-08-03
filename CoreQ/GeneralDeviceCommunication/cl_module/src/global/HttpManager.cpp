#include "HttpManager.h"

#ifndef Q_OS_WASM
#include <QThread>
#include <QEventLoop>
#include <QPointer>
#endif

class HttpManagerPrivate
{
public:
    bool started = false;
#ifndef Q_OS_WASM
    QThread worker_thread;
#endif

    std::shared_ptr<QNetworkAccessManager> network_manager;
    QHash<QNetworkReply*, std::shared_ptr<HttpManagerReplyWrapper>> task_list;
};

static int s_max_id = 0;
HttpManager *HttpManager::self = nullptr;
HttpManager::HttpManager(QObject *parent) : QObject(parent)
{
    d.reset(new HttpManagerPrivate);
    self = this;

#ifndef Q_OS_WASM
    d->worker_thread.setObjectName("HttpManager");
    d->worker_thread.moveToThread(&d->worker_thread);
    d->worker_thread.start();
#endif
}

HttpManager::~HttpManager()
{
    stop();
#ifndef Q_OS_WASM
    d->worker_thread.quit();
    d->worker_thread.wait();
#endif
    // LOG_THIS();

    self = nullptr;
}

bool HttpManager::start()
{
    if (d->started)
        return true;

    d->started = true;

    auto func = [&]() {
        d->network_manager.reset(new QNetworkAccessManager);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 2)
        d->network_manager->setAutoDeleteReplies(false);
#endif

        connect(d->network_manager.get(), &QNetworkAccessManager::finished,
                d->network_manager.get(), [this](QNetworkReply *reply){
                    d->task_list.remove(reply);
                });
    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif

    return true;
}

void HttpManager::stop()
{
    if (!d->started)
        return;

    d->started = false;

    auto func = [&]() {
        if (d->network_manager.get() == NULL)
            return;

        // 终止所有任务
        auto tasks = d->task_list;
        for (auto &task: tasks) {
            task->abort();
            task->reset();
        }
        d->task_list.clear();

        d->network_manager->disconnect();
        d->network_manager.reset();
    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif
}

void HttpManager::abort()
{
    if (!d->started)
        return;

#ifndef Q_OS_WASM
    QMetaObject::invokeMethod(&d->worker_thread, [&]()
    {
#endif
        // 终止所有任务
        auto tasks = d->task_list;
        for (auto &task: tasks)
          task->abort();
#ifndef Q_OS_WASM
    }, Qt::BlockingQueuedConnection);
#endif
}

HttpManagerReplyWrapperPtr HttpManager::send(const HttpManagerRequest &req)
{
    switch (req.method) {
    case HTTP_GET:
        return get(req.url, req.headers, req.print_mode);
    case HTTP_POST:
#if QT_CONFIG(http)
        if (req.multi_part)
            return post(req.url, req.multi_part, req.headers, req.print_mode);
        else
#endif
            if (req.dev)
                return post(req.url, req.dev, req.headers, req.print_mode);
            else
                return post(req.url, req.body, req.headers, req.print_mode);
    case HTTP_DELETE:
        return deleteResource(req.url, req.headers, req.print_mode);
    default:
        break;
    }

    return nullptr;
}

std::shared_ptr<HttpManagerReplyWrapper> HttpManager::get(QString url, HttpManagerHeaders headers,
                                                          int mode)
{
    std::shared_ptr<HttpManagerReplyWrapper> task;
    auto func = [&]()
    {
        if (d->network_manager.get() == NULL)
            return;

        int id = s_max_id++;
        QString msg;
        msg.append(QString("request(%1):\nurl: %2\n").arg(id).arg(url));

        QNetworkRequest req;
#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 2)
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
        req.setUrl(url);
        for (auto it=headers.begin(); it!=headers.end(); it++) {
            msg.append(QString("     %1: %2\n").arg(it.key().data(), it.value().data()));
            req.setRawHeader(it.key(), it.value());
        }
        auto reply = d->network_manager->get(req);

        if (mode & PRINT_REQUEST)
            LOGD("%s", qUtf8Printable(msg));

        task.reset(new HttpManagerReplyWrapper(reply, id), [](HttpManagerReplyWrapper *ptr){
            ptr->deleteLater();
        });
        task->onFinished([id, mode, url](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError)
                LOGD("error(%s):", qUtf8Printable(url)) << reply->errorString();

            if (mode & PRINT_RESPONSE) {
                auto data = reply->readAll();
                LOGD("response(%d):\n%s", id, data.data());
            }
        });
        d->task_list.insert(task->reply(), task);

    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif

    return task;
}

std::shared_ptr<HttpManagerReplyWrapper> HttpManager::post(QString url, QByteArray body,
                                                           HttpManagerHeaders headers,
                                                           int mode)
{
    std::shared_ptr<HttpManagerReplyWrapper> task;
    auto func = [&]()
    {
        if (d->network_manager.get() == NULL)
            return;

        int id = s_max_id++;
        QString msg;
        msg.append(QString("request(%1):\nurl: %2\n").arg(id).arg(url));

        QNetworkRequest req;
#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 2)
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
        req.setUrl(url);
        req.setRawHeader("content-type", "application/json");
        for (auto it=headers.begin(); it!=headers.end(); it++) {
            msg.append(QString("     %1: %2\n").arg(it.key().data(), it.value().data()));
            req.setRawHeader(it.key(), it.value());
        }

        if (body.size() < 10*1024)
            msg.append(QString("body: %1\n").arg(body.data()));
        else
            msg.append(QString("body: %1 bytes\n").arg(body.size()));

        auto reply = d->network_manager->post(req, body);

        if (mode & PRINT_REQUEST)
            LOGD("%s", qUtf8Printable(msg));

        task.reset(new HttpManagerReplyWrapper(reply, id), [](HttpManagerReplyWrapper *ptr){
            ptr->deleteLater();
        });
        task->onFinished([id, mode, url](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError)
                LOGD("error(%s):", qUtf8Printable(url)) << reply->errorString();

            if (mode & PRINT_RESPONSE) {
                auto data = reply->readAll();
                LOGD("response(%d):\n%s", id, data.data());
            }
        });
        d->task_list.insert(task->reply(), task);

    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif

    return task;
}

std::shared_ptr<HttpManagerReplyWrapper> HttpManager::post(QString url, QIODevice *data,
                                                           HttpManagerHeaders headers,
                                                           int mode)
{
    std::shared_ptr<HttpManagerReplyWrapper> task;
    auto func = [&]()
    {
        if (d->network_manager.get() == NULL)
            return;

        int id = s_max_id++;
        QString msg;
        msg.append(QString("request(%1):\nurl: %2\n").arg(id).arg(url));

        QNetworkRequest req;
#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 2)
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
        req.setUrl(url);
        for (auto it=headers.begin(); it!=headers.end(); it++) {
            msg.append(QString("     %1: %2\n").arg(it.key().data(), it.value().data()));
            req.setRawHeader(it.key(), it.value());
        }
        auto reply = d->network_manager->post(req, data);

        if (mode & PRINT_REQUEST)
            LOGD("%s", qUtf8Printable(msg));

        task.reset(new HttpManagerReplyWrapper(reply, id), [](HttpManagerReplyWrapper *ptr){
            ptr->deleteLater();
        });
        task->onFinished([id, mode, url](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError)
                LOGD("error(%s):", qUtf8Printable(url)) << reply->errorString();

            if (mode & PRINT_RESPONSE) {
                auto data = reply->readAll();
                LOGD("response(%d):\n%s", id, data.data());
            }
        });
        d->task_list.insert(task->reply(), task);

    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif

    return task;
}

#if QT_CONFIG(http)
std::shared_ptr<HttpManagerReplyWrapper> HttpManager::post(QString url, QHttpMultiPart *multi_part,
                                                           HttpManagerHeaders headers,
                                                           int mode)
{
    std::shared_ptr<HttpManagerReplyWrapper> task;

    multi_part->moveToThread(&d->worker_thread);
    QMetaObject::invokeMethod(&d->worker_thread, [&]()
    {
        if (d->network_manager.get() == NULL)
            return;

        int id = s_max_id++;
        QString msg;
        msg.append(QString("request(%1):\nurl: %2\n").arg(id).arg(url));

        QNetworkRequest req;
#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 2)
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
        req.setUrl(url);
        for (auto it=headers.begin(); it!=headers.end(); it++) {
            msg.append(QString("     %1: %2\n").arg(it.key().data(), it.value().data()));
            req.setRawHeader(it.key(), it.value());
        }
        auto reply = d->network_manager->post(req, multi_part);

        if (mode & PRINT_REQUEST)
            LOGD("%s", qUtf8Printable(msg));

        task.reset(new HttpManagerReplyWrapper(reply, id), [](HttpManagerReplyWrapper *ptr){
            ptr->deleteLater();
        });
        task->onFinished([id, mode, url](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError)
                LOGD("error(%s):", qUtf8Printable(url)) << reply->errorString();

            if (mode & PRINT_RESPONSE) {
                auto data = reply->readAll();
                LOGD("response(%d):\n%s", id, data.data());
            }
        });
        d->task_list.insert(task->reply(), task);
        multi_part->setParent(reply);

    }, Qt::BlockingQueuedConnection);

    return task;
}
#endif

std::shared_ptr<HttpManagerReplyWrapper> HttpManager::deleteResource(QString url, HttpManagerHeaders headers,
                                                                     int mode)
{
    std::shared_ptr<HttpManagerReplyWrapper> task;
    auto func = [&]()
    {
        if (d->network_manager.get() == NULL)
            return;

        int id = s_max_id++;
        QString msg;
        msg.append(QString("request(%1):\nurl: %2\n").arg(id).arg(url));

        QNetworkRequest req;
#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 2)
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
        req.setUrl(url);
        for (auto it=headers.begin(); it!=headers.end(); it++) {
            msg.append(QString("     %1: %2\n").arg(it.key().data(), it.value().data()));
            req.setRawHeader(it.key(), it.value());
        }
        auto reply = d->network_manager->deleteResource(req);

        if (mode & PRINT_REQUEST)
            LOGD("%s", qUtf8Printable(msg));

        task.reset(new HttpManagerReplyWrapper(reply, id), [](HttpManagerReplyWrapper *ptr){
            ptr->deleteLater();
        });
        task->onFinished([id, mode, url](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError)
                LOGD("error(%s):", qUtf8Printable(url)) << reply->errorString();

            if (mode & PRINT_RESPONSE) {
                auto data = reply->readAll();
                LOGD("response(%d):\n%s", id, data.data());
            }
        });
        d->task_list.insert(task->reply(), task);
    };

#ifndef Q_OS_WASM
    if (QThread::currentThread() == &d->worker_thread)
        func();
    else
        QMetaObject::invokeMethod(&d->worker_thread, func, Qt::BlockingQueuedConnection);
#else
    func();
#endif

    return task;
}

class HttpManagerReplyWrapperPrivate
{
public:
    int id = -1;
    bool abort = false;

#ifndef Q_OS_WASM
    QEventLoop *event_loop = nullptr;
#endif

    QList<HandleFinishedFunc> handle_finished_func_list;
    QList<HandleReadyReadFunc> handle_readyread_runc_list;
    QList<HandleDonwloadProgressFunc> handle_donwload_progress_func_list;
    QList<HandleUploadProgressFunc> handle_upload_progress_func_list;

    QNetworkReply *reply;
    int connect_timeout;
    int data_timeout;
    QElapsedTimer leisure_timer;
    QElapsedTimer elapsed;

    QByteArray body;

    bool finished = false;
};

HttpManagerReplyWrapper::HttpManagerReplyWrapper(QNetworkReply *reply, int id,
                                                 int connect_timeout,
                                                 int data_timeout,
                                                 QObject *parent)
    : QObject(parent)
{
    d.reset(new HttpManagerReplyWrapperPrivate);
    d->reply = reply;
    d->connect_timeout = connect_timeout;
    d->data_timeout = data_timeout;
    d->leisure_timer.start();
    d->elapsed.start();
    d->id = id;
    if (d->id == -1)
        d->id = s_max_id++;

    connect(reply, &QNetworkReply::destroyed, this, [this]()
            {
                if (d->abort)
                    return;

                d->reply = nullptr;
            }, Qt::DirectConnection);

    connect(reply, &QNetworkReply::finished, this, [this]()
            {
                if (d->abort)
                    return;

                for (auto &func: d->handle_finished_func_list)
                    func(d->reply);

#ifndef Q_OS_WASM
                if (d->event_loop)
                    d->event_loop->quit();
#endif

                d->finished = true;
                // LOGD("finished");
            }, Qt::DirectConnection);

    connect(reply, &QNetworkReply::readyRead, this, [this]()
            {
                if (d->abort)
                    return;

                for (auto &func: d->handle_readyread_runc_list)
                    func(d->reply);

                // LOGD("readyRead");
            }, Qt::DirectConnection);

    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
            {
                if (d->abort)
                    return;

                d->leisure_timer.restart();

                for (auto &func: d->handle_donwload_progress_func_list)
                    func(bytesReceived, bytesTotal);

                // LOGD("downloadProgress");
            }, Qt::DirectConnection);

    connect(reply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesSent, qint64 bytesTotal)
            {
                if (d->abort)
                    return;

                d->leisure_timer.restart();

                for (auto &func: d->handle_upload_progress_func_list)
                    func(bytesSent, bytesTotal);

                // LOGD("uploadProgress");
            }, Qt::DirectConnection);

    startTimer(1000);

    // LOGD("%d", d->id);
}

HttpManagerReplyWrapper::~HttpManagerReplyWrapper()
{
    abort();

    // LOGD("%d", d->id);
}

QNetworkReply *HttpManagerReplyWrapper::reply()
{
    return d->reply;
}

int HttpManagerReplyWrapper::connectTimeout() const
{
    return d->connect_timeout;
}

void HttpManagerReplyWrapper::setConnectTimeout(int v)
{
    d->connect_timeout = v;
}

int HttpManagerReplyWrapper::dataTimeout() const
{
    return d->data_timeout;
}

void HttpManagerReplyWrapper::setDataTimeout(int v)
{
    d->data_timeout = v;
}

QByteArray HttpManagerReplyWrapper::body() const
{
    return d->body;
}

void HttpManagerReplyWrapper::setBody(const QByteArray& v)
{
    d->body = v;
}

HttpManagerReplyWrapper *HttpManagerReplyWrapper::onFinished(HandleFinishedFunc func)
{
    if (d->finished)
        func(d->reply);
    else
        d->handle_finished_func_list.append(func);
    return this;
}

HttpManagerReplyWrapper *HttpManagerReplyWrapper::onReadyRead(HandleReadyReadFunc func)
{
    d->handle_readyread_runc_list.append(func);
    return this;
}

HttpManagerReplyWrapper *HttpManagerReplyWrapper::onDonwloadProgress(HandleDonwloadProgressFunc func)
{
    d->handle_donwload_progress_func_list.append(func);
    return this;
}

HttpManagerReplyWrapper *HttpManagerReplyWrapper::onUploadProgress(HandleUploadProgressFunc func)
{
    d->handle_upload_progress_func_list.append(func);
    return this;
}

int HttpManagerReplyWrapper::id()
{
    return d->id;
}

void HttpManagerReplyWrapper::reset()
{
    d->handle_finished_func_list.clear();
    d->handle_readyread_runc_list.clear();
    d->handle_donwload_progress_func_list.clear();
    d->handle_upload_progress_func_list.clear();
}

void HttpManagerReplyWrapper::abort()
{
    d->abort = true;

    if (d->reply) {
        // NOTE: 这里做取舍，用最简单的处理方式，abort就不触发finished回调
        //   如果调用方析构了，这里再调用就会非法内存访问
        //   不保证一定会调用finished回调，要注意资源的有序清理

        d->reply->disconnect();
        d->reply->deleteLater();
        d->reply = NULL;
    }
}

#ifndef Q_OS_WASM
void HttpManagerReplyWrapper::waitForFinished()
{
    if (d->finished)
        return;

    QEventLoop loop;
    d->event_loop = &loop;
    loop.exec();
    d->event_loop = nullptr;
}
#endif

qint64 HttpManagerReplyWrapper::elapsed() const
{
    return d->elapsed.elapsed();
}

void HttpManagerReplyWrapper::timerEvent(QTimerEvent *)
{
    if (d->finished || !d->reply)
        return;

    if (!d->reply->isOpen()) {
        if (d->connect_timeout > 0
            && d->leisure_timer.hasExpired(d->connect_timeout)) {
            LOGW("connect timeout");
            for (auto &func: d->handle_finished_func_list)
                func(d->reply);
            abort();
        }
    }
    else {
        if (!d->reply->isFinished()
            && d->data_timeout > 0
            && d->leisure_timer.hasExpired(d->data_timeout)) {
            LOGW("data timeout");
            for (auto &func: d->handle_finished_func_list)
                func(d->reply);
            abort();
        }
    }
}
