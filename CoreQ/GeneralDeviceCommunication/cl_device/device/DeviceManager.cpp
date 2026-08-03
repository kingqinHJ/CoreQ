#include "DeviceManager.h"
#include "DeviceCapabilityManager.h"
#include "CommunicationManager.h"
#include "DeviceDiscoverySerial.h"
#include "DeviceDiscoveryUdp.h"
#include "DeviceProberSerial.h"
#include "DeviceProberHttp.h"
#include "DeviceConfigManager.h"

#include "DeviceCatalogModel.h"
#include "DeviceDiscoveryModel.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceManagerPrivate
{
public:
    QString dataPath;

    std::shared_ptr<DeviceCapabilityManager> capMgr;
    std::shared_ptr<DeviceConfigManager> configMgr;
    std::shared_ptr<CommunicationManager> commMgr;

    std::shared_ptr<DeviceCatalogModel> catalogModel;
    std::shared_ptr<DeviceDiscoveryModel> discoveryModel;

    QMap<DiscoveryType, DeviceDiscoveryPtr> discoveryList;
    QMap<ConnectionType, DeviceProberPtr> proberList;

    // <..., <address>>
    QMap<ConnectionType, QSet<QString>> historyDevice;
    // <address, ...>
    QMap<QString, DeviceInfo> probedDeviceMap;
    // <address, ...>
    QMap<QString, DeviceControllerPtr> controllerMap;

    QMap<QString, QPair<ProbeFinishedCallback, QPointer<QObject>>> probeCallback;
};

DeviceManager::DeviceManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new DeviceManagerPrivate);
}

DeviceManager::~DeviceManager()
{
    cleanup();
}

bool DeviceManager::initialize()
{
    d->capMgr = std::make_shared<DeviceCapabilityManager>();
    d->configMgr = std::make_shared<DeviceConfigManager>();
    d->commMgr = std::make_shared<CommunicationManager>();
    d->catalogModel = std::make_shared<DeviceCatalogModel>();
    d->catalogModel->setCapabilityList(d->capMgr->getCapabilityList());
    d->discoveryModel = std::make_shared<DeviceDiscoveryModel>();

    d->discoveryList.insert(DT_Udp, std::make_shared<DeviceDiscoveryUdp>());
    d->discoveryList.insert(DT_Serial, std::make_shared<DeviceDiscoverySerial>());
    for (auto &it: d->discoveryList) {
        connect(it.get(), &DeviceDiscovery::deviceDiscovered, this, &DeviceManager::onDeviceDiscovered);
        connect(it.get(), &DeviceDiscovery::deviceRemoved, this, &DeviceManager::onDeviceRemoved);
    }

    d->proberList.insert(CT_Serial, std::make_shared<DeviceProberSerial>());
    d->proberList.insert(CT_Http, std::make_shared<DeviceProberHttp>());
    for (auto &it: d->proberList) {
        connect(it.get(), &DeviceProber::probeSucceeded, this, &DeviceManager::onProbeSucceeded);
        connect(it.get(), &DeviceProber::probeFailed, this, &DeviceManager::onProbeFailed);
    }

    emit deviceDiscoveryModelChanged();
    LOG_THIS();
    return true;
}

void DeviceManager::cleanup()
{
    emit aboutToCleanup();

    d->controllerMap.clear();
    d->probedDeviceMap.clear();
    d->historyDevice.clear();

    d->proberList.clear();

    for (auto &it: d->discoveryList)
        it->disconnect(this);
    d->discoveryList.clear();

    d->discoveryModel.reset();
    d->catalogModel.reset();
    d->commMgr.reset();
    d->configMgr.reset();
    d->capMgr.reset();

    emit deviceDiscoveryModelChanged();

    LOG_THIS();
}

QString DeviceManager::dataPath() const
{
    return d->dataPath;
}

void DeviceManager::setDataPath(QString v)
{
    d->dataPath = v;
    d->configMgr->setDataPath(v);
}

QObject *DeviceManager::deviceCatalogModel() const
{
    return d->catalogModel.get();
}

QObject *DeviceManager::deviceDiscoveryModel() const
{
    return d->discoveryModel.get();
}

DeviceCapability DeviceManager::getCapability(DeviceType v) const
{
    return d->capMgr->getCapability(v);
}

QString DeviceManager::getModelName(DeviceType v) const
{
    return d->capMgr->getModelName(v);
}

DeviceConfig DeviceManager::getConfig(const QString &deviceId) const
{
    return d->configMgr->getConfig(deviceId);
}

void DeviceManager::updateConfig(const DeviceConfig &config)
{
    d->configMgr->updateConfig(config);
}

void DeviceManager::startDiscovery(DiscoveryType type)
{
    if (d->discoveryList.contains(type)) {
        switch (type) {
        case DT_Udp:
            d->historyDevice[CT_Http].clear();
            break;
        case DT_Serial:
            d->historyDevice[CT_Serial].clear();
            break;
        default:
            break;
        }
        d->discoveryList[type]->start();
    }
}

void DeviceManager::stopDiscovery(DiscoveryType type)
{
    if (d->discoveryList.contains(type)) {
        d->discoveryList[type]->stop();
    }
}

void DeviceManager::toogleDiscovery(DiscoveryType type)
{
    if (d->discoveryList.contains(type)) {
        if (d->discoveryList[type]->isRunning()) {
            stopDiscovery(type);
        }
        else {
            startDiscovery(type);
        }
    }
}

DeviceControllerWPtr DeviceManager::create(QString address)
{
    if (!d->probedDeviceMap.contains(address) || d->controllerMap.contains(address))
        return DeviceControllerWPtr();

    auto device = std::make_shared<DeviceController>(d->probedDeviceMap[address], d->commMgr.get());
    connect(device.get(), &DeviceController::deviceConfigChanged, this, [this, address](){
        if (d->controllerMap.contains(address))
            updateConfig(d->controllerMap[address]->config());
    });

    d->controllerMap.insert(address, device);
    emit deviceControllerAdded(address);
    return device;
}

void DeviceManager::destroy(QString address)
{
    if (d->controllerMap.contains(address)) {
        auto device = d->controllerMap.take(address);
        device->disconnect(this);
        emit deviceControllerRemoved(address);
    }
}

CommunicationInterfaceWPtr DeviceManager::findComm(QString address)
{
    return d->commMgr->find(address);
}

DeviceInfo DeviceManager::findDeviceInfo(QString address)
{
    if (d->probedDeviceMap.contains(address))
        return d->probedDeviceMap[address];
    else
        return DeviceInfo();
}

void DeviceManager::searchHttpDevice(QString address, ProbeFinishedCallback cb, QObject *receiver)
{
    if (!address.contains(":"))
        address.append(":8080");

    d->historyDevice[CT_Http].remove(address);
    d->proberList[CT_Http]->flush(address);

    DeviceIdentifier id;
    id.address = address;
    id.connectionType = CT_Http;
    id.transportType = TT_WiFi;
    d->probeCallback.insert(address, {cb, QPointer<QObject>(receiver)});
    onDeviceDiscovered(id);
}

void DeviceManager::connectHttpDevice(DeviceType type, QString address, ProbeFinishedCallback cb, QObject *receiver)
{
    auto devices = d->historyDevice[CT_Http];
    for (auto &it: devices) {
        auto info = d->probedDeviceMap.value(it);
        if (info.model == type) {
            if (address.isEmpty()) {
                address = it;
                break;
            }
            else if (address == it) {
                break;
            }
        }
    }

    if (cb) {
        cb(address, !address.isEmpty());
    }
}

void DeviceManager::connectSerialDevice(DeviceType type, QString address, ProbeFinishedCallback cb, QObject *receiver)
{
    auto devices = d->historyDevice[CT_Serial];
    for (auto &it: devices) {
        auto info = d->probedDeviceMap.value(it);
        if (info.model == type) {
            if (address.isEmpty()) {
                address = it;
                break;
            }
            else if (address == it) {
                break;
            }
        }
    }

    if (cb) {
        cb(address, !address.isEmpty());
    }
}

void DeviceManager::toogleUdpDiscovery()
{
    toogleDiscovery(DT_Udp);
}

void DeviceManager::toogleSerialDiscovery()
{
    toogleDiscovery(DT_Serial);
}

void DeviceManager::startUdpDiscovery()
{
    startDiscovery(DT_Udp);
}

void DeviceManager::stopUdpDiscovery()
{
    stopDiscovery(DT_Udp);
}

void DeviceManager::startSerialDiscovery()
{
    startDiscovery(DT_Serial);
}

void DeviceManager::stopSerialDiscovery()
{
    stopDiscovery(DT_Serial);
}

void DeviceManager::appendHttpDevice(QString address)
{
    if (!address.contains(":"))
        address.append(":8080");

    d->historyDevice[CT_Http].remove(address);

    DeviceIdentifier id;
    id.address = address;
    id.connectionType = CT_Http;
    id.transportType = TT_WiFi;
    onDeviceDiscovered(id);
}

void DeviceManager::appendSerialDevice(QString address)
{
    d->historyDevice[CT_Serial].remove(address);

    DeviceIdentifier id;
    id.address = address;
    id.connectionType = CT_Serial;
    id.transportType = TT_USB;
    onDeviceDiscovered(id);
}

void DeviceManager::onDeviceDiscovered(const DeviceIdentifier &id)
{
    if (d->historyDevice[id.connectionType].contains(id.address))
        return;

    d->historyDevice[id.connectionType].insert(id.address);
    LOGI("%s", qUtf8Printable(id.address));

    CommunicationInterfaceWPtr comm = d->commMgr->create(id.address, id.connectionType);
    if (comm.lock()) {
        if (d->proberList.contains(id.connectionType)) {
            d->proberList[id.connectionType]->probe(comm, id);
        }
        else {
            LOGD("unknown prober type: %d", id.connectionType);
        }
    }
    else {
        LOGD("comm invalid");
    }
}

void DeviceManager::onDeviceRemoved(const DeviceIdentifier &id)
{
    if (d->probedDeviceMap.contains(id.address)) {
        d->discoveryModel->remove(d->probedDeviceMap[id.address]);
        d->probedDeviceMap.remove(id.address);
    }

    d->historyDevice[id.connectionType].remove(id.address);

    d->commMgr->destroy(id.address, false);
    destroy(id.address);
    LOGW("%s", qUtf8Printable(id.address));
}

void DeviceManager::onProbeSucceeded(const DeviceInfo &info)
{
    auto _info = info;
    _info.cap = d->capMgr->getCapability(info.model);
    _info.cfg = d->configMgr->getConfig(_info.id());
    _info.modelName = _info.cap.displayName;
    _info.connectionType = info.identifier.connectionType;
    _info.transportType = info.identifier.transportType;
    if (_info.laser.isEmpty())
        _info.laser = _info.cap.defaultLaser;
    d->probedDeviceMap.insert(info.address, _info);
    d->discoveryModel->append(_info);

    d->commMgr->destroy(info.address, false);
    LOGI("%s: %s", qUtf8Printable(info.address), qUtf8Printable(_info.modelName));

    // 探测结果回调
    if (d->probeCallback.contains(info.address)) {
        auto pair = d->probeCallback.take(info.address);
        if (pair.second.isNull())
            return;

        QMetaObject::invokeMethod(pair.second.data(), [pair, info](){
            pair.first(info.address, true);
        });
    }
}

void DeviceManager::onProbeFailed(const DeviceIdentifier &id)
{
    if (d->probedDeviceMap.contains(id.address)) {
        d->discoveryModel->remove(d->probedDeviceMap[id.address]);
        d->probedDeviceMap.remove(id.address);
    }

    d->commMgr->destroy(id.address, false);
    LOGW("%s", qUtf8Printable(id.address));

    // 探测结果回调
    if (d->probeCallback.contains(id.address)) {
        auto pair = d->probeCallback.take(id.address);
        if (pair.second.isNull())
            return;

        QMetaObject::invokeMethod(pair.second.data(), [pair, id](){
            pair.first(id.address, false);
        });
    }
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
