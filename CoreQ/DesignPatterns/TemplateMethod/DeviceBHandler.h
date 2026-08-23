#ifndef DEVICEBHANDLER_H
#define DEVICEBHANDLER_H

#include "AbstractDeviceHandler.h"

struct DeviceBState
{
    double temperature = 0.0;
    bool crcValid = false;
    int bufferSize=0;
    int retryCount=0;
    QString firmwareVersion="-";
    bool advancedCalibration = false;
}


class DeviceBHandler : public AbstractDeviceHandler
{
public:
    using AbstractDeviceHandler::AbstractDeviceHandler;
    ~DeviceBHandler() override=default;

    QString deviceName() const override ;
    QVariantMap deviceSpecificInfo() const override;
    QVariantMap deviceSpecificTitle() const override;

    void reset() override;

protected:
    // 覆盖纯虚步骤
    void step_readDeviceInfo() override;
    void step_doDeviceAction() override;
    void step_upload_execute() override;

    // 覆盖钩子方法
    void step_validateData() override;
    void step_postProcess() override;
    void step_upload_prepare() override;

    // 覆盖公共步骤（设备B的前置条件检查更严格）
    void step_checkPrerequisites() override;

private:
    DeviceBState state;
};