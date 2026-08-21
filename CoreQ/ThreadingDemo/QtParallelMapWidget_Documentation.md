# QtParallelMapWidget 并行计算功能说明

## 概述
QtParallelMapWidget 是一个演示 Qt Concurrent 并行 Map 操作的 GUI 组件，展示了如何使用 Qt 的并发框架进行高效的数据并行处理。

## 核心功能

### 1. 数据生成
- **生成模拟数据**: 自动生成两种类型的数据用于演示
  - 整数数据: 100个连续整数 (1-100)
  - 字符串数据: 50个格式化的字符串项

### 2. 并行处理
- **智能选择处理类型**: 随机选择处理整数或字符串数据
- **使用 QtConcurrent::mapped**: 自动利用多核CPU进行并行计算
- **进度监控**: 实时显示处理进度百分比
- **性能统计**: 记录处理耗时和处理项数

### 3. 用户交互
- **友好的GUI界面**: 清晰的操作按钮和状态显示
- **实时日志**: 详细的操作日志和时间戳
- **结果展示**: 显示处理结果的前10项
- **任务控制**: 支持取消正在进行的任务

## 技术实现

### 核心类设计
```cpp
class QtParallelMapWidget : public QWidget
{
    Q_OBJECT
    
private:
    // 数据容器
    QList<int> m_dataIntegers;      // 整数数据
    QStringList m_dataStrings;      // 字符串数据
    
    // UI控件
    QPushButton *m_btnLoadData;     // 生成数据按钮
    QPushButton *m_btnProcess;      // 处理按钮
    QPushButton *m_btnCancel;       // 取消按钮
    QProgressBar *m_progressBar;    // 进度条
    QListWidget *m_listResults;     // 结果列表
    QTextEdit *m_logViewer;         // 日志显示
    QLabel *m_statusLabel;          // 状态标签
    
    // 并发处理
    QFutureWatcher<int> *m_intWatcher;      // 整数处理监视器
    QFutureWatcher<QString> *m_stringWatcher; // 字符串处理监视器
    QElapsedTimer m_timer;          // 性能计时器
    bool m_isProcessing;            // 处理状态标志
};
```

### 关键处理函数

#### 整数处理函数
```cpp
static int processDataItem(const int &item)
{
    // 模拟耗时计算 (10ms/项)
    QThread::msleep(10);
    
    // 复杂的数学运算
    int result = item;
    for (int i = 0; i < 1000; ++i) {
        result = (result * 17 + 23) % 1000000;
    }
    return result;
}
```

#### 字符串处理函数
```cpp
static QString processStringItem(const QString &item)
{
    // 模拟字符串处理 (15ms/项)
    QThread::msleep(15);
    
    // 字符串变换
    QString result = item.toUpper();
    result.replace("_", "-");
    result.prepend("PROCESSED_");
    return result;
}
```

### 并发处理流程

1. **数据准备阶段**
   ```cpp
   void onLoadData() {
       // 生成100个整数和50个字符串
       for (int i = 1; i <= 100; ++i) m_dataIntegers.append(i);
       // ... 字符串生成代码
   }
   ```

2. **并行处理启动**
   ```cpp
   void onProcessClicked() {
       m_timer.start();  // 开始计时
       
       // 随机选择处理类型
       bool processIntegers = QRandomGenerator::global()->bounded(2) == 0;
       
       if (processIntegers) {
           QFuture<int> future = QtConcurrent::mapped(m_dataIntegers, processDataItem);
           m_intWatcher->setFuture(future);
       } else {
           QFuture<QString> future = QtConcurrent::mapped(m_dataStrings, processStringItem);
           m_stringWatcher->setFuture(future);
       }
   }
   ```

3. **进度监控**
   ```cpp
   void onProgressValueChanged(int value) {
       m_progressBar->setValue(value);
       if (value % 20 == 0 || value == 100) {
           logMessage(QString("处理进度: %1%").arg(value));
       }
   }
   ```

4. **结果处理**
   ```cpp
   void onProcessingFinished() {
       qint64 elapsed = m_timer.elapsed();
       
       if (sender() == m_intWatcher) {
           QFuture<int> future = m_intWatcher->future();
           logMessage(QString("整数处理完成! 耗时: %1ms, 处理项数: %2")
                      .arg(elapsed).arg(future.resultCount()));
           
           // 显示前10个结果
           QList<int> results = future.results();
           for (int i = 0; i < qMin(10, results.size()); ++i) {
               m_listResults->addItem(QString("结果[%1]: %2").arg(i+1).arg(results[i]));
           }
       }
   }
   ```

## 使用方法

### 基本操作步骤
1. 点击 **"1. 生成模拟数据"** 按钮
2. 点击 **"2. 并行处理 (Map)"** 按钮开始处理
3. 观察进度条和日志输出
4. 查看处理结果列表
5. 可随时点击 **"取消任务"** 停止处理

### 高级功能
- **清空结果**: 点击 "清空结果" 按钮清除所有显示内容
- **多次测试**: 可以重复生成数据和处理过程
- **性能对比**: 观察不同数据量和处理类型的性能差异

## 设计亮点

### 1. 线程安全性
- 使用 `QFutureWatcher` 安全地跨线程通信
- 所有UI更新都在主线程中进行
- 正确处理任务取消和资源清理

### 2. 用户体验优化
- 实时进度反馈
- 详细的日志记录
- 响应式的界面状态管理
- 自动滚动的日志显示

### 3. 代码质量
- 完善的错误处理
- 清晰的代码结构和注释
- 遵循Qt最佳实践
- 内存管理安全

## 性能特点

- **自动并行化**: QtConcurrent自动根据CPU核心数分配线程
- **负载均衡**: 框架自动平衡各线程的工作负载
- **内存效率**: 处理完的数据及时释放
- **可扩展性**: 易于添加新的处理函数和数据类型

## 学习价值

这个组件很好地演示了:
1. Qt Concurrent框架的基本使用
2. Future/Promise模式的实际应用
3. 多线程GUI应用程序的设计模式
4. 进度监控和用户体验优化
5. 资源管理和异常处理最佳实践