#include "CameraModel.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraModelPrivate
{
public:
    QMap<QString, CameraInfo> cameraMap;
    QStringList cameraList;

    QMap<QString, QStandardItem*> items;
};

CameraModel::CameraModel(QObject *parent)
    : QmlStandardItemModel{parent}
{
    d.reset(new CameraModelPrivate);

    // CameraInfo ci;
    // ci.displayName = "空相机";
    // append(ci);
}

CameraModel::~CameraModel()
{

}

void CameraModel::append(const CameraInfo &v)
{
    auto id = v.cameraId;
    d->cameraMap.insert(id, v);

    if (!d->cameraList.contains(id)) {
        d->cameraList.append(id);
        appendItem(v);
    }
    else {
        updateItem(v);
    }
}

void CameraModel::remove(const CameraInfo &v)
{
    removeItem(v);
    d->cameraMap.remove(v.cameraId);
    d->cameraList.removeAll(v.cameraId);
}

void CameraModel::clear()
{
    d->cameraMap.clear();
    d->cameraList.clear();
    d->items.clear();
    QmlStandardItemModel::clear();
}

void CameraModel::setItemData2(QStandardItem *item, const CameraInfo &v)
{
    if (!item) return;
    item->setData(v.cameraId, IdRole);
    item->setData(v.displayName, DisplayNameRole);
}

QStandardItem *CameraModel::toStandardItem(const CameraInfo &v)
{
    QStandardItem *item = new QStandardItem;
    setItemData2(item, v);
    return item;
}

void CameraModel::appendItem(const CameraInfo &v)
{
    auto item = toStandardItem(v);
    d->items[v.cameraId] = item;
    appendRow(item);
}

void CameraModel::updateItem(const CameraInfo &v)
{
    auto id = v.cameraId;

    if (d->items.contains(id))
        setItemData2(d->items[id], v);
}

void CameraModel::removeItem(const CameraInfo &v)
{
    auto id = v.cameraId;
    for (int i=0; i<rowCount(); i++) {
        auto *pitem = item(i);
        if (pitem->data(IdRole).toString() == id) {
            removeRow(i);
            d->items.remove(id);
            break;
        }
    }
}

QHash<int, QByteArray> CameraModel::roleNames() const
{
    auto role_names = QStandardItemModel::roleNames();
    role_names.insert(IdRole, "id");
    role_names.insert(DisplayNameRole, "displayName");
    return role_names;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
