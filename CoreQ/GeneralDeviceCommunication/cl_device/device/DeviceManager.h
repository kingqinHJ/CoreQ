#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "DeviceDef.h"
#include "CommunicationInterface.h"
#include "DeviceController.h"
#include <QObject>

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceManagerPrivate;
class CLDEVICE_EXPORT DeviceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceCatalogModel READ deviceCatalogModel NOTIFY deviceCatalogModelChanged FINAL)
    Q_PROPERTY(QObject* deviceDiscoveryModel READ deviceDiscoveryModel NOTIFY deviceDiscoveryModelChanged FINAL)

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    bool initialize();
    void cleanup();

    QString dataPath() const;
    void setDataPath(QString v);

    QObject* deviceCatalogModel() const;
    QObject* deviceDiscoveryModel() const;

    DeviceCapability getCapability(DeviceType v) const;
    QString getModelName(DeviceType v) const;

    DeviceConfig getConfig(const QString &deviceId) const;
    void updateConfig(const DeviceConfig &config);

    void startDiscovery(DiscoveryType type);
    void stopDiscovery(DiscoveryType type);
    void toogleDiscovery(DiscoveryType type);

    DeviceControllerWPtr create(QString address);
    void destroy(QString address);

    CommunicationInterfaceWPtr findComm(QString address);
    DeviceInfo findDeviceInfo(QString address);

    void searchHttpDevice(QString address, ProbeFinishedCallback cb, QObject *receiver);
    void connectHttpDevice(DeviceType type, QString address, ProbeFinishedCallback cb, QObject *receiver);
    void connectSerialDevice(DeviceType type, QString address, ProbeFinishedCallback cb, QObject *receiver);

public slots:
    void toogleUdpDiscovery();
    void toogleSerialDiscovery();

    void startUdpDiscovery();
    void stopUdpDiscovery();

    void startSerialDiscovery();
    void stopSerialDiscovery();

    void appendHttpDevice(QString address);
    void appendSerialDevice(QString address);

signals:
    void deviceCatalogModelChanged();
    void deviceDiscoveryModelChanged();

    void aboutToCleanup();

    void deviceControllerAdded(QString address);
    void deviceControllerRemoved(QString address);

private:
    void onDeviceDiscovered(const DeviceIdentifier &id);
    void onDeviceRemoved(const DeviceIdentifier &id);
    void onProbeSucceeded(const DeviceInfo &info);
    void onProbeFailed(const DeviceIdentifier &id);

private:
    std::shared_ptr<DeviceManagerPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEMANAGER_H
