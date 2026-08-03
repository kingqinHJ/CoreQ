#include "QmlDeviceController.h"
#include "device/DeviceManager.h"
#include "device/DeviceController.h"
#include "common/Utils.h"
#include "qml/QmlUtils.h"
#include "LaserTask.h"

#define ENSURE_HANDLE() \
    auto handle = d->handle.lock(); \
    if (!handle) \
        return;

using namespace cl::device::v1;

class QmlDeviceControllerPrivate
{
public:
    LaserTask task;
    DeviceManager *mgr = nullptr;
    DeviceControllerWPtr handle;

    PrintStatus lastPrintStatus = PrintStatus::Idle;

    DeviceType model = DM_Unknown;
    ConnectionType connectionType = CT_Unknown;
    QString address;
    QString laser;

    bool connected = false;
    int exposure = 0;

    QmlDeviceController::FireDetectLevel fireDetectLevel = QmlDeviceController::Off;
};

QmlDeviceController::QmlDeviceController(QObject *parent)
    : QObject{parent}
{
    d.reset(new QmlDeviceControllerPrivate);
}

QmlDeviceController::~QmlDeviceController()
{
    close();
    setDeviceMgr(nullptr);
    LOG_THIS();
}

QObject *QmlDeviceController::deviceMgr() const
{
    return d->mgr;
}

void QmlDeviceController::setDeviceMgr(QObject *v)
{
    if (d->mgr == v)
        return;

    if (d->mgr) {
        d->mgr->disconnect(this);
    }

    d->mgr = static_cast<DeviceManager*>(v);
    emit deviceMgrChanged();

    if (d->mgr) {
        connect(d->mgr, &DeviceManager::deviceControllerRemoved, [this](QString address){
            if (d->address == address)
                close();
        });
        connect(d->mgr, &DeviceManager::aboutToCleanup, [this](){
            close();
            setDeviceMgr(nullptr);
        });
    }
}

QObject *QmlDeviceController::task() const
{
    return &d->task;
}

bool QmlDeviceController::connected() const
{
    return d->connected;
}

void QmlDeviceController::setConnected(bool v)
{
    if (d->connected == v)
        return;

    d->connected = v;
    emit connectedChanged();
}

QVariantMap QmlDeviceController::info() const
{
    DeviceInfo info;
    DeviceCapability cap;

    if (connected()) {
        if (auto handle = d->handle.lock()) {
            info = handle->info();
            cap = info.cap;
        }
    }
    else if (d->model != DM_Unknown) {
        cap = d->mgr->getCapability(d->model);
        info.model = cap.model;
        info.modelName = d->mgr->getModelName(d->model);
        info.connectionType = d->connectionType;
        info.address = d->address;
        info.laser = d->laser.isEmpty() ? cap.defaultLaser : d->laser;
    }
    else if (d->mgr) {
        cap = d->mgr->getCapability(DM_CV40Pro);
        info.model = cap.model;
        info.modelName = d->mgr->getModelName(cap.model);
        info.laser = cap.defaultLaser;
    }

    auto data = info.toMap();
    data["presetLasers"] = cap.presetLasers;
    data["thumbnail"] = cap.thumbnail;
    data["guideImage"] = cap.guideImage;

    data["displayTimeUnit"] = cap.displayTimeUnit;

    data["speedMin"] = cap.speedMin;
    data["speedMax"] = cap.speedMax;
    data["powerMin"] = cap.powerMin;
    data["powerMax"] = cap.powerMax;

    data["supportAutoFocus"] = cap.supportAutoFocus;
    data["supportXYAxis"] = cap.supportXYAxis;
    data["supportZAxis"] = cap.supportZAxis;
    data["supportAAxis"] = cap.supportAAxis;
    data["supportBAxis"] = cap.supportBAxis;
    data["supportConveyor"] = cap.supportConveyor;
    data["supportOta"] = cap.supportOta;
    data["supportRotaryKit"] = cap.supportRotaryKit;

    // 串口通信不需要注释
    data["enableComments"] = d->connectionType != CT_Serial;

    return data;
}

QVariantMap QmlDeviceController::config() const
{
    DeviceConfig cfg;
    if (auto handle = d->handle.lock()) {
        cfg = handle->config();
    }
    else if (d->model != DM_Unknown) {
        cfg.applyDefaults(d->mgr->getCapability(d->model));
    }

    if (cfg.workAreaWidth == 0 || cfg.workAreaHeight == 0) {
        cfg.workAreaWidth = 400;
        cfg.workAreaHeight = 400;
    }

    return cfg.toMap();
}

QVariantMap QmlDeviceController::status()
{
    DeviceStatus status;
    if (auto handle = d->handle.lock())
        status = handle->status();
    return status.toMap();
}

void QmlDeviceController::onStatusChanged()
{
    if (auto handle = d->handle.lock()) {
        DeviceStatus status = handle->status();
        if (d->lastPrintStatus != status.printStatus) {
            d->lastPrintStatus = status.printStatus;

            switch (status.printStatus) {
            case cl::device::v1::PrintStatus::Idle:
                d->task.stop();
                break;
            case cl::device::v1::PrintStatus::Processing:
                if (d->task.isRunning())
                    d->task.resume();
                else
                    d->task.start();
                break;
            case cl::device::v1::PrintStatus::Paused:
                if (d->task.isRunning())
                    d->task.pause();
                else
                    d->task.start();
                break;
            default:
                break;
            }
        }
    }
}

int QmlDeviceController::borderSpeed() const
{
    if (auto handle = d->handle.lock()){
        DeviceConfig cfg = handle->config();
        return cfg.borderSpeed;
    }
    else {
        return 0;
    }
}

void QmlDeviceController::setBorderSpeed(int v)
{
    DeviceConfig cfg;
    if (auto handle = d->handle.lock()) {
        cfg = handle->config();
        if (cfg.borderSpeed == v)
            return;

        cfg.borderSpeed = v;
        handle->setConfig(cfg);
        emit configChanged();
    }
}

int QmlDeviceController::borderPower() const
{
    if (auto handle = d->handle.lock()){
        DeviceConfig cfg = handle->config();
        return cfg.borderPower;
    }
    else {
        return 0;
    }
}

void QmlDeviceController::setBorderPower(int v)
{
    if (auto handle = d->handle.lock()) {
        DeviceConfig cfg = handle->config();
        if ( cfg.borderPower == v)
            return;

        cfg.borderPower = v;
        handle->setConfig(cfg);
        emit configChanged();
    }
}

QVariantMap QmlDeviceController::connectionInfo() const
{
    QVariantMap m;
    m["model"] = d->model;
    m["connectionType"] = d->connectionType;
    m["address"] = d->address;
    return m;
}

void QmlDeviceController::setConnectionInfo(QVariantMap v)
{
    auto model = (DeviceType)v.value("model").toInt();
    if (d->model == model)
        return;

    close();

    d->model = model;
    d->connectionType = (ConnectionType)v.value("connectionType").toInt();
    d->address = v.value("address").toString();
    d->laser = QString();

    emit connectionInfoChanged();
    emit statusChanged();
    emit infoChanged();
    emit configChanged();
}

bool QmlDeviceController::canConnect() const
{
    switch (d->connectionType) {
    case CT_Serial:
    case CT_Http:
        return true;
    default:
        return !d->address.isEmpty();
    }
}

bool QmlDeviceController::isEmpty() const
{
    return d->model == DM_Unknown;
}

int QmlDeviceController::exposure() const
{
    return d->exposure;
}

void QmlDeviceController::setExposure(int v)
{
    if (d->exposure == v)
        return;

    d->exposure = v;
    emit exposureChanged();

    if (auto handle = d->handle.lock())
        handle->setExposure(v);
}

QmlDeviceController::FireDetectLevel QmlDeviceController::fireDetectLevel() const
{
    return d->fireDetectLevel;
}

void QmlDeviceController::setFireDetectLevel(FireDetectLevel v)
{
    if (d->fireDetectLevel == v)
        return;

    d->fireDetectLevel = v;
    emit fireDetectLevelChanged();

    if (auto handle = d->handle.lock())
        handle->setFireDetectLevel(::FireDetectLevel(v));
}

void QmlDeviceController::open(QString address)
{
    if (connected() && d->address == address)
        return;

    close();
    d->lastPrintStatus = cl::device::v1::PrintStatus::Idle;

    if (d->mgr)
        d->handle = d->mgr->create(address);

    bool connectState = false;
    if (auto handle = d->handle.lock()) {
        connectState = true;
        auto info = handle->info();
        d->model = info.model;
        d->connectionType = info.connectionType;
        d->address = address;
        d->laser = QString();

        handle->getExposure([this](int ecode, QVariant data){
            if (ecode == 0) {
                d->exposure = data.toInt();
                emit exposureChanged();
            }
        });
        handle->getFireDetectLevel([this](int ecode, QVariant data){
            if (ecode == 0) {
                d->fireDetectLevel = (FireDetectLevel)data.toInt();
                emit exposureChanged();
            }
        });
        connect(handle.get(), &DeviceController::disconnected, this, [this](){
            close();
        });
        connect(handle.get(), &DeviceController::deviceStatusChanged, this, [this](){
            onStatusChanged();
            emit statusChanged();
        });
    }
    else {
        setExposure(0);
        setFireDetectLevel(Off);
    }

    emit statusChanged();
    emit infoChanged();
    emit configChanged();
    emit connectionInfoChanged();

    if (connectState) {
        setConnected(true);
    }
    else {
        setConnected(false);
        emit deviceConnectFailed();
    }
}

void QmlDeviceController::tryOpen(QString address, QJSValue callback)
{
    auto callback_id = QmlUtils::storeCallback(callback);
    d->mgr->searchHttpDevice(address, [this, callback_id](QString address, bool state) {
        if (state)
            open(address);
        else
            emit deviceConnectFailed();
        QmlUtils::invokeMethod(this, callback_id, { state });
    }, this);
}

void QmlDeviceController::connectDevice()
{
    if (!canConnect())
        return;

    switch (d->connectionType) {
    case CT_Serial:
        d->mgr->connectSerialDevice(d->model, d->address, [this](QString address, bool state) {
            if (state)
                open(address);
            else
                emit deviceConnectFailed();
        }, this);
        break;
    case CT_Http: {
        d->mgr->connectHttpDevice(d->model, d->address, [this](QString address, bool state) {
            if (state)
                open(address);
            else
                emit deviceConnectFailed();
        }, this);
    }
    default:
        break;
    }
}

void QmlDeviceController::close()
{
    if (auto handle = d->handle.lock())
        handle->disconnect(this);
    if (d->mgr && !d->address.isEmpty())
        d->mgr->destroy(d->address);
    d->handle.reset();
    setConnected(false);

    emit statusChanged();
    emit infoChanged();
    emit configChanged();
}

void QmlDeviceController::updateLaser(QString laser)
{
    if (d->laser == laser)
        return;

    d->laser = laser;
    emit infoChanged();
}

void QmlDeviceController::processForPreview(QString filename, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->upload(Utils::readFromFile(filename), [this, callback_id](int ecode, QVariant data){
        bool result = ecode == 0;
        if (result) {
            if (auto handle = d->handle.lock()) {
                handle->preview([this, callback_id](int ecode, QVariant data){
                    QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
                });
                return;
            }
        }

        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, false });
    });
}

void QmlDeviceController::processForProcessing(QString gcodeFile, QString thumbnailFile, QVariantMap metadata, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->upload(Utils::readFromFile(gcodeFile), [this, gcodeFile, thumbnailFile, metadata, callback_id](int ecode, QVariant data){
        bool result = ecode == 0;
        if (result) {
            if (auto handle = d->handle.lock()) {
                d->task.setGcodeFile(gcodeFile);
                d->task.setThumbnailFile(thumbnailFile);
                d->task.setMetaData(metadata);

                handle->start([this, gcodeFile, thumbnailFile, callback_id, metadata](int ecode, QVariant data) {
                    QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool(), QString::number(ecode) });
                });
                return;
            }
        }

        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, false, QString::number(ecode) });
    });
}

void QmlDeviceController::upload(QString filename, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->upload(Utils::readFromFile(filename), [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::preview(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->preview([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::start(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->start([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::pause(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->pause([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::resume(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->resume([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::stop(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->stop([this, callback_id](int ecode, QVariant data){
        if (data.toBool() && d->task.isRunning()) {
            d->task.setErrorString("用户终止");
        }
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::home(int axis, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->home(axis, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::move(int axis, qreal distance, qreal speed, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->move(axis, distance, speed, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::sendCommand(QStringList cmds, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->sendCommand(cmds, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::moveXY(qreal x, qreal y, bool absolute, qreal speed, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->moveXY(x, y, absolute, speed, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::autoFocus(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->autoFocus([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::laserOn(int power, QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->laserOn(power, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::laserOff(QJSValue callback)
{
    ENSURE_HANDLE()
    auto callback_id = QmlUtils::storeCallback(callback);
    handle->laserOff([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toBool() });
    });
}

void QmlDeviceController::getCameraConfig(QJSValue callback)
{
    ENSURE_HANDLE()

    auto callback_id = QmlUtils::storeCallback(callback);
    handle->getCameraConfig([this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toString() });
    });
}

void QmlDeviceController::setCameraConfig(QString fileUrl, QJSValue callback)
{
    ENSURE_HANDLE()

    auto file = QUrl(fileUrl).toLocalFile();
    auto data = Utils::readFromFile(file);

    auto callback_id = QmlUtils::storeCallback(callback);
    handle->setCameraConfig(data, [this, callback_id](int ecode, QVariant data){
        QmlUtils::invokeMethod(this, callback_id, { ecode == 0, data.toString() });
    });
}

QString QmlDeviceController::getFireDetectLevelName(FireDetectLevel v)
{
    return toFireDetectLevelString(::FireDetectLevel(v));
}
