#include "CameraManager.h"
#include "CameraModel.h"
#include "device/CameraFactory.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#include <QCameraDevice>
#else
#include <QCameraInfo>
#endif

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraManagerPrivate
{
public:
    std::shared_ptr<CameraModel> cameraModel;

    QMap<QString, CameraBasePtr> cameras;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMap<QByteArray, QCameraDevice> usbCameraMap;
#else
    QMap<QString, QCameraInfo> usbCameraMap;
#endif
};

CameraManager::CameraManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new CameraManagerPrivate);
    startTimer(1000);
}

CameraManager::~CameraManager()
{
    cleanup();
}

bool CameraManager::initialize()
{
    d->cameraModel = std::make_shared<CameraModel>();

    emit cameraModelChanged();

    return true;
}

void CameraManager::cleanup()
{
    emit aboutToCleanup();

    d->cameraModel.reset();
}

QObject *CameraManager::cameraModel() const
{
    return d->cameraModel.get();
}

void CameraManager::append(QString cameraId, CameraBasePtr v)
{
    if (!v)
        return;

    d->cameras.insert(cameraId, v);
    d->cameraModel->append(v->info());
    LOGD("camera added: %s", qUtf8Printable(v->info().displayName));
}

void CameraManager::remove(QString cameraId)
{
    if (d->cameras.contains(cameraId)) {
        auto v = d->cameras.value(cameraId);
        LOGD("camera removed: %s", qUtf8Printable(v->info().displayName));
        d->cameras.remove(cameraId);
        d->cameraModel->remove(v->info());
    }
}

CameraBaseWPtr CameraManager::find(QString cameraId) const
{
    return d->cameras.value(cameraId);
}

void CameraManager::onDeviceAdded(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm)
{
    if (auto cam = CameraFactory::create(deviceInfo, comm))
        append(deviceInfo.id(), cam);
}

void CameraManager::onDeviceRemoved(const DeviceInfo &deviceInfo)
{
    remove(deviceInfo.id());
}

void CameraManager::timerEvent(QTimerEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto cameras = QMediaDevices::videoInputs();
    QMap<QByteArray, QCameraDevice> currCameraMap;
    for (auto &it: cameras) {
        currCameraMap.insert(it.id(), it);
        if (d->usbCameraMap.contains(it.id()))
            continue;

        if (auto cam = CameraFactory::create(it))
            append(it.id(), cam);
    }

    for (auto &it: d->usbCameraMap) {
        if (currCameraMap.contains(it.id()))
            continue;

        remove(it.id());
    }
    d->usbCameraMap = currCameraMap;
#else
    auto cameras = QCameraInfo::availableCameras();
    QMap<QString, QCameraInfo> currCameraMap;
    for (auto &it: cameras) {
        currCameraMap.insert(it.deviceName(), it);
        if (d->usbCameraMap.contains(it.deviceName()))
            continue;

        if (auto cam = CameraFactory::create(it))
            append(it.deviceName(), cam);
    }

    for (auto &it: d->usbCameraMap) {
        if (currCameraMap.contains(it.deviceName()))
            continue;

        remove(it.deviceName());
    }
    d->usbCameraMap = currCameraMap;
#endif
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
