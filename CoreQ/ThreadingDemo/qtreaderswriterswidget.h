#pragma once

#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QVector>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

/*
 * 访问策略枚举：
 * - ReaderPreference: 读者优先（可能导致写者饥饿）
 * - WriterPreference: 写者优先（减少写者饥饿，读者可能等待）
 * - Fair: 公平策略（尽量轮流，降低饥饿可能）
 */
enum class AccessPolicy {
  ReaderPreference,
  WriterPreference,
  Fair
};

/*
 * 共享数据存储（读写者问题核心）
 * 负责：
 * - 用条件变量和互斥锁实现访问控制（公平/优先策略）
 * - 用 QReadWriteLock 保护真实数据读写
 * - 提供读/写接口给线程调用
 */
class SharedDataStore : public QObject {
  Q_OBJECT
 public:
  explicit SharedDataStore(QObject* parent = nullptr);

  // 设置访问策略
  void setPolicy(AccessPolicy policy);

  // 读者开始访问（可能阻塞等待）
  void beginRead();

  // 读者结束访问（唤醒等待的写者或读者）
  void endRead();

  // 写者开始访问（可能阻塞等待）
  void beginWrite();

  // 写者结束访问（唤醒等待的写者或读者）
  void endWrite();

  // 执行实际的读取操作（必须在 beginRead/endRead 内调用）
  QString readValue() const;

  // 执行实际的写入操作（必须在 beginWrite/endWrite 内调用）
  void writeValue(const QString& value);

 signals:
  // 输出日志到UI
  void logMessage(const QString& msg);

 private:
  // 访问策略
  AccessPolicy policy_ = AccessPolicy::Fair;

  // 条件变量与互斥锁，用于公平/优先控制
  mutable QMutex gateMutex_;
  QWaitCondition readersQueue_;
  QWaitCondition writersQueue_;
  int activeReaders_ = 0;
  int waitingReaders_ = 0;
  int waitingWriters_ = 0;
  bool activeWriter_ = false;

  // 真实数据锁与数据
  mutable QReadWriteLock rwLock_;
  QString payload_ = "初始数据";
};

/*
 * 读者线程
 * 循环尝试读取数据，遵循访问控制策略，输出日志
 */
class ReaderThread : public QThread {
  Q_OBJECT
 public:
  ReaderThread(SharedDataStore* store, int id, int intervalMs, QObject* parent = nullptr);

  // 停止线程（软停止，配合 isInterruptionRequested）
  void stop();

 protected:
  // 线程入口：周期性进行读操作
  void run() override;

 private:
  SharedDataStore* store_ = nullptr;
  int id_ = 0;
  int intervalMs_ = 500;
  bool running_ = true;
};

/*
 * 写者线程
 * 循环尝试写入数据，遵循访问控制策略，输出日志
 */
class WriterThread : public QThread {
  Q_OBJECT
 public:
  WriterThread(SharedDataStore* store, int id, int intervalMs, QObject* parent = nullptr);

  // 停止线程（软停止，配合 isInterruptionRequested）
  void stop();

 protected:
  // 线程入口：周期性进行写操作
  void run() override;

 private:
  SharedDataStore* store_ = nullptr;
  int id_ = 0;
  int intervalMs_ = 800;
  bool running_ = true;
};

/*
 * QtReadersWritersWidget：读写者模型的UI容器
 * 提供：
 * - 配置读者/写者数量与速率
 * - 切换访问策略
 * - 启动/停止系统
 * - 查看运行日志
 */
class QtReadersWritersWidget : public QWidget {
  Q_OBJECT
 public:
  explicit QtReadersWritersWidget(QWidget* parent = nullptr);
  ~QtReadersWritersWidget();

 private slots:
  // 点击“开始”按钮
  void onStartClicked();

  // 点击“停止”按钮
  void onStopClicked();

  // 清空日志
  void onClearLogClicked();

  // 线程结束回调：恢复UI与资源清理
  void onThreadFinished();

  // 追加日志到文本框
  void appendLog(const QString& msg);

 private:
  // 初始化UI
  void setupUi();

  // UI控件
  QSpinBox* spinReaders_ = nullptr;
  QSpinBox* spinWriters_ = nullptr;
  QSpinBox* spinReaderDelay_ = nullptr;
  QSpinBox* spinWriterDelay_ = nullptr;
  QComboBox* comboPolicy_ = nullptr;

  QPushButton* btnStart_ = nullptr;
  QPushButton* btnStop_ = nullptr;
  QPushButton* btnClearLog_ = nullptr;

  QTextEdit* logViewer_ = nullptr;

  // 核心对象
  SharedDataStore* store_ = nullptr;
  QVector<ReaderThread*> readers_;
  QVector<WriterThread*> writers_;
};
