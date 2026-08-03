#ifndef DEVICECATALOGMODEL_H
#define DEVICECATALOGMODEL_H

#include <QStandardItemModel>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceCatalogModelPrivate;
class CLDEVICE_EXPORT DeviceCatalogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DeviceCatalogModel(QObject *parent = nullptr);
    ~DeviceCatalogModel();

    void setCapabilityList(const QList<DeviceCapability> &list);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

signals:

private:
    std::shared_ptr<DeviceCatalogModelPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICECATALOGMODEL_H
