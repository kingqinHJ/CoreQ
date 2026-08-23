#ifndef ABSTRACTDEVICEHANDLER_H
#define ABSTRACTDEVICEHANDLER_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <functional>
#include <QString>
#include "CommonInfo.h"
#include "HandlerCallbacks.h"

class AbstractDeviceHandler : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDeviceHandler(HandlerCallbacks callbacks);
    virtual ~AbstractDeviceHandler()=default;
    
    //模板方法
    void startRefresh();
    void stopUpload();
    void reset();

    //设备信息
    virtual QString deviceName() const=0;

    const CommonInfo& commonInfo() const=0;
    virtual QVariantMap deviceSpecificInfo() const=
    /// 构造函数
    AbstractDeviceHandler(QObject* parent = nullptr);

    QObject* context = nullptr;

    std::function<void(const QString& log)> logMessage;

    std::function<void(bool ok)> onFinish;

    std::function<void()> onDataChanged;

protected:
    / ===== 步骤函数指针类型（成员函数指针）=====
    using StenFn = void(AbstractDeviceHandler::*)();

    // ===== 异步推进引擎（基类实现，子类不覆盖）=====
    void executeStep(const QVector<StenFn>& steps);
    void advance();
    void completeStep(bool ok,const QString &msg);
    void asyncDelay(int ms,std::function<void()> action);

    // ===== 【具体步骤】公共步骤：基类实现，所有设备共享 =====
    void step_checkConnection();
    void step_checkPrerequisites();


    // ===== 【纯虚步骤】设备特定：子类必须实现 =====
    virtual void step_readDeviceInfo()=0;
    virtual void step_doDeviceAction()=0;
    virtual void step_upload_execute()=0;

    // ===== 【钩子方法】基类默认实现，子类可选覆盖 =====
    virtual void step_validateData()=0;
    virtual void step_postProcess()=0;
    virtual void step_upload_prepare()=0;

    HandlerCallbacks m_cb;
    CommonInfo m_common;
private:
    QVector<StenFn> m_currentSteps;
    int m_stepIndex=0;
    bool m_busy=false;
};
#endif // ABSTRACTDEVICEHANDLER_H