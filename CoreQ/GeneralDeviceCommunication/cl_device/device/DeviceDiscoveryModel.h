#ifndef DEVICEDISCOVERYMODEL_H
#define DEVICEDISCOVERYMODEL_H

#include "qml/QmlStandardItemModel.h"
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceDiscoveryModelPrivate;
class CLDEVICE_EXPORT DeviceDiscoveryModel : public QmlStandardItemModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole+1,
        ModelRole,
        ModelNameRole,
        ConnectionTypeRole,
        ConnectionTypeNameRole,
        AddressRole,
        LaserRole,
        Description,
    };

    explicit DeviceDiscoveryModel(QObject *parent = nullptr);
    ~DeviceDiscoveryModel();

    void append(const DeviceInfo &v);
    void remove(const DeviceInfo &v);
    void clear();

    void setItemData2(QStandardItem *item, const DeviceInfo &v);
    QStandardItem *toStandardItem(const DeviceInfo &v);
    void appendItem(const DeviceInfo &v);
    void updateItem(const DeviceInfo &v);
    void removeItem(const DeviceInfo &v);

    QHash<int,QByteArray> roleNames() const override;

signals:

private:
    std::shared_ptr<DeviceDiscoveryModelPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEDISCOVERYMODEL_H
