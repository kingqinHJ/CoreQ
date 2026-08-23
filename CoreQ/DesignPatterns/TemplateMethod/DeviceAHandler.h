#ifndef DEVICEAHANDLER_H
#define DEVICEAHANDLER_H

#include "AbstractDeviceHandler.h"

struct DeviceAState
{
    bool calibrated = false;
    int calibrationValue = 0;
    QString mode = QStringLiteral("标准模式");
};

class DeviceAHandler : public AbstractDeviceHandler
{
public:
    using AbstractDeviceHandler::AbstractDeviceHandler;
    ~DeviceAHandler() override = default;

    QString deviceName() const override;
    QVariantMap deviceSpecificInfo() const override;
    QVariantMap deviceSpecificTitles() const override;
    void reset() override;

protected:
    void step_readDeviceInfo() override;
    void step_doDeviceAction() override;
    void step_upload_execute() override;

private:
    DeviceAState m_state;
};

#endif // DEVICEAHANDLER_H
