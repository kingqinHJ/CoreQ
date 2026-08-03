#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include "CameraBase.h"
#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraManagerPrivate;
class CLDEVICE_EXPORT CameraManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* cameraModel READ cameraModel NOTIFY cameraModelChanged FINAL)
public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    bool initialize();
    void cleanup();

    QObject *cameraModel() const;

    void append(QString cameraId, CameraBasePtr v);
    void remove(QString cameraId);
    CameraBaseWPtr find(QString cameraId) const;

    void onDeviceAdded(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm);
    void onDeviceRemoved(const DeviceInfo &deviceInfo);

protected:
    void timerEvent(QTimerEvent *e) override;

signals:
    void cameraModelChanged();
    void cameraAdded(QString cameraId);
    void cameraRemoved(QString cameraId);
    void aboutToCleanup();

private:
    std::shared_ptr<CameraManagerPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERAMANAGER_H
