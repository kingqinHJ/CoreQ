#ifndef DEVICEBHANDLER_H
#define DEVICEBHANDLER_H

#include "AbstractDeviceHandler.h"

struct DeviceBState
{
    double temperature = 0.0;
    bool crcValid = false;
    int bufferSize = 0;
    int retryCount = 0;
    QString firmwareHash = QStringLiteral("—");
    bool advancedCalibrated = false;
};

class DeviceBHandler : public AbstractDeviceHandler
{
public:
    using AbstractDeviceHandler::AbstractDeviceHandler;
    ~DeviceBHandler() override = default;

    QString deviceName() const override;
    QVariantMap deviceSpecificInfo() const override;
    QVariantMap deviceSpecificTitles() const override;
    void reset() override;

protected:
    void step_readDeviceInfo() override;
    void step_doDeviceAction() override;
    void step_upload_execute() override;

    void step_validateData() override;
    void hook_postProcess() override;
    void step_upload_prepare() override;
    void step_checkPrerequisites() override;

private:
    DeviceBState m_state;
};

#endif // DEVICEBHANDLER_H
