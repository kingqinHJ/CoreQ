#include "ProtocolHandlerHttp.h"
#include "common/Utils.h"

#include <QWebSocket>

#define ENSURE_COMM() \
    auto comm = d->comm.lock(); \
    if (!comm) \
        return;

#define EXPOSURE_SCALE  2000
#define PING_LOSS_LIMIT 10

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class ProtocolHandlerHttpPrivate
{
public:
    ProtocolHandlerHttp *q;
    int timerId = 0;

    DeviceInfo info;
    CommunicationInterfaceWPtr comm;

    DeviceStatus status;

    QString wsAddress;
    QWebSocket wsSock;
    int pingLossCount = 0;

    PrintStatus mapPrintStatus(int state, PrintStatus currStatus);

    void parseStatusMessage(const QString &msg);
    void reconnect();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
};

PrintStatus ProtocolHandlerHttpPrivate::mapPrintStatus(int state, PrintStatus currStatus)
{
    switch (state) {
    case 1:
    case 2:
        return PrintStatus::Idle;
    case 32:
        return PrintStatus::Processing;
    case 64:
        return PrintStatus::Previewing;
    case 128:
        if (currStatus == PrintStatus::Previewing)
            return PrintStatus::PreviewPaused;
        else
            return PrintStatus::Paused;
    default:
        return PrintStatus::Busy;
    }
}

void ProtocolHandlerHttpPrivate::parseStatusMessage(const QString &msg)
{
    auto jo = Utils::stringToJson(msg.toUtf8()).toObject();

    QJsonArray ja;
    if (jo.contains("modulelist"))
        ja = jo.value("modulelist").toArray();
    else
        ja.append(jo);

    LOGD() << msg;

    DeviceStatus tmpStatus = status;

    for (auto jv: ja) {
        auto jo = jv.toObject();

        if (!jo.contains("module"))
            continue;

        auto module = jo.value("module").toString();

        // 打印状态
        // {"module":"printer","curState":2}
        // {"module":"printer","progress":"98.277374"}
        if (module == "printer") {
            if (jo.contains("curState")) {
                int state = jo.value("curState").toInt(2);
                tmpStatus.printStatus = mapPrintStatus(state, tmpStatus.printStatus);
                LOGD("Printer state:") << getPrintStatusName(tmpStatus.printStatus);
            }
        }
        // 开关门状态
        // { "module": "safeDoor", "curState": 1 }
        else if (module == "safeDoor") {
            int state = jo.value("curState").toInt(0);
            tmpStatus.doorStatus = state == 1 ? DoorStatus::Open : DoorStatus::Closed;
            LOGD("Door state:") << getDoorStatusName(tmpStatus.doorStatus);
        }
        // 告警码
        // { "module": "alarm", "type": 2, "code": "01001059" }
        else if (module == "alarm") {
            tmpStatus.alarmCode = jo.value("code").toString();
        }
        // OTA升级状态
        // { "module": "ota", "status": 2, "progress": 50 }
        else if (module == "ota") {
            tmpStatus.otaStatus = static_cast<OtaStatus>(jo.value("status").toInt(0));
            tmpStatus.otaProgress = jo.value("progress").toInt(0);
            LOGD("Ota state:") << getOtaStatusName(tmpStatus.otaStatus);
        }
        // 配件轴
        // {"module":"extAxis","export0":1,"export1":-2}
        else if (module == "extAxis") {
            int export0 = jo.value("export0").toInt();
            int export1 = jo.value("export1").toInt();
            std::map<QString, int> ports {
                { "A", export0 },
                { "B", export1 },
            };
            for (auto &it: ports) {
                // 只关心插入状态，忽略其余状态
                switch (it.second) {
                case 0: // 旋转轴插入
                    if (tmpStatus.rotaryAxis != it.first) {
                        tmpStatus.rotaryAxis = it.first;
                        LOGD("%s Axis: Rotary", qUtf8Printable(it.first));
                    }
                    break;
                case 2: // 传送带插入
                    if (tmpStatus.conveyorAxis != it.first) {
                        tmpStatus.conveyorAxis = it.first;
                        LOGD("%s Axis: Converyor", qUtf8Printable(it.first));
                    }
                    break;
                case 4: // 送料机插入
                    if (tmpStatus.feederAxis != it.first) {
                        tmpStatus.feederAxis = it.first;
                        LOGD("%s Axis: Feeder", qUtf8Printable(it.first));
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (status != tmpStatus) {
        status = tmpStatus;
        q->deviceStatusChanged();
    }
}

void ProtocolHandlerHttpPrivate::reconnect()
{
    if (wsSock.state() != QAbstractSocket::UnconnectedState)
        return;

    wsSock.open(wsAddress);
    LOGD() << wsAddress << wsSock.state();
}

void ProtocolHandlerHttpPrivate::onConnected()
{
    LOGD() << wsAddress;
}

void ProtocolHandlerHttpPrivate::onDisconnected()
{
    LOGW() << wsAddress << ":" << wsSock.errorString();
    emit q->disconnected();
}

void ProtocolHandlerHttpPrivate::onErrorOccurred(QAbstractSocket::SocketError error)
{
    LOGW() << wsAddress << ":" << wsSock.errorString();
}

ProtocolHandlerHttp::ProtocolHandlerHttp()
{
    d.reset(new ProtocolHandlerHttpPrivate);
    d->q = this;

    d->wsSock.setProxy(QNetworkProxy::NoProxy);
    connect(&d->wsSock, &QWebSocket::textMessageReceived, this, [this](const QString &msg){
        d->parseStatusMessage(msg);
    }, Qt::QueuedConnection);
    connect(&d->wsSock, &QWebSocket::connected, this, [this](){
        d->onConnected();
    }, Qt::QueuedConnection);
    connect(&d->wsSock, &QWebSocket::disconnected, this, [this](){
        d->onDisconnected();
    }, Qt::QueuedConnection);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(&d->wsSock, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error){
        d->onErrorOccurred(error);
    }, Qt::QueuedConnection);
#else
    connect(&d->wsSock, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, [this](QAbstractSocket::SocketError error){
        d->onErrorOccurred(error);
    }, Qt::QueuedConnection);
#endif
    connect(&d->wsSock, &QWebSocket::pong, this, [this](quint64 elapsedTime, const QByteArray &payload) {
        // LOGD("ping elapsed:") << elapsedTime;
        d->pingLossCount = 0;
    }, Qt::QueuedConnection);

    d->timerId = startTimer(1000);
}

ProtocolHandlerHttp::~ProtocolHandlerHttp()
{
    if (auto comm = d->comm.lock())
        comm->clearAllPendingCallbacks();

    killTimer(d->timerId);
    d->wsSock.disconnect(this);
    d->wsSock.close();
    LOGD("%s", qUtf8Printable(d->info.address));
}

void ProtocolHandlerHttp::attach(CommunicationInterfaceWPtr comm, const DeviceInfo &info)
{
    if (auto _comm = comm.lock()) {
        if (_comm->type() == CT_Http) {
            d->comm = comm;
            d->info = info;

            QString ip = QUrl("http://"+_comm->address()).host();
            quint16 port = 11111;

            d->wsAddress = QString("ws://%1:%2").arg(ip).arg(port);
            d->wsSock.open(d->wsAddress);

            auto seq = _comm->get("/work/state");
            _comm->onResponse(seq, [this](const QByteArray &data){
                auto jo = Utils::stringToJson(data).toObject();
                auto ecode = jo.value("errorcode").toInt();
                auto joPayload = jo.value("payload").toObject();
                auto state = joPayload.value("state").toInt(1);
                d->status.printStatus = d->mapPrintStatus(state, d->status.printStatus);
                emit deviceStatusChanged();
            }, this);
        }
    }
}

void ProtocolHandlerHttp::upload(const QByteArray &data, ResultCallback cb)
{
    ENSURE_COMM();

    HttpHeaders headers;
    headers["content-type"] = "application/octet-stream";
    auto seq = comm->post("/work/upload?action=upload", data, headers);
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::preview(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->post("/work/preview", QByteArray());
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::start(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->post("/work/start", QByteArray());
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::pause(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->get("/work/pause");
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::resume(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->get("/work/restart");
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::stop(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->get("/work/stop");
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::home(int axis, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["axis"] = axis;
    auto seq = comm->post("/work/home", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::move(int axis, qreal distance, qreal speed, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["axis"] = axis;
    body["distance"] = distance;
    body["speed"] = speed;
    auto seq = comm->post("/work/Jog", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::sendCommand(QStringList cmds, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["cmd"] = cmds.join("\n");
    auto seq = comm->post("/work/Jog", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::moveXY(qreal x, qreal y, bool absolute, qreal speed, ResultCallback cb)
{
    ENSURE_COMM();

    QStringList gcodes;
    gcodes.append("G00 G17 G40 G54");
    gcodes.append("G21");
    gcodes.append(absolute ? "G90" : "G91");
    gcodes.append(QString("G1 X%1Y%2S0F%3").arg(x).arg(y).arg(speed));
    gcodes.append("");

    sendCommand(gcodes, cb);
}

void ProtocolHandlerHttp::autoFocus(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->post("/work/probe", QByteArray(), {}, 5000, 30000);
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::laserOn(int power, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["power"] = power;
    auto seq = comm->post("/work/fire", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::laserOff(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->post("/work/unfire", QByteArray());
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::getExposure(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->get("/media/getExposure");
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt();
        auto joPayload = jo.value("payload").toObject();
        auto value = 100*joPayload.value("exposureValue").toInt()/EXPOSURE_SCALE;
        if (cb) cb(ecode, value);
    }, this);
}

void ProtocolHandlerHttp::setExposure(int value, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["exposureValue"] = value*EXPOSURE_SCALE/100;
    auto seq = comm->post("/media/setExposure", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::getFireDetectLevel(ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->get("/media/getFireDetect");
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt();
        auto joPayload = jo.value("payload").toObject();
        auto value = joPayload.value("switch").toString();
        if (cb) cb(ecode, (int)parseFireDetectLevelString(value));
    }, this);
}

void ProtocolHandlerHttp::setFireDetectLevel(FireDetectLevel level, ResultCallback cb)
{
    ENSURE_COMM();

    QVariantMap body;
    body["switch"] = toFireDetectLevelString(level);
    auto seq = comm->post("/media/setFireDetect", Utils::jsonToString(body));
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

void ProtocolHandlerHttp::getCameraConfig(ResultCallback cb)
{
    ENSURE_COMM();

    HttpHeaders headers;
    headers["Content-Disposition"] = "cameraConfig.txt";
    auto seq = comm->get("/work/download", headers);
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        if (jo.contains("errorcode")) {
            auto ecode = jo.value("errorcode").toInt();
            if (cb) cb(ecode, QVariant());
        }
        else {
            if (cb) cb(0, data);
        }
    }, this);
}

void ProtocolHandlerHttp::setCameraConfig(QByteArray data, ResultCallback cb)
{
    ENSURE_COMM();

    auto seq = comm->post("/work/upload?action=camera", data);
    comm->onResponse(seq, [this, cb](const QByteArray &data){
        auto jo = Utils::stringToJson(data).toObject();
        auto ecode = jo.value("errorcode").toInt(-1);
        auto joPayload = jo.value("payload").toObject();
        auto result = joPayload.value("result").toInt(-1);
        if (cb) cb(ecode, result == 0);
    }, this);
}

DeviceStatus ProtocolHandlerHttp::status() const
{
    return d->status;
}

void ProtocolHandlerHttp::timerEvent(QTimerEvent *e)
{
    d->reconnect();

    if (d->wsSock.state() == QAbstractSocket::ConnectedState) {
        if (d->pingLossCount < PING_LOSS_LIMIT) {
            d->pingLossCount++;
            d->wsSock.ping("?");
        }
        else {
            LOGW("Ping loss exceeded limit");
            emit disconnected();
        }
    }
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
