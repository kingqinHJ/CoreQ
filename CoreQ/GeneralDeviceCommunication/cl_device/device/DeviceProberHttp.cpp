#include "DeviceProberHttp.h"
#include "common/Utils.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

static const char *BasicInfoApi = "/system/basicInfo";
static const char *LaserInfoApi = "/work/getLayerType";

/*
 * FIBER 光纤激光器
 * DIODE 半导体二极管激光器
 * MOPA(Master Oscillator Power Amplifier) 带MOPA结构的光纤激光器
 * SOLID 固体激光器
 *
 * 频率（pulse frequency）：激光每秒发射的脉冲次数，单位通常是 kHz（千赫）
 *   低频（1 – 30 kHz）	每个脉冲能量高，间隔时间长	打得深、熔蚀强、容易变色或烧焦
 *   中频（30 – 100 kHz）	能量与效率平衡	常用于普通    金属打标或切割
 *   高频（>100 kHz）	    单脉冲能量低、重叠密度高	表面光滑、精细、但深度浅
 *
 * 脉冲宽度（pulse duration）：每个激光脉冲持续的时间，单位常为 ns（纳秒） 或 µs（微秒）
 *   短脉冲（<100 ns）	能量集中、热扩散小	    边缘锐利、表面光滑、热变色少
 *   长脉冲（>200 ns）	能量分散、热作用明显	易产生熔融、氧化、颜色变化
 *
 * 想“打得深”，用低频长脉冲；
 * 想“打得细、表面光”，用高频短脉冲。
*/
static QString formatLaserType(QString info)
{
    static const QMap<QString, QString> m {
        { "RED-2W-IR", "2W IR" },
        { "BLUE-20W-UNKONW", "20W" },
        { "RED-20W-FIBER", "20W Fiber" },
        { "PURPLE-5W-SOLID", "5W UV" },
        { "BLUE-20W-DIODE", "20W Diode" },
        { "BLUE-40W-DIODE", "40W Diode" },
        { "MOPA-60W-MOPA", "60W MOPA" },
    };
    return m.value(info.toUpper(), info);
}

class DeviceProberHttpPrivate
{
public:
    QMap<QString, CommunicationInterfaceWPtr> cacheList;
    QMap<QString, DeviceIdentifier> idList;
};

DeviceProberHttp::DeviceProberHttp()
{
    d.reset(new DeviceProberHttpPrivate);
}

DeviceProberHttp::~DeviceProberHttp()
{

}

void DeviceProberHttp::probe(CommunicationInterfaceWPtr comm, const DeviceIdentifier &id)
{
    QString key = id.address;
    if (d->cacheList.contains(key)) {
        LOGW("skip probe: %s", qUtf8Printable(key));
        return;
    }

    if (auto _comm = comm.lock()) {
        auto seq = _comm->get(BasicInfoApi);
        _comm->onResponse(seq, [this, key](const QByteArray &data) {
            onResponse(key, data);
        }, this);

        d->cacheList.insert(key, comm);
        d->idList.insert(key, id);
    }
    else {
        emit probeFailed(id);
    }
}

void DeviceProberHttp::flush(QString address)
{
    d->cacheList.remove(address);
}

/*
{
    "errorcode": 0,
    "payload": {
        "sn": "wthgehfre",
        "dn": "37562FFFFFFF125F19018040ProF98",
        "model": "Creality Falcon A1 Pro",
        "deviceSize": "567*468*211mm",
        "deviceWeight": "16.8kg",
        "workAreaSize": "358*268mm",
        "firmwareVersion": "1.0.28",
        "mac": "64:2b:6d:69:bb:e3",
        "infIp": "172.23.212.50",
        "publicIp": "",
        "laserType": "BLUE-20W-UNKONW",
        "laserClass": "0",
        "zAxisVersion": ""
    }
}
*/
void DeviceProberHttp::onResponse(const QString &key, const QByteArray &data)
{
    static const QMap<QString, DeviceType> patternList {
        { "Creality Falcon A1 Pro", DM_CV40Pro },
        { "Creality Falcon A1 mini", DM_CV40C },
        { "Creality Falcon A1C", DM_CV40C },
        { "Creality Falcon T1", DM_CV60 },
    };

    DeviceType model = DM_Unknown;
    auto id = d->idList[key];
    auto _comm = d->cacheList[key];
    auto comm = _comm.lock();
    if (!comm) {
        emit probeFailed(id);
        d->cacheList.remove(key);
        d->idList.remove(key);
        return;
    }

    QJsonObject jo;
    if (!data.isEmpty()) {
        jo = Utils::stringToJson(data).toObject().value("payload").toObject();
        model = patternList.value(jo.value("model").toString(), DM_Unknown);
    }

    if (model != DM_Unknown) {
        DeviceInfo info;
        info.identifier = id;
        info.model = model;
        info.sn = jo.value("sn").toString();
        info.dn = jo.value("dn").toString();
        info.firmwareVersion = jo.value("firmwareVersion").toString();
        info.connectionType = CT_Http;
        info.address = comm->address();
        info.laser = formatLaserType(jo.value("laserType").toString());

        auto seq = comm->get(LaserInfoApi);
        comm->onResponse(seq, [this, key, info](const QByteArray &data) mutable {
            auto jo = Utils::stringToJson(data).toObject().value("payload").toObject();

            info.laserType = jo.value("laserType").toString();
            info.laserSn = jo.value("laserSn").toString();
            info.fieldLens = jo.value("fieldLensStr").toString();
            emit probeSucceeded(info);
        }, this);
    }
    else {
        emit probeFailed(id);
    }

    d->cacheList.remove(key);
    d->idList.remove(key);
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
