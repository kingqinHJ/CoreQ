#ifndef CAMERAHTTP_H
#define CAMERAHTTP_H

#include "CameraBase.h"
#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraHttpPrivate;
class CLDEVICE_EXPORT CameraHttp : public CameraBase
{
public:
    CameraHttp();
    ~CameraHttp();

    void inject(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm);

    void start(const QSize &v = QSize()) override;
    void stop() override;
    bool isRunning() override;

    int capture(const QString &fileName) override;

    bool previewActive() const override;
    void setPreviewActive(bool v) override;

    CameraInfo info() const override;

private:
    std::shared_ptr<CameraHttpPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERAHTTP_H
