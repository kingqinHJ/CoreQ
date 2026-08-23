#ifndef DEVICEAHANDLER_H
#define DEVICEAHANDLER_H

#include "AbstractDeviceHandler.h"

struct DeviceAState
{
    bool calibrated = false;
    int calibrationValue = 0;
    QString mode="normal";
}

class DeviceAHandler : public AbstractDeviceHandler
{
public:
    using AbstractDeviceHandler::AbstractDeviceHandler;
    DeviceAHandler(QObject* parent = nullptr);
    ~DeviceAHandler() override;

    QString deviceName() const override ;
    QVariantMap deviceSpecificInfo() const override;
    QVariantMap deviceSpecificTitle() const override;

    void reset() override;

protected:
    void step_readCalibrationValue() override;
    void step_doDeviceAction() override;
    void step_upload_execute() override;

    void executeSteps() override;

private:
    DeviceAState m_state;
};
#endif