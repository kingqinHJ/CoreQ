#include "DeviceDiscoveryModel.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceDiscoveryModelPrivate
{
public:
    QMap<QString, DeviceInfo> deviceMap;
    QStringList deviceList;

    QMap<QString, QStandardItem*> items;
};

DeviceDiscoveryModel::DeviceDiscoveryModel(QObject *parent)
    : QmlStandardItemModel{parent}
{
    d.reset(new DeviceDiscoveryModelPrivate);
}

DeviceDiscoveryModel::~DeviceDiscoveryModel()
{

}

void DeviceDiscoveryModel::append(const DeviceInfo &v)
{
    auto id = v.id();
    d->deviceMap.insert(id, v);

    if (!d->deviceList.contains(id)) {
        d->deviceList.append(id);
        appendItem(v);
    }
    else {
        updateItem(v);
    }
}

void DeviceDiscoveryModel::remove(const DeviceInfo &v)
{
    removeItem(v);
    d->deviceMap.remove(v.id());
    d->deviceList.removeAll(v.id());
}

void DeviceDiscoveryModel::clear()
{
    d->deviceMap.clear();
    d->deviceList.clear();
    d->items.clear();
    QmlStandardItemModel::clear();
}

void DeviceDiscoveryModel::setItemData2(QStandardItem *item, const DeviceInfo &v)
{
    if (!item) return;
    item->setData(v.id(), IdRole);
    item->setData(v.model, ModelRole);
    item->setData(v.modelName, ModelNameRole);
    item->setData(v.connectionType, ConnectionTypeRole);
    item->setData(getConnectionTypeName(v.connectionType), ConnectionTypeNameRole);
    item->setData(v.address, AddressRole);
    if (!v.laser.isEmpty())
        item->setData(v.laser, LaserRole);
    else
        item->setData(v.cap.defaultLaser, LaserRole);
    item->setData(QString("%1 / %2").arg(v.laserType, v.fieldLens), Description);
}

QStandardItem *DeviceDiscoveryModel::toStandardItem(const DeviceInfo &v)
{
    QStandardItem *item = new QStandardItem;
    setItemData2(item, v);
    return item;
}

void DeviceDiscoveryModel::appendItem(const DeviceInfo &v)
{
    auto item = toStandardItem(v);
    d->items[v.id()] = item;
    appendRow(item);
}

void DeviceDiscoveryModel::updateItem(const DeviceInfo &v)
{
    auto id = v.id();

    if (d->items.contains(id))
        setItemData2(d->items[id], v);
}

void DeviceDiscoveryModel::removeItem(const DeviceInfo &v)
{
    auto id = v.id();
    for (int i=0; i<rowCount(); i++) {
        auto *pitem = item(i);
        if (pitem->data(IdRole).toString() == id) {
            removeRow(i);
            d->items.remove(id);
            break;
        }
    }
}

QHash<int, QByteArray> DeviceDiscoveryModel::roleNames() const
{
    auto role_names = QStandardItemModel::roleNames();
    role_names.insert(IdRole, "id");
    role_names.insert(ModelRole, "deviceModel");
    role_names.insert(ModelNameRole, "modelName");
    role_names.insert(ConnectionTypeRole, "connectionType");
    role_names.insert(ConnectionTypeNameRole, "connectionTypeName");
    role_names.insert(AddressRole, "address");
    role_names.insert(LaserRole, "laser");
    role_names.insert(Description, "description");
    return role_names;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
