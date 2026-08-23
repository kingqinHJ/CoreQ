#include "DeviceAHandler.h"

QString DeviceAHandler::deviceName() const
{
    return QStringLiteral("设备A(简易版)");
}

QVariantMap DeviceAHandler::deviceSpecificInfo() const
{
    return {
        {"calibrated", m_state.calibrated ? QStringLiteral("是") : QStringLiteral("否")},
        {"calibrationValue", m_state.calibrationValue},
        {"mode", m_state.mode},
    };
}

QVariantMap DeviceAHandler::deviceSpecificTitles() const
{
    return {
        {"calibrated", QStringLiteral("已校准")},
        {"calibrationValue", QStringLiteral("校准值")},
        {"mode", QStringLiteral("工作模式")},
    };
}

void DeviceAHandler::reset()
{
    AbstractDeviceHandler::reset();
    m_state = DeviceAState{};
}

void DeviceAHandler::step_readDeviceInfo()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 读取设备A基本信息..."));
    }
    asyncDelay(350, [this]() {
        m_common.id = QStringLiteral("A001");
        m_common.version = QStringLiteral("1.0.3");
        completeStep(true, QStringLiteral("  ✓ 设备A信息: ID=A001, 固件版本=1.0.3"));
    });
}

void DeviceAHandler::step_doDeviceAction()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 执行设备A快速校准..."));
    }
    asyncDelay(300, [this]() {
        m_state.calibrated = true;
        m_state.calibrationValue = 42;
        completeStep(true, QStringLiteral("  ✓ 校准完成, 校准值=42"));
    });
}

void DeviceAHandler::step_upload_execute()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 上传固件到设备A..."));
    }
    asyncDelay(500, [this]() {
        completeStep(true, QStringLiteral("  ✓ 设备A固件上传完成"));
    });
}
