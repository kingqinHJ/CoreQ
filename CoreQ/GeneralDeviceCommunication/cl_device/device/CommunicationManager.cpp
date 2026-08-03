#include "CommunicationManager.h"
#include "CommunicationSerial.h"
#include "CommunicationHttp.h"

static const int delayDestoryTime = 5000;

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

struct CommunicationInfo
{
    CommunicationInfo(CommunicationInterfacePtr comm) : comm(comm) {}
    CommunicationInterfacePtr comm;
    QElapsedTimer destroyTimer;
    int ref = 0;
};

using CommunicationInfoPtr = std::shared_ptr<CommunicationInfo>;

class CommunicationManagerPrivate
{
public:
    QMap<QString, CommunicationInfoPtr> cachedComm;
};

CommunicationManager::CommunicationManager(QObject *parent)
    : QObject{parent}
{
    d.reset(new CommunicationManagerPrivate);
    startTimer(1000);
}

CommunicationInterfaceWPtr CommunicationManager::create(const QString &address, ConnectionType type)
{
    if (d->cachedComm.contains(address)) {
        d->cachedComm[address]->ref++;
        d->cachedComm[address]->destroyTimer.invalidate();
        return d->cachedComm[address]->comm;
    }

    CommunicationInterfacePtr comm;
    switch (type) {
    case CT_Serial:
        comm.reset(new CommunicationSerial);
        break;
    case CT_Http:
        comm.reset(new CommunicationHttp);
        break;
    default:
        break;
    }

    comm->setAddress(address);
    comm->open();

    auto info = std::make_shared<CommunicationInfo>(comm);
    info->ref =1;
    d->cachedComm.insert(address, info);
    return comm;
}

void CommunicationManager::destroy(const QString &address, bool immediate)
{
    if (d->cachedComm.contains(address)) {
        auto info = d->cachedComm[address];
        info->ref--;

        if (info->ref <= 0) {

            if (immediate) {
                d->cachedComm.remove(address);
            }
            else {
                d->cachedComm[address]->destroyTimer.restart();
            }
        }
    }
}

CommunicationInterfaceWPtr CommunicationManager::find(const QString &address)
{
    return d->cachedComm.value(address)->comm;
}

void CommunicationManager::timerEvent(QTimerEvent *e)
{
    auto list = d->cachedComm.keys();
    for (auto &key: list) {
        auto info = d->cachedComm[key];
        if (info->destroyTimer.isValid() && info->destroyTimer.hasExpired(delayDestoryTime)) {
            d->cachedComm.remove(key);
        }
    }
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
