#include "DeviceCapabilityManager.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceCapabilityManagerPrivate
{
public:
    QMap<DeviceType, DeviceCapability> defaultCapability;
    QList<DeviceCapability> capabilityList;
};

DeviceCapabilityManager::DeviceCapabilityManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new DeviceCapabilityManagerPrivate);

    // CV30
    DeviceCapability cap;
    cap.model = DM_CV30;
    cap.thumbnail = "qrc:/cl_device/CR-Laser_Falcon.png";
    cap.guideImage = "qrc:/cl_device/guide/CR-Laser_Falcon.png";
    cap.displayName = "CR-Laser Falcon";
    cap.workAreaWidth = 400;
    cap.workAreaHeight = 415;
    cap.defaultLaser = "5W";
    cap.presetLasers = QStringList{ "5W", "7.5W", "10W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV30 Pro
    cap = DeviceCapability();
    cap.model = DM_CV30Pro;
    cap.thumbnail = "qrc:/cl_device/CR-Laser_Falcon_Pro.png";
    cap.guideImage = "qrc:/cl_device/guide/CR-Laser_Falcon.png";
    cap.displayName = "CR-Laser Falcon Pro";
    cap.workAreaWidth = 400;
    cap.workAreaHeight = 415;
    cap.defaultLaser = "10W";
    cap.presetLasers = QStringList{ "10W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV50
    cap = DeviceCapability();
    cap.model = DM_CV50;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon2.png";
    cap.guideImage = "qrc:/cl_device/guide/CR-Laser_Falcon.png";
    cap.displayName = "Creality Falcon2";
    cap.workAreaWidth = 400;
    cap.workAreaHeight = 415;
    cap.defaultLaser = "22W";
    cap.presetLasers = QStringList{ "12W", "22W", "40W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV50 Pro
    cap = DeviceCapability();
    cap.model = DM_CV50Pro;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon2_Pro.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon2.png";
    cap.displayName = "Creality Falcon2 Pro";
    cap.workAreaWidth = 400;
    cap.workAreaHeight = 415;
    cap.defaultLaser = "22W";
    cap.presetLasers = QStringList{ "1.6W", "22W", "40W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV50 Pro 60W
    cap = DeviceCapability();
    cap.model = DM_CV50Pro60;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon2_Pro.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon2.png";
    cap.displayName = "Creality Falcon2 Pro 60W";
    cap.workAreaWidth = 400;
    cap.workAreaHeight = 400;
    cap.defaultLaser = "40W";
    cap.presetLasers = QStringList{ "1.6W", "22W", "40W", "60W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV50 Pro S
    cap = DeviceCapability();
    cap.model = DM_CV50ProS;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon2_Pro_S.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon2.png";
    cap.displayName = "Creality Falcon2 Pro S";
    cap.workAreaWidth = 355;
    cap.workAreaHeight = 390;
    cap.defaultLaser = "22W";
    cap.presetLasers = QStringList{ "22W", "40W" };
    cap.defaultConnectionType = CT_Serial;
    cap.supportZAxis = true;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV40
    cap = DeviceCapability();
    cap.model = DM_CV40;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon_A1.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon_A1.png";
    cap.displayName = "Creality Falcon A1";
    cap.workAreaWidth = 381;
    cap.workAreaHeight = 305;
    cap.defaultLaser = "10W";
    cap.presetLasers = QStringList{ "10W" };
    cap.defaultConnectionType = CT_Serial;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV40 Pro
    cap = DeviceCapability();
    cap.model = DM_CV40Pro;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon_A1_Pro.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon_A1.png";
    cap.displayName = "Creality Falcon A1 Pro";
    cap.workAreaWidth = 358;
    cap.workAreaHeight = 268;
    cap.defaultLaser = "20W";
    cap.presetLasers = QStringList{ "2W IR", "20W" };
    cap.defaultConnectionType = CT_Http;
    cap.supportAutoFocus = true;
    cap.supportZAxis = true;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV60
    cap = DeviceCapability();
    cap.model = DM_CV60;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon_T1.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon_T1.png";
    cap.displayName = "Creality Falcon T1";
    cap.workAreaWidth = 175;
    cap.workAreaHeight = 175;
    cap.defaultLaser = "20W Diode";
    cap.presetLasers = QStringList{ "20W Diode", "20W Fiber", "5W UV", "40W Diode", "60W MOPA" };
    cap.defaultConnectionType = CT_Http;
    cap.displayTimeUnit = TU_Second;
    cap.speedMax = 600000;
    cap.defaultBorderSpeed = 600000;
    cap.supportAutoFocus = true;
    cap.supportXYAxis = false;
    cap.supportZAxis = true;
    cap.supportAAxis = true;
    cap.supportBAxis = true;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);

    // CV40C
    cap = DeviceCapability();
    cap.model = DM_CV40C;
    cap.thumbnail = "qrc:/cl_device/Creality_Falcon_A1C.png";
    cap.guideImage = "qrc:/cl_device/guide/Creality_Falcon_A1C.png";
    cap.displayName = "Creality Falcon A1C";
    cap.workAreaWidth = 150;
    cap.workAreaHeight = 150;
    cap.defaultLaser = "5W";
    cap.presetLasers = QStringList{ "5W" };
    cap.defaultConnectionType = CT_Serial;
    cap.supportAutoFocus = true;
    cap.supportZAxis = true;
    d->defaultCapability[cap.model] = cap;
    d->capabilityList.append(cap);
}

DeviceCapabilityManager::~DeviceCapabilityManager()
{

}

DeviceCapability DeviceCapabilityManager::getCapability(DeviceType model) const
{
    return d->defaultCapability.value(model);
}

QString DeviceCapabilityManager::getModelName(DeviceType model) const
{
    return d->defaultCapability.value(model).displayName;
}

QList<DeviceCapability> DeviceCapabilityManager::getCapabilityList() const
{
    return d->capabilityList;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
