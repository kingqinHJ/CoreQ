#include "DeviceCatalogModel.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceCatalogModelPrivate
{
public:
    QList<DeviceCapability> capabilityList;
};

DeviceCatalogModel::DeviceCatalogModel(QObject *parent)
    : QAbstractListModel{parent}
{
    d.reset(new DeviceCatalogModelPrivate);
}

DeviceCatalogModel::~DeviceCatalogModel()
{
}

void DeviceCatalogModel::setCapabilityList(const QList<DeviceCapability> &list)
{
    beginResetModel();
    d->capabilityList = QList<DeviceCapability>(list.rbegin(), list.rend());
    endResetModel();
}

int DeviceCatalogModel::rowCount(const QModelIndex &parent) const
{
    return d->capabilityList.count();
}

QVariant DeviceCatalogModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!(0 <= row && row < rowCount()))
        return QVariant();

    auto &cap = d->capabilityList[row];

    switch (role) {
    case Qt::UserRole+1: return cap.model;
    case Qt::UserRole+2: return cap.displayName;
    case Qt::UserRole+3: return cap.thumbnail;
    case Qt::UserRole+4: return cap.workAreaWidth;
    case Qt::UserRole+5: return cap.workAreaHeight;

    case Qt::UserRole+6: return cap.defaultLaser;
    case Qt::UserRole+7: return cap.presetLasers;

    case Qt::UserRole+8: return cap.defaultConnectionType;
    }

    return QVariant();
}

bool DeviceCatalogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

QHash<int, QByteArray> DeviceCatalogModel::roleNames() const
{
    static const QHash<int, QByteArray> m {
        { Qt::UserRole+1, "modelType" },
        { Qt::UserRole+2, "displayName" },
        { Qt::UserRole+3, "thumbnail" },
        { Qt::UserRole+4, "workAreaWidth" },
        { Qt::UserRole+5, "workAreaHeight" },

        { Qt::UserRole+6, "defaultLaser" },
        { Qt::UserRole+7, "presetLasers" },

        { Qt::UserRole+8, "defaultConnectionType" },
    };
    return m;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
