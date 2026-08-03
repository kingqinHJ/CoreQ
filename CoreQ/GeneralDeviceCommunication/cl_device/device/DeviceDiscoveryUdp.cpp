#include "DeviceDiscoveryUdp.h"
#include "DeviceDef.h"
#include "common/Utils.h"

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkProxy>

static const int listenPort = 12345;

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceDiscoveryUdpPrivate
{
public:
    std::shared_ptr<QUdpSocket> udp;
};

DeviceDiscoveryUdp::DeviceDiscoveryUdp()
{
    d.reset(new DeviceDiscoveryUdpPrivate);
}

DeviceDiscoveryUdp::~DeviceDiscoveryUdp()
{
    stop();
    LOG_THIS();
}

bool DeviceDiscoveryUdp::start()
{
    if (d->udp)
        return false;

    d->udp = std::make_shared<QUdpSocket>();
    d->udp->setProxy(QNetworkProxy::NoProxy);
    if (!d->udp->bind(QHostAddress::AnyIPv4, listenPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        LOGW("bind error(%d): %s", listenPort, qUtf8Printable(d->udp->errorString()));
        return false;
    }

    connect(d->udp.get(), &QUdpSocket::readyRead, [this](){
        while (d->udp->hasPendingDatagrams()) {
            QNetworkDatagram datagram = d->udp->receiveDatagram();

            // {"version":1,"devName":"cv_laser_40pro","ip":"172.23.222.87","port":"8080","subnet":"255.255.240.0"}
            auto jo = Utils::stringToJson(datagram.data()).toObject();
            auto ip = jo.value("ip").toString();
            auto port = jo.value("port").toString();
            if (ip.isEmpty())
                ip = datagram.senderAddress().toString();
            if (port.isEmpty())
                port = QStringLiteral("8080");

            DeviceIdentifier id;
            id.connectionType = CT_Http;
            id.transportType = ip == "192.168.10.1" ? TT_USB : TT_WiFi;
            id.address = QString("%1:%2").arg(ip, port);
            emit deviceDiscovered(id);
        }
    });

    LOG_THIS();
    return true;
}

void DeviceDiscoveryUdp::stop()
{
    if (d->udp) {
        d->udp->disconnect(this);
        d->udp->close();
        d->udp.reset();
        LOG_THIS();
    }
}

bool DeviceDiscoveryUdp::isRunning()
{
    return d->udp.get() != NULL;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
