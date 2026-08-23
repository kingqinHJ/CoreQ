#ifndef ABSTRACTDEVICEHANDLER_H
#define ABSTRACTDEVICEHANDLER_H

#include <QVariantMap>
#include <QVector>
#include <QString>
#include <functional>
#include "CommonInfo.h"
#include "HandlerCallbacks.h"

class AbstractDeviceHandler
{
public:
    explicit AbstractDeviceHandler(HandlerCallbacks cb);
    virtual ~AbstractDeviceHandler() = default;

    // ===== 模板方法（public 入口，非 virtual）=====
    void startRefresh();
    void startUpload();
    virtual void reset();

    // ===== 设备信息 =====
    virtual QString deviceName() const = 0;

    // ===== 数据访问（供 UI 读取）=====
    const CommonInfo& commonInfo() const { return m_common; }
    virtual QVariantMap deviceSpecificInfo() const { return {}; }
    virtual QVariantMap deviceSpecificTitles() const { return {}; }

    bool isBusy() const { return m_busy; }

protected:
    // ===== 步骤函数指针类型（成员函数指针）=====
    using StepFn = void (AbstractDeviceHandler::*)();

    // ===== 异步推进引擎（基类实现，子类不覆盖）=====
    void executeSteps(const QVector<StepFn>& steps);
    void advance();
    void completeStep(bool ok, const QString& msg);
    void asyncDelay(int ms, std::function<void()> action);

    // ===== 【具体步骤】公共步骤：基类实现，所有设备共享（不允许覆盖）=====
    void step_checkConnection();

    // ===== 【纯虚步骤】设备特定：子类必须实现 =====
    virtual void step_readDeviceInfo() = 0;
    virtual void step_doDeviceAction() = 0;
    virtual void step_upload_execute() = 0;

    // ===== 【钩子方法】基类默认实现，子类可选覆盖 =====
    virtual void step_checkPrerequisites();
    virtual void step_validateData();
    virtual void hook_postProcess();
    virtual void step_upload_prepare();

    HandlerCallbacks m_cb;
    CommonInfo m_common;

private:
    QVector<StepFn> m_currentSteps;
    int m_stepIndex = 0;
    bool m_busy = false;
};

#endif // ABSTRACTDEVICEHANDLER_H
