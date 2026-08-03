#ifndef QMLCAMERACONTROLLER_H
#define QMLCAMERACONTROLLER_H

#include <QObject>
#include <QJSValue>

class QmlCameraControllerPrivate;
class CLDEVICE_EXPORT QmlCameraController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* cameraMgr READ cameraMgr WRITE setCameraMgr NOTIFY cameraMgrChanged FINAL)
    Q_PROPERTY(QObject* cameraModel READ cameraModel NOTIFY cameraModelChanged FINAL)
    Q_PROPERTY(QString cameraId READ cameraId WRITE setCameraId NOTIFY cameraIdChanged FINAL)
    Q_PROPERTY(QString preferredCameraId READ preferredCameraId NOTIFY preferredCameraIdChanged FINAL)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged FINAL)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)

    Q_PROPERTY(bool previewActive READ previewActive WRITE setPreviewActive NOTIFY previewActiveChanged FINAL)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_PROPERTY(QObject* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged FINAL)
#else
    Q_PROPERTY(QObject* videoSurface READ videoSurface WRITE setVideoSurface NOTIFY videoSurfaceChanged FINAL)
#endif

public:
    explicit QmlCameraController(QObject *parent = nullptr);
    ~QmlCameraController();

    QObject *cameraMgr() const;
    void setCameraMgr(QObject *v);

    QObject *cameraModel() const;

    QString cameraId() const;
    void setCameraId(QString v);

    QString preferredCameraId() const;

    QString deviceId() const;
    void setDeviceId(QString v);

    bool active() const;
    void setActive(bool v);

    bool previewActive() const;
    void setPreviewActive(bool v);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject *videoSink() const;
    void setVideoSink(QObject *v);
#else
    QObject *videoSurface() const;
    void setVideoSurface(QObject *v);
#endif

public slots:
    int capture(QString fileName = QString(), QJSValue callback = QJSValue());

signals:
    void cameraMgrChanged();
    void cameraModelChanged();
    void preferredCameraIdChanged();
    void cameraIdChanged();
    void deviceIdChanged();
    void activeChanged();

    void previewActiveChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void videoSinkChanged();
#else
    void videoSurfaceChanged();
#endif

    void imageSaved(int id, const QString &fileName);

private:
    std::shared_ptr<QmlCameraControllerPrivate> d;
};

#endif // QMLCAMERACONTROLLER_H
