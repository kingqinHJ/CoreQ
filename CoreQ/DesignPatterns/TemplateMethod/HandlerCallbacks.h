#ifndef HANDLERCALLBACKS_H
#define HANDLERCALLBACKS_H

#include <QObject>
#include <QString>
#include <functional>

struct HandlerCallbacks
{
    /// QObject* 上下文，用于 QTimer::singleShot 安全调度
    /// 传入 Widget 指针，析构后排队的 lambda 自动取消
    QObject* context = nullptr;

    /// 追加日志消息到 UI
    std::function<void(const QString&)> logMessage;

    /// 流程完成（成功或失败）
    std::function<void(bool ok)> onFinished;

    /// 数据变更（公共数据或私有数据变化，通知 UI 刷新面板）
    std::function<void()> onDataChanged;
};

#endif // HANDLERCALLBACKS_H
