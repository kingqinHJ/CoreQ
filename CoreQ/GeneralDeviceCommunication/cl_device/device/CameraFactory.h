#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H

#include "CameraBase.h"
#include "CommunicationInterface.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QCameraDevice>
#else
#include <QCameraInfo>
#endif

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CLDEVICE_EXPORT CameraFactory
{
public:
    static CameraBasePtr create(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static CameraBasePtr create(const QCameraDevice &cameraDevice);
#else
    static CameraBasePtr create(const QCameraInfo &cameraDevice);
#endif
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERAFACTORY_H
