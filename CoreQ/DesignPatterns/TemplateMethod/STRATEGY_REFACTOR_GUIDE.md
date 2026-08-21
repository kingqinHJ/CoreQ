# Strategy 模式重构指南（简化版，供其他项目使用）

## 1. 架构概览

将一个 QObject 巨型类（QML 壳）的内部分支逻辑，按设备/类型拆分为 Strategy 子类。

```
┌───────────────────────────────────────────────────┐
│          QML 壳 (QObject, 保留 PIMPL)              │
│  Q_PROPERTY / Q_ENUM / slots 签名不变              │
│  持有 std::unique_ptr<IStrategy>                  │
│  持有执行引擎: advance() / finishStep() / done()  │
└─────────────────────────┬─────────────────────────┘
                          │ 委托
         ┌────────────────┼────────────────┐
         ▼                ▼                ▼
   DeviceAStrategy   DeviceBStrategy   DeviceCStrategy
```

### 核心决策

| 决策 | 选择 | 原因 |
|------|------|------|
| 多态方式 | Strategy（组合） | QML 类型注册约束，壳对象必须是固定类型 |
| Strategy 基类 | 纯虚接口（非 QObject） | 不需要 moc，可测试性好 |
| 通知方式 | StrategyCallbacks（回调结构体注入） | 轻量解耦，可 mock |
| 数据模型 | CommonInfo（公共）+ Strategy 私有 State | 避免超集结构体膨胀 |
| 迁移方式 | Strangler Pattern + m_strategy 判空 | 安全退回旧路径 |
| PIMPL | 壳保留 PIMPL，Strategy 不用 | 壳隔离编译依赖，Strategy 内部直来直去 |

---

## 2. 文件清单 & 各自职责

你的新项目需要创建以下文件：

| 文件 | 职责 | 是否 QObject | 是否 PIMPL |
|------|------|:---:|:---:|
| `IStrategy.h` | 纯虚接口 | 否 | 否 |
| `StrategyCallbacks.h` | 回调结构体 | 否 | 否 |
| `CommonInfo.h` | 公共数据（所有设备共享的字段） | 否 | 否 |
| `StrategyFactory.h` | 工厂函数（根据类型创建） | 否 | 否 |
| `DeviceAStrategy.h/.cpp` | 具体策略实现 | 否 | 否 |
| `YourShell.h/.cpp` | QML 壳（对外接口不变） | **是** | **是** |
| `YourShellPrivate.h` | 壳的 PIMPL 内部类 | 否 | — |

---

## 3. 各文件模板

### 3.1 IStrategy.h（纯虚接口）

```cpp
#ifndef ISTRATEGY_H
#define ISTRATEGY_H

#include <QVariantMap>
#include <QVariantList>
#include <QVector>
#include <QStringList>
#include <functional>

struct CommonInfo;

class IStrategy
{
public:
    /// 步骤函数签名
    using StepFn = std::function<void(int callbackId, QStringList msg)>;

    virtual ~IStrategy() = default;

    // ===== UI 模型 =====
    virtual QVariantMap infoMap(const CommonInfo& common) const = 0;
    virtual void buildItems(std::function<void(QVariantList)> callback) = 0;
    virtual QVariantMap sectionTitles() const = 0;

    // ===== 步骤计划 =====
    virtual QVector<StepFn> buildRefreshPlan(int targetType) const = 0;
    virtual QVector<StepFn> buildUploadPlan() const = 0;
    virtual QVector<StepFn> buildRestoreFactoryPlan() const = 0;

    // ===== 前置条件 =====
    virtual bool isPrerequisitesMet() const = 0;

    // ===== 依赖关系 =====
    virtual bool isDependent(int sourceType, int checkType) const = 0;

    // ===== 生命周期 =====
    virtual void reset() = 0;
};

#endif // ISTRATEGY_H
```

### 3.2 StrategyCallbacks.h（回调结构体）

```cpp
#ifndef STRATEGYCALLBACKS_H
#define STRATEGYCALLBACKS_H

#include <functional>
#include <QStringList>
#include <QObject>

struct StrategyCallbacks
{
    /// QObject* 上下文，用于 QTimer::singleShot 安全调度
    /// 传入壳对象的 this，析构后排队的 lambda 自动取消
    QObject* context = nullptr;

    /// 数据变更通知
    std::function<void()> onInfoChanged;
    std::function<void()> onItemsChanged;

    /// 步骤完成通知（驱动执行引擎推进）
    std::function<void(bool ok, const QStringList& msg, bool retry, bool ignoreError)> onStepFinished;
};

#endif // STRATEGYCALLBACKS_H
```

**要点**：`context` 字段传入壳对象指针。Strategy 内部需要延迟执行时用：
```cpp
QTimer::singleShot(0, m_callbacks.context, [...]() { ... });
```
这样壳析构后 lambda 自动不执行，避免悬空指针。

### 3.3 CommonInfo.h（公共数据）

```cpp
#ifndef COMMONINFO_H
#define COMMONINFO_H

#include <QString>
#include <QVariantMap>

struct CommonInfo
{
    QString id;
    QString name;
    QString version;
    // ... 所有设备共有字段

    QVariantMap toMap() const {
        return {
            {"id", id},
            {"name", name},
            {"version", version},
        };
    }

    void reset() { *this = CommonInfo{}; }
};

#endif // COMMONINFO_H
```

### 3.4 StrategyFactory.h（工厂）

```cpp
#ifndef STRATEGYFACTORY_H
#define STRATEGYFACTORY_H

#include <memory>
#include "IStrategy.h"
#include "StrategyCallbacks.h"

// 前向声明各 Strategy
class DeviceAStrategy;
// class DeviceBStrategy;

inline std::unique_ptr<IStrategy> createStrategy(
    int deviceType,
    StrategyCallbacks callbacks)
{
    switch (deviceType) {
    // case DeviceType_A:
    //     return std::make_unique<DeviceAStrategy>(std::move(callbacks));
    default:
        return nullptr; // 走旧路径
    }
}

#endif // STRATEGYFACTORY_H
```

### 3.5 DeviceAStrategy.h（具体策略 - 示例骨架）

```cpp
#ifndef DEVICEASTRATEGY_H
#define DEVICEASTRATEGY_H

#include "IStrategy.h"
#include "CommonInfo.h"
#include "StrategyCallbacks.h"
#include <QElapsedTimer>

class DeviceAStrategy : public IStrategy
{
public:
    explicit DeviceAStrategy(StrategyCallbacks callbacks);
    ~DeviceAStrategy() override;

    // IStrategy 接口
    QVariantMap infoMap(const CommonInfo& common) const override;
    void buildItems(std::function<void(QVariantList)> callback) override;
    QVariantMap sectionTitles() const override;
    QVector<StepFn> buildRefreshPlan(int targetType) const override;
    QVector<StepFn> buildUploadPlan() const override;
    QVector<StepFn> buildRestoreFactoryPlan() const override;
    bool isPrerequisitesMet() const override;
    bool isDependent(int sourceType, int checkType) const override;
    void reset() override;

private:
    // 设备 A 专属状态
    struct State {
        bool stepADone = false;
        bool stepBDone = false;
        QString stepADuration;
    };

    // 步骤函数
    void doStepA(int cbId, QStringList msg);
    void doStepB(int cbId, QStringList msg);

    StrategyCallbacks m_callbacks;
    State m_state;
    QElapsedTimer m_elapsed;
    CommonInfo m_commonCache; // 缓存公共信息供壳读取
};

#endif // DEVICEASTRATEGY_H
```

### 3.6 壳对象（QML 壳 + PIMPL）

**YourShell.h**（对外接口零改动）：
```cpp
#ifndef YOURSHELL_H
#define YOURSHELL_H

#include <QObject>
#include <QJSValue>
#include <QVector>
#include <functional>
#include <memory>

class YourShellPrivate;
class IStrategy;

class YourShell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap info READ info NOTIFY infoChanged FINAL)
    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged FINAL)

public:
    explicit YourShell(QObject *parent = nullptr);
    ~YourShell();

    enum ItemType { TypeA, TypeB, TypeC };
    Q_ENUM(ItemType)

    QVariantMap info() const;
    QVariantList items() const;

public slots:
    void refresh(QJSValue callback, int targetType = -1);
    void upload(QJSValue callback);
    void restoreFactory(QJSValue callback);
    void reset();

signals:
    void infoChanged();
    void itemsChanged();

private:
    void switchStrategy(int deviceType);
    void advance();
    void finishStep(bool ok, const QStringList& msg, bool retry, bool ignoreError);
    void done(bool ok);

    std::shared_ptr<YourShellPrivate> d; // PIMPL
    std::unique_ptr<IStrategy> m_strategy;

    // 执行引擎状态
    using StepFn = std::function<void(int, QStringList)>;
    QVector<StepFn> m_plan;
    int m_index = 0;
    int m_callbackId = 0;
    QStringList m_msgs;
};

#endif // YOURSHELL_H
```

**关键接入点**（壳的 .cpp 里）：

```cpp
void YourShell::switchStrategy(int deviceType)
{
    StrategyCallbacks cb;
    cb.context              = this; // ← 关键：传入 QObject* 上下文
    cb.onInfoChanged        = [this]() { emit infoChanged(); };
    cb.onItemsChanged       = [this]() { emit itemsChanged(); };
    cb.onStepFinished       = [this](bool ok, const QStringList& msg, bool retry, bool ignore) {
        finishStep(ok, msg, retry, ignore);
    };

    m_strategy = createStrategy(deviceType, std::move(cb));
}

void YourShell::refresh(QJSValue callback, int targetType)
{
    m_callbackId = storeCallback(callback); // 你的回调存储机制

    if (m_strategy) {
        // 新路径
        m_plan = m_strategy->buildRefreshPlan(targetType);
        m_index = 0;
        m_msgs.clear();
        advance();
    } else {
        // 旧路径（原代码不动）
        __refresh(m_callbackId, targetType);
    }
}

void YourShell::advance()
{
    if (m_index >= m_plan.size()) {
        done(true);
        return;
    }
    m_plan[m_index](m_callbackId, m_msgs);
}

void YourShell::finishStep(bool ok, const QStringList& msg, bool retry, bool ignoreError)
{
    m_msgs = msg;
    if (!ok && !ignoreError) {
        if (retry && m_retryCount < MAX_RETRY) {
            m_retryCount++;
            m_plan[m_index](m_callbackId, m_msgs); // 重试当前步骤
            return;
        }
        done(false);
        return;
    }
    m_retryCount = 0;
    m_index++;
    advance();
}

void YourShell::done(bool ok)
{
    m_plan.clear();
    // 触发 QML 回调
    invokeCallback(m_callbackId, ok, m_msgs);
}
```

---

## 4. 关键设计要点

### 4.1 PIMPL 只用于壳

壳（QObject 类）使用 PIMPL 的目的是**隔离编译依赖**：
- 壳头文件只暴露 Q_PROPERTY/slot/signal
- 私有实现细节（成员变量、内部工具类指针）藏在 Private 类里
- 修改内部实现不触发依赖壳头文件的所有 .cpp 重编译

Strategy 类**不需要 PIMPL**：
- 不暴露给外部（只有壳通过接口指针使用）
- 头文件不会被广泛 include
- 内部逻辑直接、直白，不需要额外间接层

### 4.2 QTimer::singleShot 安全使用

Strategy 不是 QObject，不能用 `QMetaObject::invokeMethod`。需要延迟到下一轮事件循环时：

```cpp
// 安全写法：传入 context
QTimer::singleShot(0, m_callbacks.context, [this]() {
    doSomething(); // 如果 context 已析构，这行不会执行
});

// 危险写法：不传 context
QTimer::singleShot(0, [this]() {
    doSomething(); // 如果 Strategy 已析构，this 是悬空指针！
});
```

### 4.3 buildXxxPlan() 的 const 问题

`buildRefreshPlan` 标记为 `const`（只是组装计划，不执行步骤），但内部 lambda 捕获 `this` 后会修改状态。两种处理方式：

**方案 A**：去掉 const（推荐，诚实）
```cpp
QVector<StepFn> buildRefreshPlan(int targetType); // 非 const
```

**方案 B**：保留 const + const_cast（当前做法）
```cpp
QVector<StepFn> buildRefreshPlan(int targetType) const {
    plan.append([this](int cbId, QStringList msg) mutable {
        const_cast<DeviceAStrategy*>(this)->doStepA(cbId, msg);
    });
}
```

方案 A 更清晰，方案 B 的好处是语义上强调"组装计划本身不改状态"。

### 4.4 Strangler Pattern 退回机制

```cpp
// 退回单个设备：工厂返回 nullptr，走旧路径
if (m_strategy) { /* 新路径 */ } else { /* 旧路径 */ }

// 退回全部：switchStrategy() 函数体置空
void YourShell::switchStrategy(int) { /* 不创建任何 strategy */ }

// 彻底退回：git checkout 回原分支
```

### 4.5 执行引擎留在壳里

执行引擎（`advance/finishStep/done`）留在壳对象中，原因：
- 需要管理 QJSValue callback（QML 回调机制）
- 需要 emit signal（必须是 QObject）
- 重试/排队/忙碌标志 是全局流程状态，不属于某个设备策略

Strategy 通过 `onStepFinished` 回调把控制权交还给壳。

### 4.6 数据流向

```
设备/网络 ──HTTP──▶ Strategy 步骤函数
                         │
                         ▼ 写入 m_state (设备私有)
                         ▼ 写入 m_commonCache (公共)
                         │
                         ▼ m_callbacks.onInfoChanged()
                         │
                    壳 emit infoChanged()
                         │
                    QML 读取 info property
                         │
                    壳 调用 strategy->infoMap(common)
                         │
                    返回 QVariantMap 给 QML
```

---

## 5. 迁移步骤 Checklist

1. [ ] 创建 `IStrategy.h` / `StrategyCallbacks.h` / `CommonInfo.h`
2. [ ] 创建 `StrategyFactory.h`
3. [ ] 创建第一个 `DeviceAStrategy` 骨架（方法签名 + TODO）
4. [ ] 壳添加 `m_strategy` + `switchStrategy()` + 执行引擎
5. [ ] 壳的 `refresh/upload/restoreFactory` 添加 `if(m_strategy)` 分支
6. [ ] 从壳的 .cpp 搬运步骤函数实现到 Strategy
7. [ ] 联调验证新路径
8. [ ] 迁移其他设备
9. [ ] 清理旧代码

---

## 6. 与本项目的差异说明

| 本项目 | 你的项目 |
|--------|----------|
| 有 HttpManager（共享指针传入 Strategy） | 无 HTTP 库 — 去掉 `std::shared_ptr<HttpManager>` 参数，改为你的 IO/通信方式 |
| 有物理设备地址 `deviceAddress` | 无设备 — 去掉或替换为你的数据源标识 |
| AsyncChain / HttpDeviceDriver | 不需要 — 步骤函数里直接用你的异步方式 |
| `FirmwareFlasher::CalibrationType` 枚举在壳上 | 保持不变 — 枚举留在壳上供 QML 引用 |
| `QJSValue` 回调存储机制 | 保留或替换为你的回调方案（signal/slot 亦可） |
| `miniz` 打包 | 不需要 |
| `ClApplication* g_app` 全局 | 替换为你的全局上下文 |

---

## 7. 最小可运行示例结构

```
YourProject/
├── Strategy/
│   ├── IStrategy.h
│   ├── StrategyCallbacks.h
│   ├── CommonInfo.h
│   ├── StrategyFactory.h
│   ├── DeviceAStrategy.h
│   └── DeviceAStrategy.cpp
├── YourShell.h          (QObject 壳, PIMPL)
├── YourShell.cpp
└── YourShellPrivate.h   (壳的私有实现)
```

拿着这个结构和上面的模板代码，另一条对话里可以直接开始实现。
