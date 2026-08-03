#include "DeviceConfigManager.h"
#include "common/Utils.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

static const int saveConfigDelay = 500;
static const char *saveConfigFilename = "deviceconfig.json";

class DeviceConfigManagerPrivate
{
public:
    QString dataPath;
    QMap<QString, DeviceConfig> configs;

    int timerId = 0;

    void load();
    void store();
};

void DeviceConfigManagerPrivate::load()
{
    configs.clear();

    if (dataPath.isEmpty())
        return;

    QString fileName = QString("%1/%2").arg(dataPath).arg(saveConfigFilename);
    auto contents = Utils::readFromFile(fileName);
    auto ja = Utils::stringToJson(contents).toArray();
    for (auto jv: ja) {
        auto joConfig = jv.toObject();
        auto config = DeviceConfig::fromJson(joConfig);
        if (!config.deviceId.isEmpty())
            configs.insert(config.deviceId, config);
    }

    LOGD("load count:") << configs.size();
}

void DeviceConfigManagerPrivate::store()
{
    QJsonArray ja;
    for (auto &cfg: configs)
        ja.append(cfg.toJson());

    QString fileName = QString("%1/%2").arg(dataPath).arg(saveConfigFilename);
    auto contents = Utils::jsonToString(ja);
    Utils::writeToFile(fileName, contents);

    LOGD("save count:") << configs.size();
}

DeviceConfigManager::DeviceConfigManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new DeviceConfigManagerPrivate);
}

DeviceConfigManager::~DeviceConfigManager()
{

}

QString DeviceConfigManager::dataPath() const
{
    return d->dataPath;
}

void DeviceConfigManager::setDataPath(QString v)
{
    d->dataPath = v;
    d->load();
}

DeviceConfig DeviceConfigManager::getConfig(const QString &deviceId) const
{
    return d->configs.value(deviceId);
}

void DeviceConfigManager::updateConfig(const DeviceConfig &config)
{
    if (config.deviceId.isEmpty())
        return;

    d->configs[config.deviceId] = config;

    if (d->timerId != 0)
        killTimer(d->timerId);
    d->timerId = startTimer(saveConfigDelay);
}

void DeviceConfigManager::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->timerId) {
        killTimer(d->timerId);
        d->timerId = 0;

        d->store();
    }
    QObject::timerEvent(e);
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
