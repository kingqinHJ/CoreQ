#include "DeviceDiscoverySerial.h"
#include "DeviceDef.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <SetupAPI.h>
#include <Usbiodef.h>
#include <Usbioctl.h>
#include <devguid.h>
#include <winioctl.h>
#include <INITGUID.h>
#include <Dbt.h>
#include <cfgmgr32.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "user32.lib")
#endif

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

static const int updatePortTime = 2000;

class USBDeviceEventFilter;

class DeviceDiscoverySerialPrivate
{
public:
    QTimer timer;
    int timerId = -1;

    QMap<QString, DeviceIdentifier> portMap;

    std::shared_ptr<USBDeviceEventFilter> usbFilter;
};

#ifdef Q_OS_WIN
class USBDeviceEventFilter : public QAbstractNativeEventFilter
{
public:
    USBDeviceEventFilter(DeviceDiscoverySerial *q) : q(q) {
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr*result) override
#else
    bool nativeEventFilter(const QByteArray& eventType, void* message, long*result) override
#endif
    {
        if (eventType == "windows_generic_MSG") {
            MSG* msg = reinterpret_cast<MSG*>(message);
            UINT msgType = msg->message;
            if (msgType == WM_DEVICECHANGE) {
                PDEV_BROADCAST_HDR lpdb = PDEV_BROADCAST_HDR(msg->lParam);
                switch (msg->wParam) {
                case DBT_DEVICEARRIVAL:
                    if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                        PDEV_BROADCAST_DEVICEINTERFACE pDevInf = PDEV_BROADCAST_DEVICEINTERFACE(lpdb);
                        LOGD("append device: %s", qUtf8Printable(QString::fromWCharArray((wchar_t*)pDevInf->dbcc_name, int(pDevInf->dbcc_size))));
                    }
                    break;
                case DBT_DEVICEREMOVECOMPLETE:
                    if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                        PDEV_BROADCAST_DEVICEINTERFACE pDevInf = PDEV_BROADCAST_DEVICEINTERFACE(lpdb);
                        LOGD("remove device: %s", qUtf8Printable(QString::fromWCharArray((wchar_t*)pDevInf->dbcc_name, int(pDevInf->dbcc_size))));
                    }
                    break;
                case DBT_DEVNODES_CHANGED:
                    break;
                }

                LOGD("USB device changed");
                q->updateDevice();
            }
        }

        return false;
    }
private:
    DeviceDiscoverySerial *q;
};
#endif

DeviceDiscoverySerial::DeviceDiscoverySerial()
{
    d.reset(new DeviceDiscoverySerialPrivate);

    d->timer.setInterval(500);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, [this](){
        updateDevice(false);
    });
}

DeviceDiscoverySerial::~DeviceDiscoverySerial()
{
    stop();
    LOG_THIS();
}

bool DeviceDiscoverySerial::start()
{
    if (d->timerId != -1)
        return false;

    d->timerId = startTimer(updatePortTime);

#ifdef Q_OS_WIN
    d->usbFilter = std::make_shared<USBDeviceEventFilter>(this);
    d->portMap.clear();
    qApp->installNativeEventFilter(d->usbFilter.get());
#endif

    updateDevice();
    LOG_THIS();
    return true;
}

void DeviceDiscoverySerial::stop()
{
    if (d->timerId == -1)
        return;

    killTimer(d->timerId);
    d->timerId = -1;

    d->usbFilter.reset();
    d->portMap.clear();
#ifdef Q_OS_WIN
    qApp->removeNativeEventFilter(d->usbFilter.get());
#endif
    LOG_THIS();
}

bool DeviceDiscoverySerial::isRunning()
{
    return d->timerId != -1;
}

void DeviceDiscoverySerial::timerEvent(QTimerEvent *e)
{
    if (!d->timer.isActive())
        updateDevice();
}

void DeviceDiscoverySerial::updateDevice(bool delay)
{
    if (delay) {
        d->timer.stop();
        d->timer.start();
        return;
    }

    QMap<QString, DeviceIdentifier> newPortMap;

    auto ports = QSerialPortInfo::availablePorts();
    for (auto &it: ports) {
        DeviceIdentifier id;
        id.connectionType = CT_Serial;
        id.transportType = TT_USB;
        id.address = it.portName();
        id.vid = it.hasVendorIdentifier() ? it.vendorIdentifier() : 0;
        id.pid = it.hasProductIdentifier() ? it.productIdentifier() : 0;
        id.serialNumber = it.serialNumber();
        id.manufacturer = it.manufacturer();

        newPortMap.insert(id.address, id);
    }

    for (auto &it: newPortMap) {
        // if (!d->portMap.contains(it.address)) {
            emit deviceDiscovered(it);
        // }
    }

    for (auto &it: d->portMap) {
        if (!newPortMap.contains(it.address)) {
            emit deviceRemoved(it);
        }
    }

    d->portMap = newPortMap;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
