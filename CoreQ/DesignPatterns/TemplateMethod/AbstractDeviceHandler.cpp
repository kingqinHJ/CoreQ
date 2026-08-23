#include "AbstractDeviceHandler.h"
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>


AbstractDeviceHandler::AbstractDeviceHandler(HandlerCallbacks callbacks)
    : m_cb(std::move(cb))
{
    if (m_busy) {
        if (m_cb.logMessage) {
            m_cb.logMessage("⚠  当前正忙，忽略本次请求");
        }
        return;
    }
    m_busy = true;
    m_common.reset();
    m_common.name = deviceName();
    m_common.status = "刷新中...";
    m_common.progress = 0;
    if (m_cb.onDataChanged) m_cb.onDataChanged();

    if (m_cb.logMessage) {
        m_cb.logMessage(QString("========== 开始刷新: %1 ==========").arg(deviceName()));
    }

    // 模板方法核心：固定步骤顺序，子类不能改变流程
    QVector<StepFn> refreshSteps = {
        &AbstractDeviceHandler::step_checkConnection,
        &AbstractDeviceHandler::step_checkPrerequisites,
        &AbstractDeviceHandler::step_readDeviceInfo,
        &AbstractDeviceHandler::step_validateData,
        &AbstractDeviceHandler::step_doDeviceAction,
        &AbstractDeviceHandler::hook_postProcess,
    };
    executeSteps(refreshSteps);
}

void AbstractDeviceHandler::reset()
{
    m_busy = false;
    m_stepIndex = 0;
    m_currentSteps.clear();
    m_common.reset();
    if (m_cb.onDataChanged) m_cb.onDataChanged();
}

void AbstractDeviceHandler::executeSteps()
{
    m_currentSteps = steps;
    m_stepIndex = 0;
    advance();
}

void AbstractDeviceHandler::advance()
{
    if (m_stepIndex >= m_currentSteps.size()) {
        m_busy = false;
        m_common.status = "完成";
        m_common.progress = 100;
        if (m_cb.onDataChanged) m_cb.onDataChanged();
        if (m_cb.logMessage) {
            m_cb.logMessage(QString("========== %1 流程结束 ==========").arg(deviceName()));
        }
        if (m_cb.onFinished) {
            m_cb.onFinished(true);
        }
        return;
    }

    // 随机延迟 300~700ms，模拟异步IO/网络请求
    int delayMs = 300 + QRandomGenerator::global()->bounded(400);
    asyncDelay(delayMs, [this] {
       StepFn fn = m_currentSteps[m_stepIndex];
        (this->*fn)();  // 多态调用：子类覆盖的话执行子类版本
    });
}


void AbstractDeviceHandler::completeStep(bool ok, const QString& msg)
{
    if (m_cb.logMessage && !msg.isEmpty()) {
        m_cb.logMessage(msg);
    }
    if (!ok) {
        m_busy = false;
        m_common.status = "失败";
        if (m_cb.onDataChanged) m_cb.onDataChanged();
        if (m_cb.logMessage) {
            m_cb.logMessage("❌ 流程失败！");
        }
        if (m_cb.onFinished) {
            m_cb.onFinished(false);
        }
        return;
    }
    m_stepIndex++;
    // 更新进度（按步骤比例）
    if (!m_currentSteps.isEmpty()) {
        m_common.progress = (m_stepIndex * 100) / m_currentSteps.size();
    }
    if (m_cb.onDataChanged) m_cb.onDataChanged();
    advance();
}

void AbstractDeviceHandler::asyncDelay(int ms, std::function<void()> action)
{
    if (m_cb.context) {
        // 安全写法：传入 context，context 析构后 lambda 自动不执行
        QTimer::singleShot(ms, m_cb.context, [action]() {
            action();
        });
    } else {
        QTimer::singleShot(ms, [action]() {
            action();
        });
    }
}

// ===== 公共步骤实现 =====
void AbstractDeviceHandler::step_checkConnection()
{
    if (m_cb.logMessage) {
        m_cb.logMessage("  → 检查设备连接...");
    }
    asyncDelay(300, [this]() {
        m_common.status = "已连接";
        completeStep(true, "  ✓ 连接正常");
    });
}

void AbstractDeviceHandler::step_checkPrerequisites()
{
    if (m_cb.logMessage) {
        m_cb.logMessage("  → 检查前置条件...");
    }
    asyncDelay(200, [this]() {
        completeStep(true, "  ✓ 前置条件满足（默认检查）");
    });
}

// ===== 钩子默认实现 =====
void AbstractDeviceHandler::step_validateData()
{
    completeStep(true, "  ✓ 数据校验通过（默认校验）");
}

void AbstractDeviceHandler::hook_postProcess()
{
    completeStep(true, "  ✓ 后处理完成（默认空操作）");
}

void AbstractDeviceHandler::step_upload_prepare()
{
    if (m_cb.logMessage) {
        m_cb.logMessage("  → 准备上传资源...");
    }
    asyncDelay(300, [this]() {
        completeStep(true, "  ✓ 上传准备完成");
    });
}