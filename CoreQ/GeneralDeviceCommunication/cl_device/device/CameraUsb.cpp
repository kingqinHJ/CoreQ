#include "CameraUsb.h"

#include <QCamera>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QVideoSink>
#else
#include <QCameraImageCapture>
#include <QVideoProbe>
#endif

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraUsbPrivate
{
public:
    CameraUsb *q;
    CameraInfo info;
    bool running = false;
    bool previewActive = false;

    QSize size;

    std::shared_ptr<QCamera> camera;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QCameraDevice cameraDevice;
    std::shared_ptr<QMediaCaptureSession> session;
    std::shared_ptr<QImageCapture> imageCapture;
    std::shared_ptr<QVideoSink> videoSink;
#else
    QCameraInfo cameraDevice;
    std::shared_ptr<QCameraImageCapture> imageCapture;
    std::shared_ptr<QVideoProbe> videoProbe;
#endif
};

CameraUsb::CameraUsb()
{
    d.reset(new CameraUsbPrivate);
    d->q = this;
}

CameraUsb::~CameraUsb()
{

}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CameraUsb::inject(const QCameraDevice &cameraDevice)
{
    d->cameraDevice = cameraDevice;
    d->info.cameraId = cameraDevice.id();
    d->info.displayName = cameraDevice.description();
}
#else
void CameraUsb::inject(const QCameraInfo &cameraDevice)
{
    d->cameraDevice = cameraDevice;
    d->info.cameraId = cameraDevice.deviceName();
    d->info.displayName = cameraDevice.description();
}
#endif

void CameraUsb::start(const QSize &v)
{
    if (d->running)
        return;

    d->size = v;
    d->running = true;
    d->camera = std::make_shared<QCamera>(d->cameraDevice);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    d->session = std::make_shared<QMediaCaptureSession>();
    d->session->setCamera(d->camera.get());
    d->imageCapture = std::make_shared<QImageCapture>();
    d->session->setImageCapture(d->imageCapture.get());
    d->videoSink = std::make_shared<QVideoSink>();

    connect(d->camera.get(), &QCamera::activeChanged, [this](bool active){
        LOGD("activeChanged") << active;
    });
    connect(d->camera.get(), &QCamera::errorChanged, [this](){
        LOGD("errorChanged") << d->camera->errorString();
    });
    connect(d->imageCapture.get(), &QImageCapture::errorOccurred, [](int id, QImageCapture::Error error, const QString &errorString){
        LOGD("errorOccurred") << id << error << errorString;
    });
    connect(d->imageCapture.get(), &QImageCapture::imageSaved, this, &CameraUsb::imageSaved);
    connect(d->videoSink.get(), &QVideoSink::videoFrameChanged, [this](const QVideoFrame &frame){
        // LOGD() << frame.size() << frame.pixelFormat();
        emit videoFrameProbed(frame);
    });
#else
    d->imageCapture = std::make_shared<QCameraImageCapture>(d->camera.get());
    d->camera->setCaptureMode(QCamera::CaptureVideo);
    d->videoProbe = std::make_shared<QVideoProbe>();
    d->videoProbe->setSource(d->camera.get());

    connect(d->camera.get(), &QCamera::statusChanged, [this](QCamera::Status status){
        LOGD("QCamera::statusChanged") << status;
    });
    connect(d->camera.get(), &QCamera::errorOccurred, [this](QCamera::Error error){
        LOGD("QCamera::errorOccurred") << d->camera->errorString();
    });
    connect(d->imageCapture.get(), qOverload<int, QCameraImageCapture::Error, const QString &>(&QCameraImageCapture::error),
            [this](int id, QCameraImageCapture::Error error, const QString &errorString){
        LOGD("QCameraImageCapture::error") << id << error << errorString;
    });
    connect(d->imageCapture.get(), &QCameraImageCapture::imageSaved, this, &CameraUsb::imageSaved);
    connect(d->videoProbe.get(), &QVideoProbe::videoFrameProbed, [this](const QVideoFrame &frame){
        if (d->previewActive) {
            LOGD() << frame.size() << frame.pixelFormat();
            emit videoFrameProbed(QVideoFrame(frame.image()));
        }
    });
#endif

    setPreviewActive(d->previewActive);
    LOGD("%s", qUtf8Printable(d->cameraDevice.description()));

    d->camera->start();
}

void CameraUsb::stop()
{
    if (!d->running)
        return;

    d->camera->stop();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    d->imageCapture.reset();
    d->videoSink.reset();
    d->session.reset();
    LOGD("%s", qUtf8Printable(d->cameraDevice.description()));
#else
    d->imageCapture.reset();
    d->videoProbe.reset();
#endif
    d->camera.reset();

    d->running = false;
}

bool CameraUsb::isRunning()
{
    return d->running;
}

int CameraUsb::capture(const QString &fileName)
{
    int id = -1;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (d->imageCapture)
        id = d->imageCapture->captureToFile(fileName);
#else
    if (d->imageCapture)
        id = d->imageCapture->capture(fileName);
#endif
    return id;
}

bool CameraUsb::previewActive() const
{
    return d->previewActive;
}

void CameraUsb::setPreviewActive(bool v)
{
    d->previewActive = v;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (d->previewActive) {
        d->session->setVideoSink(d->videoSink.get());
    }
    else {
        d->session->setVideoSink(nullptr);
    }
#endif
}

CameraInfo CameraUsb::info() const
{
    return d->info;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
