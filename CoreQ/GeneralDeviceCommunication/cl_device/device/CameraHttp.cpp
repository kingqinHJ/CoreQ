#include "CameraHttp.h"

#include <QFileInfo>

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CameraHttpPrivate
{
public:
    CameraHttp *q;
    CameraInfo info;
    bool running = false;
    bool previewActive = false;

    bool busy = false;

    QSize size;

    CommunicationInterfaceWPtr comm;
    DeviceInfo deviceInfo;

    int imageId = 0;
    QMap<int, QString> ids;

    int capturePhoto(QString fileName = QString());
};

int CameraHttpPrivate::capturePhoto(QString fileName)
{
    if (fileName.isEmpty() && !previewActive) {
        LOGW("fileName is empty");
        return -1;
    }

    if (!fileName.isEmpty())
        fileName = QFileInfo(fileName).absoluteFilePath();

    if (auto ptr = comm.lock()) {
        auto tmpSize = size;
        if (!tmpSize.isValid())
            tmpSize = toSize(info.defaultResolution);
        if (!tmpSize.isValid()) {
            LOGW("no default size");
            return -1;
        }

        int id = imageId++;
        if (!fileName.isEmpty())
            ids.insert(id, fileName);
        if (busy)
            return id;

        busy = true;
        QString body = QString(R"({ "width": %1, "height": %2, "exposureValue": -1 })")
                           .arg(tmpSize.width()).arg(tmpSize.height());
        int req = ptr->post("/media/getCapturePhoto", body.toUtf8());
        ptr->onResponse(req, [this](const QByteArray &body){

            QImage img;
            img.loadFromData(body);
            if (img.isNull()) {
                LOGW("image is null: %d\n", body.size()) << body.data();
            }
            else {
                // CV60的相机需要翻转才是正视图
                if (deviceInfo.model == DM_CV60)
                    img = img.mirrored(true, true);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0) || QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                if (previewActive)
                    emit q->videoFrameProbed(QVideoFrame(img));
#endif
            }

            QSet<QString> savedFileNames;
            for (auto it=ids.begin(); it!=ids.end(); it++) {

                if (img.isNull()) {
                    emit q->imageSaved(it.key(), QString());
                    continue;
                }

                if (savedFileNames.contains(it.value())) {
                    emit q->imageSaved(it.key(), it.value());
                    continue;
                }

                if (img.save(it.value())) {
                    savedFileNames.insert(it.value());
                    emit q->imageSaved(it.key(), it.value());
                }
                else {
                    LOGW("save error: %d", qUtf8Printable(it.value()));
                    emit q->imageSaved(it.key(), QString());
                }
            }
            ids.clear();

            busy = false;

            // 实时更新
            if (previewActive) {
                QMetaObject::invokeMethod(q, [this](){
                    capturePhoto();
                });
            }

        }, q);
        return id;
    }
    else {
        LOGW("comm instance is invalid");
        return -1;
    }
}

CameraHttp::CameraHttp()
{
    d.reset(new CameraHttpPrivate);
    d->q = this;
}

CameraHttp::~CameraHttp()
{

}

void CameraHttp::inject(const DeviceInfo &deviceInfo, CommunicationInterfaceWPtr comm)
{
    d->comm = comm;
    d->deviceInfo = deviceInfo;

    d->info.cameraId = deviceInfo.id();
    d->info.displayName = QString("Creality Falcon Camera(%1)").arg(deviceInfo.address);
    switch (d->deviceInfo.model) {
    case DM_CV40Pro:
        // 1300W : 4160,3120
        // 800W : 3280,2460
        // d->info.defaultResolution = "4160x3120";
        d->info.defaultResolution = "3280x2460";
        break;
    case DM_CV60:
        d->info.defaultResolution = "2560x1920";
        break;
    case DM_CV40C:
        d->info.defaultResolution = "1600x1200";
        break;
    default:
        d->info.defaultResolution = "3280x2460";
        break;
    }
}

void CameraHttp::start(const QSize &v)
{
    if (d->running)
        return;

    d->size = v;
    d->running = true;

    if (d->previewActive)
        d->capturePhoto();

    LOGD("%s", qUtf8Printable(d->deviceInfo.address));
}

void CameraHttp::stop()
{
    if (!d->running)
        return;

    d->running = false;
    LOGD("%s", qUtf8Printable(d->deviceInfo.address));
}

bool CameraHttp::isRunning()
{
    return d->running;
}

int CameraHttp::capture(const QString &fileName)
{
    return d->capturePhoto(fileName);
}

bool CameraHttp::previewActive() const
{
    return d->previewActive;
}

void CameraHttp::setPreviewActive(bool v)
{
    d->previewActive = v;
    if (d->previewActive)
        d->capturePhoto();
}

CameraInfo CameraHttp::info() const
{
    return d->info;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
