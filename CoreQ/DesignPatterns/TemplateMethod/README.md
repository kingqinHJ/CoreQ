# 模板方法模式（Template Method）示例

> 背景：对应现实项目中的多设备分支逻辑重构（参见上级目录 `STRATEGY_REFACTOR_GUIDE.md`）。
> Strategy 版已完成，本目录探索用模板方法模式解决同一类问题。

## 模式定义

在抽象基类中定义算法的**骨架**（固定流程），把会随设备/类型变化的步骤延迟到子类实现。
父类控制流程顺序，子类只填充/微调具体步骤。

```
AbstractDeviceHandler（抽象基类，定义骨架）
│  handle() {                     ← 模板方法：固定流程，final 不允许覆盖
│      step1_checkPrerequisites() ← 公共步骤，基类实现
│      step2_doDeviceSpecific()   ← 抽象步骤，子类必须实现
│      step3_postProcess()        ← 钩子(hook)，基类默认实现，子类可选覆盖
│  }
├── DeviceAHandler
└── DeviceBHandler
```

## 与 Strategy 模式的关键差异（本项目视角）

| 维度 | Strategy（组合） | Template Method（继承） |
|------|-----------------|------------------------|
| 关系 | 壳 has-a IStrategy | 壳基类 is-a，子类继承 |
| 运行时切换 | 可以（换指针即可） | 不可以（编译期确定类型） |
| 流程控制权 | 壳的执行引擎驱动 | 基类模板方法驱动 |
| 复用粒度 | 整个算法可替换 | 只替换个别步骤，其余共享 |
| QML 壳约束 | 适合（壳类型固定） | 需注意：QML 注册类型必须固定 |

## 待设计问题（写代码前需要想清楚）

1. [ ] 现实项目里壳必须是固定 QObject 类型（QML 注册约束），模板方法如何绕过？
       （候选思路：壳内持有一个 AbstractHandler，Handler 层面用模板方法）
2. [ ] 步骤之间的异步推进（回调/重试）在模板方法里怎么表达？
       （Strategy 版是 buildXxxPlan() 返回步骤列表 + 执行引擎；模板方法版可用状态机或协程式推进）
3. [ ] 钩子方法（hook）设计：哪些步骤是公共的、哪些是子类可覆盖的、哪些是抽象的
4. [ ] CommonInfo / StrategyCallbacks 这类共享结构是否直接复用

## 文件规划（暂未实现）

| 文件 | 职责 |
|------|------|
| `AbstractDeviceHandler.h` | 抽象基类：模板方法 + 抽象步骤 + 钩子 |
| `DeviceAHandler.h/.cpp` | 具体子类 A |
| `DeviceBHandler.h/.cpp` | 具体子类 B |
| `templatemethodwidget.h/.cpp` | 演示页（挂到 DesignPatternsWidget 的选项卡） |
