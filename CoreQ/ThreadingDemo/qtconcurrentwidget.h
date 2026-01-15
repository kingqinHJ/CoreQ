#ifndef QTCONCURRENTWIDGET_H
#define QTCONCURRENTWIDGET_H

#include <QWidget>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QGroupBox>

// ==========================================
// QtConcurrent 演示窗口
// ==========================================
class QtConcurrentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtConcurrentWidget(QWidget *parent = nullptr);
    ~QtConcurrentWidget();

private slots:
    void runSimpleTask();
    void runTaskWithResult();
    void runTaskWithProgress();
    
    // FutureWatcher 槽函数
    void onTaskFinished();
    void onTaskResultReadyAt(int index);
    void onTaskProgressValueChanged(int value);
    void onTaskCanceled();

    void cancelTask();
    void clearLog();

private:
    void logMessage(const QString &msg);

private:
    // UI Controls
    QPushButton *m_btnSimpleRun;
    QPushButton *m_btnResultRun;
    QPushButton *m_btnProgressRun;
    QPushButton *m_btnCancel;
    QProgressBar *m_progressBar;
    QTextEdit *m_logDisplay;
    QLabel *m_statusLabel;

    // Watcher 用于监控异步任务
    // 注意：这里使用void类型作为通用演示，实际使用时应根据run的返回值指定类型
    QFutureWatcher<void> *m_voidWatcher;
    QFutureWatcher<int> *m_intWatcher; // 用于有返回值的任务
};

#endif // QTCONCURRENTWIDGET_H
