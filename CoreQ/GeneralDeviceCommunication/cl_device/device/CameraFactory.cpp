#include "CameraFactory.h"
#include "device/CameraHttp.h"
#include "device/CameraUsb.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

CameraBasePtr CameraFactory::create(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm)
{
    switch (deviceInfo.connectionType) {
    case CT_Http: {
        auto cam = std::make_shared<CameraHttp>();
        cam->inject(deviceInfo, comm);
        return cam;
    }
    default:
        break;
    }

    return CameraBasePtr();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
CameraBasePtr CameraFactory::create(const QCameraDevice &cameraDevice)
{
    auto cam = std::make_shared<CameraUsb>();
    cam->inject(cameraDevice);
    return cam;
}
#else
CameraBasePtr CameraFactory::create(const QCameraInfo &cameraDevice)
{
    auto cam = std::make_shared<CameraUsb>();
    cam->inject(cameraDevice);
    return cam;
}
#endif

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
