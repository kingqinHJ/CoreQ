#include "CommunicationSerial.h"

#include <QSerialPort>
#include <QTimer>

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

static const int resTimeout = 1000;
static const int dataTimeout = 100;

struct ResponseInfo
{
    int seq = 0;
    QObject *receiver = nullptr;
    ResponseCallback callback;
};

class CommunicationSerialPrivate
{
public:
    int seq = 1;

    QString portName;
    std::shared_ptr<QSerialPort> io;
    std::shared_ptr<QThread> thread;

    bool hasError = false;

    QList<ResponseInfo> resList;
    QElapsedTimer resTimer;     // 响应的超时/1000ms
    QElapsedTimer dataTimer;    // 数据流的超时/100ms

    std::mutex mutex;
    QByteArray buffer;
    ResponseFilter filter;
};

CommunicationSerial::CommunicationSerial()
{
    d.reset(new CommunicationSerialPrivate);
}

CommunicationSerial::~CommunicationSerial()
{
    CommunicationSerial::close();
    LOGD("%s", qUtf8Printable(d->portName));
}

QString CommunicationSerial::address() const
{
    return d->portName;
}

void CommunicationSerial::setAddress(const QString &address)
{
    d->portName = address;
}

void CommunicationSerial::open()
{
    if (d->thread.get())
        return;

    d->thread = std::make_shared<QThread>();
    d->thread->start();
    d->thread->moveToThread(d->thread.get());
    QMetaObject::invokeMethod(d->thread.get(), [this](){
        d->io = std::make_shared<QSerialPort>();
        d->io->setPortName(d->portName);
        d->io->setParity(QSerialPort::NoParity);
        d->io->setDataBits(QSerialPort::Data8);
        d->io->setFlowControl(QSerialPort::NoFlowControl);
        d->io->setStopBits(QSerialPort::OneStop);
        d->io->setBaudRate(QSerialPort::Baud115200);
        if (!d->io->open(QIODevice::ReadWrite)) {
            LOGW("open error(%s): %s", qUtf8Printable(d->portName), qUtf8Printable(d->io->errorString()));
            d->hasError = true;
            emit communicationError();
            return;
        }

        d->io->setDataTerminalReady(true);
        connect(d->io.get(), &QSerialPort::readyRead, [this](){ onReadyRead(); });
        connect(d->io.get(), &QSerialPort::bytesWritten, [this](qint64 bytes){ onBytesWritten(bytes); });
        connect(d->io.get(), &QSerialPort::errorOccurred, [this](){ onErrorOccurred(); });
    });

    return;
}

void CommunicationSerial::close()
{
    if (d->thread) {
        QMetaObject::invokeMethod(d->thread.get(), [this](){
            if (d->io) {
                d->io->disconnect(this);
                d->io->close();
                d->io.reset();
            }

            d->thread->quit();
        });

        d->thread->wait();
        d->thread.reset();
    }
}

int CommunicationSerial::send(const QByteArray &data)
{
    QMetaObject::invokeMethod(d->thread.get(), [=](){
        if (!d->hasError)
            d->io->write(data);
    });
    return d->seq++;
}

bool CommunicationSerial::receive(QByteArray &data)
{
    if (d->thread) {
        d->mutex.lock();
        data = d->buffer;
        d->buffer.clear();
        d->mutex.unlock();
        return true;
    }
    else {
        return false;
    }
}

void CommunicationSerial::onResponse(int seq, ResponseCallback callback, QObject *receiver)
{
    if (!callback || !d->thread)
        return;

    d->resList.append({seq, receiver, callback});
    d->resTimer.restart();
    d->dataTimer.restart();

    if (d->hasError)
        invokeCallback();
}

void CommunicationSerial::clearAllPendingCallbacks()
{
    d->resList.clear();
    d->resTimer.invalidate();
    d->dataTimer.invalidate();
}

void CommunicationSerial::setResponseFilter(ResponseFilter filter)
{
    d->mutex.lock();
    d->filter = filter;
    d->mutex.unlock();
}

void CommunicationSerial::timerEvent(QTimerEvent *e)
{
    if (d->resTimer.hasExpired(resTimeout)
        || (!d->resTimer.isValid() && d->dataTimer.hasExpired(dataTimeout))) {

        invokeCallback();
    }
}

void CommunicationSerial::onReadyRead()
{
    d->mutex.lock();
    while (d->io->canReadLine()){
        auto line = d->io->readLine();
        if (d->filter) d->filter(line);
        LOGD() << line.trimmed();
        d->buffer.append(line);
    }
    d->mutex.unlock();

    QMetaObject::invokeMethod(this, [this](){
        d->resTimer.invalidate();
        d->dataTimer.restart();
    });
}

void CommunicationSerial::onBytesWritten(qint64 bytes)
{
    // LOGD() << bytes;
}

void CommunicationSerial::onErrorOccurred()
{
    if (d->io->error() == QSerialPort::ResourceError) {
        emit communicationError();
    }
}

void CommunicationSerial::invokeCallback()
{
    d->mutex.lock();
    QByteArray buffer = d->buffer;
    d->buffer.clear();
    d->mutex.unlock();

    auto tmpResList = d->resList;
    d->resList.clear();

    for (auto &it: tmpResList) {
        if (it.receiver) {
            QMetaObject::invokeMethod(it.receiver, [cb = it.callback, buffer](){
                cb(buffer);
            });
        }
        else {
            it.callback(buffer);
        }
    }

    d->resTimer.invalidate();
    d->dataTimer.invalidate();
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
