#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include "qml/QmlStandardItemModel.h"
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraModelPrivate;
class CLDEVICE_EXPORT CameraModel : public QmlStandardItemModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole+1,
        DisplayNameRole,
    };

    explicit CameraModel(QObject *parent = nullptr);
    ~CameraModel();

    void append(const CameraInfo &v);
    void remove(const CameraInfo &v);
    void clear();

    void setItemData2(QStandardItem *item, const CameraInfo &v);
    QStandardItem *toStandardItem(const CameraInfo &v);
    void appendItem(const CameraInfo &v);
    void updateItem(const CameraInfo &v);
    void removeItem(const CameraInfo &v);

    QHash<int,QByteArray> roleNames() const override;

signals:

private:
    std::shared_ptr<CameraModelPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERAMODEL_H
