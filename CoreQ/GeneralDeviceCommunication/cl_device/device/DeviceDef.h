#ifndef DEVICE_H
#define DEVICE_H

#include <QtCore>
#include "DeviceGlobal.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

enum DiscoveryType {
    DT_Serial,
    DT_Udp,
};

enum ConnectionType {
    CT_Unknown,
    CT_Serial,
    CT_Http,
};

inline QString getConnectionTypeName(ConnectionType v) {
    static const QMap<ConnectionType, QString> map {
        { CT_Serial, "Serial" },
        { CT_Http, "Http" },
    };
    return map.value(v, QStringLiteral("Unknown"));
}

enum TransportType {
    TT_Unknown = 0,
    TT_USB,
    TT_WiFi,
};

inline QString getTransportTypeName(TransportType v) {
    static const QMap<TransportType, QString> map {
        { TT_USB, "USB" },
        { TT_WiFi, "WiFi" },
    };
    return map.value(v, QStringLiteral("Unknown"));
}

enum TimeUnit {
    TU_Second,          // s
    TU_Minute,          // min(basic)
};

enum DeviceType {
    DM_Unknown,
    DM_CV30,
    DM_CV30Pro,
    DM_CV40,
    DM_CV40C,
    DM_CV40Pro,
    DM_CV50,
    DM_CV50Pro,
    DM_CV50Pro60,
    DM_CV50ProS,
    DM_CV60,
};

enum class OtaStatus {
    Idle,
    Updating,
    Success,
    Failed
};

inline QString getOtaStatusName(OtaStatus v) {
    static const QMap<OtaStatus, QString> map {
        { OtaStatus::Idle, QStringLiteral("Idle") },
        { OtaStatus::Updating, QStringLiteral("Updating") },
        { OtaStatus::Success, QStringLiteral("Success") },
        { OtaStatus::Failed, QStringLiteral("Failed") }
    };
    return map.value(v, QStringLiteral("Unknown"));
}

enum class PrintStatus {
    Idle,               // 空闲
    Processing,         // 加工中
    Previewing,         // 预览中
    Paused,             // 加工暂停
    PreviewPaused,      // 预览暂停
    Busy,               // 设备忙：其它未收录的状态
};

inline QString getPrintStatusName(PrintStatus v) {
    static const QMap<PrintStatus, QString> map {
        { PrintStatus::Idle, QStringLiteral("Idle") },
        { PrintStatus::Processing, QStringLiteral("Processing") },
        { PrintStatus::Previewing, QStringLiteral("Previewing") },
        { PrintStatus::Paused, QStringLiteral("Paused") },
        { PrintStatus::PreviewPaused, QStringLiteral("PreviewPaused") },
        { PrintStatus::Busy, QStringLiteral("Busy") }
    };
    return map.value(v, QStringLiteral("Unknown"));
}

enum class DoorStatus {
    Closed,
    Open
};

inline QString getDoorStatusName(DoorStatus v) {
    static const QMap<DoorStatus, QString> map {
        { DoorStatus::Closed, QStringLiteral("Closed") },
        { DoorStatus::Open, QStringLiteral("Open") }
    };
    return map.value(v, QStringLiteral("Unknown"));
}

enum class FireDetectLevel {
    High,
    Low,
    Off,
};

inline FireDetectLevel parseFireDetectLevelString(const QString &v) {
    static const QMap<QString, FireDetectLevel> map {
        { QStringLiteral("high"), FireDetectLevel::High },
        { QStringLiteral("low"), FireDetectLevel::Low },
        { QStringLiteral("off"), FireDetectLevel::Off },
    };
    return map.value(v.toLower(), FireDetectLevel::Off);
}
inline QString toFireDetectLevelString(FireDetectLevel v) {
    static const QMap<FireDetectLevel, QString> map {
        { FireDetectLevel::High, QStringLiteral("high") },
        { FireDetectLevel::Low, QStringLiteral("low") },
        { FireDetectLevel::Off, QStringLiteral("off") }
    };
    return map.value(v, QStringLiteral("off"));
}

struct DeviceStatus {
    PrintStatus printStatus = PrintStatus::Idle;
    DoorStatus doorStatus = DoorStatus::Closed;
    QString alarmCode;

    OtaStatus otaStatus = OtaStatus::Idle;
    int otaProgress = 0; // 0 ~ 100

    QString conveyorAxis;     // 传送带加工轴
    QString feederAxis;       // 送料机加工轴
    QString rotaryAxis;       // 滚轮套件加工轴

    bool operator==(const DeviceStatus &v) const {
        return printStatus == v.printStatus
               && doorStatus == v.doorStatus
               && alarmCode == v.alarmCode
               && otaStatus == v.otaStatus
               && otaProgress == v.otaProgress
               && conveyorAxis == v.conveyorAxis
               && feederAxis == v.feederAxis
               && rotaryAxis == v.rotaryAxis;
    }
    bool operator!=(const DeviceStatus &v) const {
        return printStatus != v.printStatus
               || alarmCode != v.alarmCode
               || otaStatus != v.otaStatus
               || otaProgress != v.otaProgress
               || conveyorAxis != v.conveyorAxis
               || feederAxis != v.feederAxis
               || rotaryAxis != v.rotaryAxis;
    }

    QVariantMap toMap() const {
        QVariantMap m;
        m["printStatusName"] = getPrintStatusName(printStatus);
        m["printStatus"] = (int)printStatus;
        m["doorStatusName"] = getDoorStatusName(doorStatus);
        m["doorStatus"] = (int)doorStatus;
        m["alarmCode"] = alarmCode;

        m["otaStatusName"] = getOtaStatusName(otaStatus);
        m["otaStatus"] = (int)otaStatus;
        m["otaProgress"] = otaProgress;

        m["conveyorAxis"] = conveyorAxis;
        m["feederAxis"] = feederAxis;
        m["rotaryAxis"] = rotaryAxis;
        return m;
    }
};

enum DeviceAxis {
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS,
    A_AXIS,
    B_AXIS,
    C_AXIS,
    XY_AXIS,        //!<  同时回零XY
    XZ_AXIS,        //!<  同时回零XZ
    YZ_AXIS,        //!<  同时回零YZ
    MASK_AXIS,      //!<  对所支持的轴全部进行回零, 最终效果由GRBL解决
    MAX_AXIS,
};

// 静态能力
struct DeviceCapability
{
    DeviceType model = DM_Unknown;  // 设备型号
    QString displayName;            // 设备名称
    QString thumbnail;              // 设备缩略图
    QString guideImage;             // 操作指导图
    qreal workAreaWidth = 0;        // 工作区宽度/mm
    qreal workAreaHeight = 0;       // 工作区高度/mm

    QString defaultLaser;           // 默认光源
    QStringList presetLasers;       // 预设的激光列表

    ConnectionType defaultConnectionType = CT_Unknown;

    TimeUnit displayTimeUnit = TU_Minute;

    int speedMin = 0;
    int speedMax = 60000;
    int powerMin = 0;
    int powerMax = 100;

    int defaultBorderSpeed = 6000;  // 默认走边框速度(mm/min)
    int defaultBorderPower = 1;     // 默认走边框功率(%)

    bool supportAutoFocus = false;  // 是否支持自动对焦
    bool supportXYAxis = true;      // 是否支持XY轴移动
    bool supportZAxis = false;      // 是否支持Z轴移动
    bool supportAAxis = false;      // 是否支持A轴移动
    bool supportBAxis = false;      // 是否支持B轴移动
    bool supportConveyor = false;   // 是否支持传送带
    bool supportOta = false;        // 是否支持 OTA
    bool supportRotaryKit = false;  // 是否支持旋转套件
};

// 动态配置
struct DeviceConfig
{
    QString deviceId;                   // 设备id，索引真实的物理设备
    DeviceType model = DM_Unknown;      // 设备型号
    QString name;                       // 设备名称
    qreal workAreaWidth = 0;            // 工作区宽度
    qreal workAreaHeight = 0;           // 工作区高度

    QString laser;                      // 当前光源

    int borderSpeed = -1;               // 走边框速度(mm/min)
    int borderPower = -1;               // 走边框功率(%)

    QVariantMap toMap() const {
        QVariantMap m;
        m["deviceId"] = deviceId;
        m["modelType"] = model;
        m["name"] = name;
        m["workAreaWidth"] = workAreaWidth;
        m["workAreaHeight"] = workAreaHeight;

        m["laser"] = laser;

        m["borderSpeed"] = borderSpeed;
        m["borderPower"] = borderPower;
        return m;
    }

    QJsonObject toJson() const {
        QJsonObject m;
        m["deviceId"] = deviceId;
        m["model"] = model;
        m["name"] = name;
        m["workAreaWidth"] = workAreaWidth;
        m["workAreaHeight"] = workAreaHeight;

        m["laser"] = laser;

        m["borderSpeed"] = borderSpeed;
        m["borderPower"] = borderPower;
        return m;
    }

    static DeviceConfig fromJson(const QJsonObject &m) {
        DeviceConfig v;
        v.deviceId = m.value("deviceId").toString();
        v.model = (DeviceType)m.value("model").toInt(DM_Unknown);
        v.name = m.value("name").toString();
        v.workAreaWidth = m.value("workAreaWidth").toInt(0);
        v.workAreaHeight = m.value("workAreaHeight").toInt(0);

        v.laser = m.value("laser").toString();

        v.borderSpeed = m.value("borderSpeed").toInt(0);
        v.borderPower = m.value("borderPower").toInt(0);
        return v;
    }

    void applyDefaults(const DeviceCapability &cap) {
        if (model == DM_Unknown)
            model = cap.model;
        if (name.isEmpty())
            name = cap.displayName;
        if (workAreaWidth <= 0 || workAreaHeight <= 0) {
            workAreaWidth = cap.workAreaWidth;
            workAreaHeight = cap.workAreaHeight;
        }

        if (laser.isEmpty())
            laser = cap.defaultLaser;

        if (borderSpeed < 0)
            borderSpeed = cap.defaultBorderSpeed;
        if (borderPower < 0)
            borderPower = cap.defaultBorderPower;
    }
};

using ProbeFinishedCallback = std::function<void(QString, bool)>;
struct DeviceIdentifier {
    ConnectionType connectionType = CT_Unknown;
    TransportType transportType = TT_Unknown;
    QString address;
    quint16 vid;
    quint16 pid;
    QString serialNumber;
    QString manufacturer;
};

struct DeviceInfo
{
    DeviceIdentifier identifier;
    DeviceCapability cap;
    DeviceConfig cfg;

    DeviceType model = DM_Unknown;      // 型号
    QString modelName;
    ConnectionType connectionType = CT_Unknown; // 连接类型
    TransportType transportType = TT_Unknown;   // 介质类型
    QString address;                    // 地址：串口端口/IP地址

    QString sn;
    QString dn;
    QString laser;
    QString firmwareVersion;

    // for camera calib
    QString laserType;
    QString laserSn;
    QString fieldLens;

    bool isValid() const { return !address.isEmpty(); }

    QString id() const {
        QString v = QString::number(model);
        if (!dn.isEmpty())
            v.append(":"+dn);
        if (!sn.isEmpty())
            v.append(":"+sn);
        return v;
    }

    QVariantMap toMap() const {
        QVariantMap m;
        m["id"] = id();
        m["modelType"] = model;
        m["modelName"] = modelName;
        m["connectionType"] = connectionType;
        m["connectionTypeName"] = getConnectionTypeName(connectionType);
        m["transportType"] = transportType;
        m["transportTypeName"] = getTransportTypeName(transportType);
        m["address"] = address;

        m["sn"] = sn;
        m["dn"] = dn;
        m["laser"] = laser;
        m["firmwareVersion"] = firmwareVersion;

        m["laserType"] = laserType;
        m["laserSn"] = laserSn;
        m["fieldLens"] = fieldLens;
        return m;
    }
};

struct CameraInfo
{
    QString cameraId;
    QString displayName;
    QString defaultResolution;  // 默认分辨率
    QStringList resolutions;    // 分辨率列表
    QString opaque;             // 相机与设备相关性的权重判定
};

static inline QString fromSize(QSize v)
{ return QString("%1x%2").arg(v.width()).arg(v.height()); }

static QSize toSize(QString v)
{
    int p = v.indexOf("x");
    if (p == -1) return QSize();
    else return QSize(v.mid(0, p).toInt(), v.mid(p+1).toInt());
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICE_H
