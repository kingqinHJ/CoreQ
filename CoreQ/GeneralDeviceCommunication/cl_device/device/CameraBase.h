#ifndef CAMERABASE_H
#define CAMERABASE_H

#include <QObject>
#include <QVideoFrame>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CLDEVICE_EXPORT CameraBase : public QObject
{
    Q_OBJECT
public:
    virtual void start(const QSize &v = QSize()) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;

    virtual int capture(const QString &fileName) = 0;

    virtual bool previewActive() const = 0;
    virtual void setPreviewActive(bool v) = 0;

    virtual CameraInfo info() const = 0;

signals:
    void cameraInfoChanged();
    void runningChanged();
    void imageSaved(int id, const QString &fileName);
    void videoFrameProbed(const QVideoFrame &frame);
};

using CameraBasePtr = std::shared_ptr<CameraBase>;
using CameraBaseWPtr = std::weak_ptr<CameraBase>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // CAMERABASE_H
