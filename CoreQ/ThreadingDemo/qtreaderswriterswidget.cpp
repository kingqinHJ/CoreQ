#include "qtreaderswriterswidget.h"
#include <QDateTime>

// ============================
// SharedDataStore 实现
// ============================

/*
 * 构造函数：初始化状态
 */
SharedDataStore::SharedDataStore(QObject* parent)
    : QObject(parent) {}

/*
 * 设置访问策略
 */
void SharedDataStore::setPolicy(AccessPolicy policy) {
  QMutexLocker locker(&gateMutex_);
  policy_ = policy;
  // 策略变更后，适当唤醒等待队列，避免长时间等待
  // 公平做法：唤醒一个写者，唤醒全部读者，让他们重新评估条件
  writersQueue_.wakeOne();
  readersQueue_.wakeAll();
}

/*
 * 读者开始访问：根据策略判断是否需要等待
 * - WriterPreference：若有写者等待或有活动写者，读者需要等待
 * - ReaderPreference：读者尽量快进入，只有活动写者时才等待
 * - Fair：基本公平，写者等待时优先安排写者
 */
void SharedDataStore::beginRead() {
  QMutexLocker locker(&gateMutex_);
  ++waitingReaders_;

  while (activeWriter_ ||
         (policy_ == AccessPolicy::WriterPreference && waitingWriters_ > 0)) {
    readersQueue_.wait(&gateMutex_);
  }

  --waitingReaders_;
  ++activeReaders_;
}

/*
 * 读者结束访问：如果没有读者了，唤醒一个写者
 */
void SharedDataStore::endRead() {
  QMutexLocker locker(&gateMutex_);
  --activeReaders_;
  if (activeReaders_ == 0) {
    // 读者全部退出后，优先让写者进行
    writersQueue_.wakeOne();
  }
}

/*
 * 写者开始访问：必须等待所有读者退出以及写者空闲
 */
void SharedDataStore::beginWrite() {
  QMutexLocker locker(&gateMutex_);
  ++waitingWriters_;

  while (activeWriter_ || activeReaders_ > 0) {
    writersQueue_.wait(&gateMutex_);
  }

  --waitingWriters_;
  activeWriter_ = true;
}

/*
 * 写者结束访问：根据策略唤醒后续访问者
 * - WriterPreference：若仍有写者在等，继续唤醒写者；否则唤醒所有读者
 * - ReaderPreference：优先唤醒所有读者；若无读者，则唤醒一个写者
 * - Fair：若有写者在等，唤醒一个写者；同时也唤醒读者，让他们重新竞争
 */
void SharedDataStore::endWrite() {
  QMutexLocker locker(&gateMutex_);
  activeWriter_ = false;

  switch (policy_) {
    case AccessPolicy::WriterPreference:
      if (waitingWriters_ > 0) writersQueue_.wakeOne();
      else readersQueue_.wakeAll();
      break;
    case AccessPolicy::ReaderPreference:
      if (waitingReaders_ > 0) readersQueue_.wakeAll();
      else if (waitingWriters_ > 0) writersQueue_.wakeOne();
      break;
    case AccessPolicy::Fair:
      if (waitingWriters_ > 0) writersQueue_.wakeOne();
      // 公平策略下也允许读者竞争进入
      readersQueue_.wakeAll();
      break;
  }
}

/*
 * 执行读取：需要持有读锁
 */
QString SharedDataStore::readValue() const {
  QReadLocker locker(&rwLock_);
  return payload_;
}

/*
 * 执行写入：需要持有写锁
 */
void SharedDataStore::writeValue(const QString& value) {
  QWriteLocker locker(&rwLock_);
  payload_ = value;
}

// ============================
// ReaderThread 实现
// ============================

/*
 * 构造函数：保存共享存储指针和配置
 */
ReaderThread::ReaderThread(SharedDataStore* store, int id, int intervalMs, QObject* parent)
    : QThread(parent), store_(store), id_(id), intervalMs_(intervalMs) {}

/*
 * 请求停止线程（软停止）
 */
void ReaderThread::stop() {
  running_ = false;
}

/*
 * 线程入口：周期性进行读操作，遵循访问控制策略
 */
void ReaderThread::run() {
  running_ = true;
  while (running_ && !isInterruptionRequested()) {
    msleep(intervalMs_);

    // 访问开始
    store_->beginRead();

    // 执行读取（持有读锁）
    const QString value = store_->readValue();
    emit store_->logMessage(
        QString("读者[%1] 读取到: %2").arg(id_).arg(value));

    // 访问结束
    store_->endRead();
  }
}

// ============================
// WriterThread 实现
// ============================

/*
 * 构造函数：保存共享存储指针和配置
 */
WriterThread::WriterThread(SharedDataStore* store, int id, int intervalMs, QObject* parent)
    : QThread(parent), store_(store), id_(id), intervalMs_(intervalMs) {}

/*
 * 请求停止线程（软停止）
 */
void WriterThread::stop() {
  running_ = false;
}

/*
 * 线程入口：周期性进行写操作，遵循访问控制策略
 */
void WriterThread::run() {
  running_ = true;
  while (running_ && !isInterruptionRequested()) {
    msleep(intervalMs_);

    // 访问开始
    store_->beginWrite();

    // 执行写入（持有写锁）
    const QString newValue =
        QString("写者[%1]@%2").arg(id_).arg(QDateTime::currentDateTime()
                                                    .toString("HH:mm:ss"));
    store_->writeValue(newValue);
    emit store_->logMessage(
        QString("写者[%1] 写入: %2").arg(id_).arg(newValue));

    // 访问结束
    store_->endWrite();
  }
}

// ============================
// QtReadersWritersWidget 实现
// ============================

/*
 * 构造函数：初始化UI
 */
QtReadersWritersWidget::QtReadersWritersWidget(QWidget* parent)
    : QWidget(parent) {
  setupUi();
}

/*
 * 析构：确保线程停止并释放资源
 */
QtReadersWritersWidget::~QtReadersWritersWidget() {
  onStopClicked();
}

/*
 * 初始化UI布局与控件
 */
void QtReadersWritersWidget::setupUi() {
  auto* mainLayout = new QVBoxLayout(this);

  // 控制区
  auto* grpControl = new QGroupBox("读写者控制", this);
  auto* ctlLayout = new QHBoxLayout(grpControl);

  // 读者数量
  ctlLayout->addWidget(new QLabel("读者数量:"));
  spinReaders_ = new QSpinBox(this);
  spinReaders_->setRange(1, 10);
  spinReaders_->setValue(3);
  ctlLayout->addWidget(spinReaders_);

  // 写者数量
  ctlLayout->addWidget(new QLabel("写者数量:"));
  spinWriters_ = new QSpinBox(this);
  spinWriters_->setRange(1, 10);
  spinWriters_->setValue(2);
  ctlLayout->addWidget(spinWriters_);

  // 读者延时
  ctlLayout->addWidget(new QLabel("读延时(ms):"));
  spinReaderDelay_ = new QSpinBox(this);
  spinReaderDelay_->setRange(10, 2000);
  spinReaderDelay_->setValue(400);
  ctlLayout->addWidget(spinReaderDelay_);

  // 写者延时
  ctlLayout->addWidget(new QLabel("写延时(ms):"));
  spinWriterDelay_ = new QSpinBox(this);
  spinWriterDelay_->setRange(10, 2000);
  spinWriterDelay_->setValue(700);
  ctlLayout->addWidget(spinWriterDelay_);

  // 策略选择
  ctlLayout->addWidget(new QLabel("访问策略:"));
  comboPolicy_ = new QComboBox(this);
  comboPolicy_->addItem("读者优先", static_cast<int>(AccessPolicy::ReaderPreference));
  comboPolicy_->addItem("写者优先", static_cast<int>(AccessPolicy::WriterPreference));
  comboPolicy_->addItem("公平", static_cast<int>(AccessPolicy::Fair));
  ctlLayout->addWidget(comboPolicy_);

  // 按钮
  btnStart_ = new QPushButton("开始", this);
  btnStop_ = new QPushButton("停止", this);
  btnStop_->setEnabled(false);
  btnClearLog_ = new QPushButton("清空日志", this);
  ctlLayout->addWidget(btnStart_);
  ctlLayout->addWidget(btnStop_);
  ctlLayout->addWidget(btnClearLog_);

  mainLayout->addWidget(grpControl);

  // 日志区
  auto* grpLog = new QGroupBox("运行日志", this);
  auto* logLayout = new QVBoxLayout(grpLog);
  logViewer_ = new QTextEdit(this);
  logViewer_->setReadOnly(true);
  logLayout->addWidget(logViewer_);
  mainLayout->addWidget(grpLog);

  // 连接
  connect(btnStart_, &QPushButton::clicked, this, &QtReadersWritersWidget::onStartClicked);
  connect(btnStop_, &QPushButton::clicked, this, &QtReadersWritersWidget::onStopClicked);
  connect(btnClearLog_, &QPushButton::clicked, this, &QtReadersWritersWidget::onClearLogClicked);
}

/*
 * 点击开始：创建存储与线程，连接信号
 */
void QtReadersWritersWidget::onStartClicked() {
  appendLog(">>> 正在启动读写者系统...");
  btnStart_->setEnabled(false);
  btnStop_->setEnabled(true);

  // 创建共享存储
  if (store_) delete store_;
  store_ = new SharedDataStore(this);
  connect(store_, &SharedDataStore::logMessage, this, &QtReadersWritersWidget::appendLog);

  // 设置策略
  const auto policy = static_cast<AccessPolicy>(comboPolicy_->currentData().toInt());
  store_->setPolicy(policy);

  // 启动读者线程
  const int readerCount = spinReaders_->value();
  const int readerDelay = spinReaderDelay_->value();
  readers_.clear();
  for (int i = 1; i <= readerCount; ++i) {
    auto* th = new ReaderThread(store_, i, readerDelay, this);
    readers_.push_back(th);
    connect(th, &QThread::finished, this, &QtReadersWritersWidget::onThreadFinished);
    th->start();
  }

  // 启动写者线程
  const int writerCount = spinWriters_->value();
  const int writerDelay = spinWriterDelay_->value();
  writers_.clear();
  for (int i = 1; i <= writerCount; ++i) {
    auto* th = new WriterThread(store_, i, writerDelay, this);
    writers_.push_back(th);
    connect(th, &QThread::finished, this, &QtReadersWritersWidget::onThreadFinished);
    th->start();
  }

  appendLog(">>> 系统已启动");
}

/*
 * 点击停止：软停止，不阻塞主线程；线程结束后通过 finished 信号收尾
 */
void QtReadersWritersWidget::onStopClicked() {
  appendLog("<<< 正在停止系统...");
  btnStart_->setEnabled(false);
  btnStop_->setEnabled(false);

  // 停止读者
  for (auto* th : readers_) {
    if (!th) continue;
    th->stop();
    th->requestInterruption();
  }

  // 停止写者
  for (auto* th : writers_) {
    if (!th) continue;
    th->stop();
    th->requestInterruption();
  }

  // 注意：不在主线程 wait()，等待 finished 回调做清理与恢复UI
}

/*
 * 线程结束回调：置空指针，清理资源，恢复UI
 */
void QtReadersWritersWidget::onThreadFinished() {
  auto* th = qobject_cast<QThread*>(sender());
  if (!th) return;
  th->deleteLater();

  // 从列表中移除
  for (int i = 0; i < readers_.size(); ++i) {
    if (readers_[i] == th) readers_[i] = nullptr;
  }
  for (int i = 0; i < writers_.size(); ++i) {
    if (writers_[i] == th) writers_[i] = nullptr;
  }

  // 判断是否所有线程均已退出
  bool anyAlive = false;
  for (auto* r : readers_) anyAlive = anyAlive || (r != nullptr);
  for (auto* w : writers_) anyAlive = anyAlive || (w != nullptr);

  if (!anyAlive) {
    readers_.clear();
    writers_.clear();
    if (store_) {
      delete store_;
      store_ = nullptr;
    }
    btnStart_->setEnabled(true);
    appendLog("<<< 系统已安全停止");
  }
}

/*
 * 清空日志
 */
void QtReadersWritersWidget::onClearLogClicked() {
  logViewer_->clear();
}

/*
 * 追加日志到文本框（带时间戳）
 */
void QtReadersWritersWidget::appendLog(const QString& msg) {
  const auto ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
  logViewer_->append(QString("[%1] %2").arg(ts, msg));
}
