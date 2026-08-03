#include "LaserProcessingManager.h"
#include "device/DeviceManager.h"
#include "device/CameraManager.h"

using namespace cl::device::v1;

class LaserProcessingManagerPrivate
{
public:
    QString dataPath;
    std::shared_ptr<DeviceManager> deviceMgr;
    std::shared_ptr<CameraManager> cameraMgr;
};

LaserProcessingManager::LaserProcessingManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new LaserProcessingManagerPrivate);
    initialize();
}

LaserProcessingManager::~LaserProcessingManager()
{
    cleanup();
}

bool LaserProcessingManager::initialize()
{
    d->deviceMgr = std::make_shared<DeviceManager>();
    d->deviceMgr->initialize();

    d->cameraMgr = std::make_shared<CameraManager>();
    d->cameraMgr->initialize();

    connect(d->deviceMgr.get(), &DeviceManager::deviceControllerAdded, this, &LaserProcessingManager::onDeviceControllerAdded);
    connect(d->deviceMgr.get(), &DeviceManager::deviceControllerRemoved, this, &LaserProcessingManager::onDeviceControllerRemoved);

    emit deviceMgrChanged();
    emit cameraMgrChanged();

    return true;
}

void LaserProcessingManager::cleanup()
{
    d->cameraMgr->disconnect(this);
    d->deviceMgr->disconnect(this);

    d->cameraMgr.reset();
    d->deviceMgr.reset();

    emit deviceMgrChanged();
    emit cameraMgrChanged();
}

QObject *LaserProcessingManager::deviceMgr() const
{
    return d->deviceMgr.get();
}

QObject *LaserProcessingManager::cameraMgr() const
{
    return d->cameraMgr.get();
}

QString LaserProcessingManager::dataPath() const
{
    return d->dataPath;
}

void LaserProcessingManager::setDataPath(QString v)
{
    if (d->dataPath == v)
        return;

    d->dataPath = v;
    emit dataPathChanged();

    d->deviceMgr->setDataPath(v);
}

void LaserProcessingManager::onDeviceControllerAdded(QString address)
{
    auto deviceInfo = d->deviceMgr->findDeviceInfo(address);
    if (deviceInfo.isValid()) {
        d->cameraMgr->onDeviceAdded(deviceInfo, d->deviceMgr->findComm(address));
    }
}

void LaserProcessingManager::onDeviceControllerRemoved(QString address)
{
    auto deviceInfo = d->deviceMgr->findDeviceInfo(address);
    d->cameraMgr->onDeviceRemoved(deviceInfo);
}
