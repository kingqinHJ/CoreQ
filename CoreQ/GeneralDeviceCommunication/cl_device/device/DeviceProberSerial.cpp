#include "DeviceProberSerial.h"

#define COMBINE_VID_PID(vid, pid) (((static_cast<uint32_t>(vid) & 0xFFFF) << 16) | (static_cast<uint32_t>(pid) & 0xFFFF))

// 30 30Pro 40 50 50Pro 50S
#define DEVICE1_COMBINE_ID  COMBINE_VID_PID(0x303A, 0x4001)
// 40Pro 60（该类设备不支持串口）
#define DEVICE2_COMBINE_ID  COMBINE_VID_PID(0x1D6B, 0x0104)

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

static const int retryCount = 3;
static const char *infoCmd = "$I\n";
static const char *propertyCmd = "$$\n";
static const char *initCmd = "?\n";

class DeviceProberSerialPrivate
{
public:
    QMap<QString, CommunicationInterfaceWPtr> cacheList;
    QMap<QString, DeviceIdentifier> idList;
    QMap<QString, int> retryList;
};

DeviceProberSerial::DeviceProberSerial()
{
    d.reset(new DeviceProberSerialPrivate);
}

DeviceProberSerial::~DeviceProberSerial()
{

}

void DeviceProberSerial::probe(CommunicationInterfaceWPtr comm, const DeviceIdentifier &id)
{
    QString key = id.address;
    if (d->cacheList.contains(key)) {
        LOGW("skip probe: %s", qUtf8Printable(key));
        return;
    }

    if (DEVICE1_COMBINE_ID != COMBINE_VID_PID(id.vid, id.pid)) {
        emit probeFailed(id);
        return;
    }

    if (auto _comm = comm.lock()) {
        auto seq = _comm->send(infoCmd);
        _comm->onResponse(seq, [this, key](const QByteArray &data) {
            onResponseInfoCmd(key, data);
        }, this);

        d->cacheList.insert(key, comm);
        d->idList.insert(key, id);
        d->retryList.insert(key, 1);
    }
    else {
        emit probeFailed(id);
    }
}

void DeviceProberSerial::flush(QString address)
{
    d->cacheList.remove(address);
}

void DeviceProberSerial::onResponseInfoCmd(const QString &key, const QByteArray &data)
{
    static const QMap<QString, DeviceType> patternList {
        { "CV-30", DM_CV30 },
        { "CV30-Pro", DM_CV30Pro },
        { "CV40-MASTER", DM_CV40 },
        { "CV40mini-MASTER", DM_CV40C },
        { "CV40PRO-MASTER", DM_CV40Pro },

        { "CV50-MASTER", DM_CV50 },
        { "CV50-Pro-MASTER", DM_CV50Pro },
        { "CV50-S-MASTER", DM_CV50ProS },
        { "CV40Pro Laser Master", DM_CV40Pro }
    };

    auto id = d->idList[key];
    DeviceType model = DM_Unknown;
    auto _comm = d->cacheList[key];
    auto comm = _comm.lock();
    if (!comm) {
        emit probeFailed(id);
        d->cacheList.remove(key);
        d->idList.remove(key);
        d->retryList.remove(key);
        return;
    }

    if (!data.isEmpty()) {
        QString str = data;
        for (auto it=patternList.begin(); it!=patternList.end(); ++it) {
            if (str.contains(it.key())) {
                model = it.value();
                break;
            }
        }
    }

    if (model == DM_Unknown) {
        auto retryIndex = d->retryList[key];
        if (retryIndex < retryCount) {
            auto seq = comm->send(infoCmd);
            comm->onResponse(seq, [this, key](const QByteArray &data) {
                onResponseInfoCmd(key, data);
            }, this);

            d->retryList.insert(key, retryIndex+1);
            return;
        }

        // 未读到任何数据，判定为非Grbl激光设备
        if (data.isEmpty()) {
            emit probeFailed(id);
            d->cacheList.remove(key);
            d->idList.remove(key);
            d->retryList.remove(key);
            return;
        }
    }

    if (model == DM_CV50Pro) {
        auto seq = comm->send(propertyCmd);
        comm->onResponse(seq, [this, key](const QByteArray &data) {
            onResponsePropertyCmd(key, data);
        }, this);
        return;
    }
    else if (model == DM_Unknown) {
        auto seq = comm->send(initCmd);
        comm->onResponse(seq, [this, key](const QByteArray &data) {
            onResponseInitCmd(key, data);
        }, this);
        return;
    }
    else {
        DeviceInfo info;
        info.identifier = id;
        info.model = model;
        info.connectionType = CT_Serial;
        info.address = comm->address();
        emit probeSucceeded(info);

        d->cacheList.remove(key);
        d->idList.remove(key);
        d->retryList.remove(key);
    }
}

void DeviceProberSerial::onResponsePropertyCmd(const QString &key, const QByteArray &data)
{
    // TODO: check DM_CV50Pro OR DM_CV50Pro60
    auto id = d->idList[key];
    auto _comm = d->cacheList[key];
    auto comm = _comm.lock();
    if (!comm) {
        emit probeFailed(id);
        d->cacheList.remove(key);
        d->idList.remove(key);
        d->retryList.remove(key);
        return;
    }

    DeviceInfo info;
    info.identifier = id;
    info.model = DM_CV50Pro;
    info.connectionType = CT_Serial;
    info.address = comm->address();
    emit probeSucceeded(info);

    d->cacheList.remove(key);
    d->idList.remove(key);
    d->retryList.remove(key);
}

void DeviceProberSerial::onResponseInitCmd(const QString &key, const QByteArray &data)
{
    auto id = d->idList[key];
    auto _comm = d->cacheList[key];
    auto comm = _comm.lock();
    if (!comm) {
        emit probeFailed(id);
        d->cacheList.remove(key);
        d->idList.remove(key);
        d->retryList.remove(key);
        return;
    }

    if (data.contains("Idle") || data.contains("Run") || data.contains("Hold")) {
        DeviceInfo info;
        info.identifier = id;
        info.model = DM_CV30;
        info.connectionType = CT_Serial;
        info.address = comm->address();
        emit probeSucceeded(info);
    }
    else {
        emit probeFailed(id);
    }

    d->cacheList.remove(key);
    d->idList.remove(key);
    d->retryList.remove(key);
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
