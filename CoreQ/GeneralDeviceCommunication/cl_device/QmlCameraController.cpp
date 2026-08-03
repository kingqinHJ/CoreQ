#include "QmlCameraController.h"
#include "device/CameraManager.h"
#include "device/CameraBase.h"
#include "device/CameraModel.h"
#include "qml/QmlSortFilterProxyModel.h"
#include "qml/QmlUtils.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QVideoSink>
#else
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#endif

#define ENSURE_HANDLE() \
    auto handle = d->handle.lock(); \
    if (!handle) \
        return;

using namespace cl::device::v1;

class CameraModelProxy : public QmlSortFilterProxyModel
{
public:
    CameraModelProxy(QObject *parent = nullptr)
        : QmlSortFilterProxyModel(parent)
    {
        componentComplete();
    }

    QmlCameraControllerPrivate *d = nullptr;
protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};

class QmlCameraControllerPrivate
{
public:
    QmlCameraController *q;
    CameraManager *mgr = nullptr;
    CameraBaseWPtr handle;

    // <task_id, callback_id>
    QMap<int, int> callbackList;

    bool active = false;
    bool previewActive = false;

    QString cameraId;
    QString deviceId;

    CameraModelProxy cameraModel;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QVideoSink *videoSink = nullptr;
#else
    QAbstractVideoSurface *videoSurface = nullptr;
    QVideoSurfaceFormat videoFormat;
#endif

    void onCameraChanged();
};

void QmlCameraControllerPrivate::onCameraChanged()
{
    if (auto cam = handle.lock()) {
        if (active) {
            cam->start();
            cam->setPreviewActive(previewActive);
        }
        else {
            cam->stop();
        }
    }
}

bool CameraModelProxy::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    if (d->deviceId.isEmpty())
        return true;

    auto leftCameraId = sourceLeft.data(CameraModel::IdRole).toString();
    auto rightCameraId = sourceRight.data(CameraModel::IdRole).toString();
    if (d->deviceId == leftCameraId)
        return true;

    if (d->deviceId == rightCameraId)
        return false;

    return true;
}

QmlCameraController::QmlCameraController(QObject *parent)
    : QObject{parent}
{
    d.reset(new QmlCameraControllerPrivate);
    d->q = this;
    d->cameraModel.d = d.get();
}

QmlCameraController::~QmlCameraController()
{
    setCameraId(QString());
}

QObject *QmlCameraController::cameraMgr() const
{
    return d->mgr;
}

void QmlCameraController::setCameraMgr(QObject *v)
{
    if (d->mgr == v)
        return;

    if (d->mgr) {
        d->mgr->disconnect(this);
    }

    d->mgr = static_cast<CameraManager*>(v);
    emit cameraMgrChanged();

    if (d->mgr) {
        d->cameraModel.setSource(d->mgr->cameraModel());
        connect(d->mgr, &CameraManager::cameraRemoved, [this](QString cameraId){
            if (d->cameraId == cameraId) {
                setCameraId(QString());
            }
        });
        connect(d->mgr, &CameraManager::aboutToCleanup, [this](){
            setCameraId(QString());
            setCameraMgr(NULL);
        });
    }
    else {
        d->cameraModel.setSource(NULL);
    }
}

QObject *QmlCameraController::cameraModel() const
{
    return &d->cameraModel;
}

QString QmlCameraController::cameraId() const
{
    return d->cameraId;
}

void QmlCameraController::setCameraId(QString v)
{
    if (d->cameraId == v)
        return;

    if (auto cam = d->handle.lock()) {
        cam->disconnect(this);
        cam->stop();
    }

    d->cameraId = v;

    d->handle = d->mgr->find(v);
    if (auto cam = d->handle.lock()) {
        QObject::connect(cam.get(), &CameraBase::imageSaved, this, [this](int id, const QString &fileName) {
            if (d->callbackList.contains(id)) {
                auto callback_id = d->callbackList.take(id);
                QmlUtils::invokeMethod(this, callback_id, {id, fileName});
            }

            emit imageSaved(id, fileName);
        }, Qt::QueuedConnection);

        QObject::connect(cam.get(), &CameraBase::videoFrameProbed, this, [this](const QVideoFrame &frame){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (d->videoSink)
                d->videoSink->setVideoFrame(frame);
#else
            if (d->videoSurface) {
                if (d->videoSurface->isActive() && d->videoFormat.frameSize() != frame.size()) {
                    d->videoSurface->stop();
                }

                if (!d->videoSurface->isActive()) {

                    d->videoFormat = QVideoSurfaceFormat(frame.size(), frame.pixelFormat());
                    if (d->videoSurface && !d->videoSurface->isFormatSupported(d->videoFormat)) {
                        LOGD("Unsupported videoformat") << d->videoFormat;
                        d->videoFormat = QVideoSurfaceFormat();
                    }

                    if (d->videoFormat.isValid()) {
                        if (!d->videoSurface->start(d->videoFormat))
                            d->videoFormat = QVideoSurfaceFormat();
                        LOGD("render size: %dx%d", frame.size().width(), frame.size().height());
                    }
                    else if (!d->videoFormat.isValid()) {
                        LOGW("invalid format");
                    }
                }

                if (d->videoSurface->isActive())
                    d->videoSurface->present(frame);
                else
                    LOGW("skip frame") << frame.size();
            }
#endif
        });
    }

    emit cameraIdChanged();

    d->onCameraChanged();
}

QString QmlCameraController::preferredCameraId() const
{
    return d->deviceId;
}

QString QmlCameraController::deviceId() const
{
    return d->deviceId;
}

void QmlCameraController::setDeviceId(QString v)
{
    if (d->deviceId == v)
        return;

    d->deviceId = v;
    emit deviceIdChanged();
    emit preferredCameraIdChanged();

    d->cameraModel.sort(0);
}

bool QmlCameraController::active() const
{
    return d->active;
}

void QmlCameraController::setActive(bool v)
{
    if (d->active == v)
        return;

    d->active = v;
    emit activeChanged();

    d->onCameraChanged();
}

bool QmlCameraController::previewActive() const
{
    return d->previewActive;
}

void QmlCameraController::setPreviewActive(bool v)
{
    if (d->previewActive == v)
        return;

    d->previewActive = v;
    emit previewActiveChanged();

    d->onCameraChanged();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QObject *QmlCameraController::videoSink() const
{
    return d->videoSink;
}

void QmlCameraController::setVideoSink(QObject *v)
{
    if (d->videoSink == v)
        return;

    d->videoSink = static_cast<QVideoSink*>(v);
    emit videoSinkChanged();
}
#else
QObject *QmlCameraController::videoSurface() const
{
    return d->videoSurface;
}

void QmlCameraController::setVideoSurface(QObject *v)
{
    if (d->videoSurface == v)
        return;

    if (d->videoSurface)
        d->videoSurface->stop();

    d->videoSurface = qobject_cast<QAbstractVideoSurface*>(v);
    emit videoSurfaceChanged();
}
#endif

int QmlCameraController::capture(QString fileName, QJSValue callback)
{
    int id = -1;
    if (auto handle = d->handle.lock()) {
        id = handle->capture(fileName);
        if (id >= 0) {
            if (callback.isCallable()) {
                d->callbackList.insert(id, QmlUtils::storeCallback(callback));
            }
        }
        else {
            QmlUtils::invokeMethod(this, callback, {id, QString()});
        }
    }
    else {
        QmlUtils::invokeMethod(this, callback, {id, QString()});
    }

    return -1;
}
