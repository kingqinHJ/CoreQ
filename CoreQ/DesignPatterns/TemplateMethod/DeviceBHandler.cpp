#include "DeviceBHandler.h"
#include <QRandomGenerator>

QString DeviceBHandler::deviceName() const
{
    return QStringLiteral("设备B(高级版)");
}

QVariantMap DeviceBHandler::deviceSpecificInfo() const
{
    return {
        {"temperature", QString::number(m_state.temperature, 'f', 1) + QStringLiteral(" °C")},
        {"crcValid", m_state.crcValid ? QStringLiteral("通过") : QStringLiteral("未校验")},
        {"bufferSize", QString::number(m_state.bufferSize) + QStringLiteral(" B")},
        {"retryCount", m_state.retryCount},
        {"firmwareHash", m_state.firmwareHash},
        {"advancedCalibrated", m_state.advancedCalibrated ? QStringLiteral("是") : QStringLiteral("否")},
    };
}

QVariantMap DeviceBHandler::deviceSpecificTitles() const
{
    return {
        {"temperature", QStringLiteral("传感器温度")},
        {"crcValid", QStringLiteral("CRC校验")},
        {"bufferSize", QStringLiteral("缓冲区大小")},
        {"retryCount", QStringLiteral("重试次数")},
        {"firmwareHash", QStringLiteral("固件哈希")},
        {"advancedCalibrated", QStringLiteral("高级校准")},
    };
}

void DeviceBHandler::reset()
{
    AbstractDeviceHandler::reset();
    m_state = DeviceBState{};
}

void DeviceBHandler::step_checkPrerequisites()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 检查设备B高级前置条件（温度、电压、权限...）"));
    }
    asyncDelay(500, [this]() {
        completeStep(true, QStringLiteral("  ✓ 高级前置条件全部满足"));
    });
}

void DeviceBHandler::step_readDeviceInfo()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 读取设备B扩展信息（含传感器数据）..."));
    }
    asyncDelay(700, [this]() {
        m_common.id = QStringLiteral("B002");
        m_common.version = QStringLiteral("2.1.7");
        m_state.temperature = 35.0 + QRandomGenerator::global()->bounded(30) / 10.0;
        completeStep(true, QStringLiteral("  ✓ 设备B信息: ID=B002, 固件=2.1.7, 温度=%1°C")
                              .arg(m_state.temperature, 0, 'f', 1));
    });
}

void DeviceBHandler::step_validateData()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 执行设备B高级数据校验（CRC32 + 数字签名）..."));
    }
    asyncDelay(600, [this]() {
        m_state.crcValid = true;
        completeStep(true, QStringLiteral("  ✓ CRC32校验通过，数字签名有效"));
    });
}

void DeviceBHandler::step_doDeviceAction()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 执行设备B高级校准（多点校准+温度补偿）..."));
    }
    asyncDelay(800, [this]() {
        m_state.advancedCalibrated = true;
        m_state.bufferSize = 1024;
        m_state.firmwareHash = QStringLiteral("a3f2c9d1");
        completeStep(true, QStringLiteral("  ✓ 高级校准完成, 温度补偿已启用, 缓冲区=1024B"));
    });
}

void DeviceBHandler::hook_postProcess()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 设备B后处理：清理临时缓冲区、同步状态到闪存..."));
    }
    asyncDelay(400, [this]() {
        m_state.bufferSize = 0;
        completeStep(true, QStringLiteral("  ✓ 临时缓冲区已清理，闪存同步完成"));
    });
}

void DeviceBHandler::step_upload_prepare()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 设备B上传准备：计算分片、校验固件签名..."));
    }
    asyncDelay(500, [this]() {
        completeStep(true, QStringLiteral("  ✓ 固件签名验证通过，分片数=16"));
    });
}

void DeviceBHandler::step_upload_execute()
{
    if (m_cb.logMessage) {
        m_cb.logMessage(QStringLiteral("  → 分片上传固件到设备B（含进度回调）..."));
    }
    asyncDelay(1200, [this]() {
        m_state.retryCount = 1;
        completeStep(true, QStringLiteral("  ✓ 固件上传完成（重试1次）"));
    });
}
