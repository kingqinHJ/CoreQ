#ifndef CAMERAUSB_H
#define CAMERAUSB_H

#include "CameraBase.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QCameraDevice>
#else
#include <QCameraInfo>
#endif

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraUsbPrivate;
class CLDEVICE_EXPORT CameraUsb : public CameraBase
{
public:
    CameraUsb();
    ~CameraUsb();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void inject(const QCameraDevice &cameraDevice);
#else
    void inject(const QCameraInfo &cameraDevice);
#endif

    void start(const QSize &v = QSize()) override;
    void stop() override;
    bool isRunning() override;

    int capture(const QString &fileName) override;

    bool previewActive() const override;
    void setPreviewActive(bool v) override;

    CameraInfo info() const override;

private:
    std::shared_ptr<CameraUsbPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERAUSB_H
