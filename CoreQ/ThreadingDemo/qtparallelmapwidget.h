#ifndef QTPARALLELMAPWIDGET_H
#define QTPARALLELMAPWIDGET_H

#include <QWidget>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QThread>
#include <QElapsedTimer>

// ==========================================
// QtParallelMapWidget - 并行Map操作演示
// ==========================================
class QtParallelMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QtParallelMapWidget(QWidget *parent = nullptr);
    ~QtParallelMapWidget();

private slots:
    void onLoadData();
    void onProcessClicked();
    void onCancelClicked();
    
    // FutureWatcher 槽函数
    void onProcessingFinished();
    void onProgressValueChanged(int value);
    void onResultReadyAt(int index);
    void onProcessingCanceled();
    
    void clearResults();

private:
    void setupUi();
    void logMessage(const QString &msg);
    void cleanupProcessingState();
    
    // 模拟数据处理函数
    static int processDataItem(const int &item);
    static QString processStringItem(const QString &item);
    
    // 数据容器
    QList<int> m_dataIntegers;
    QStringList m_dataStrings;
    
    // UI控件
    QPushButton *m_btnLoadData;
    QPushButton *m_btnProcess;
    QPushButton *m_btnCancel;
    QPushButton *m_btnClear;
    QProgressBar *m_progressBar;
    QListWidget *m_listResults;
    QTextEdit *m_logViewer;
    QLabel *m_statusLabel;
    
    // 并行处理相关
    QFutureWatcher<int> *m_intWatcher;
    QFutureWatcher<QString> *m_stringWatcher;
    QElapsedTimer m_timer;
    bool m_isProcessing;
};

#endif // QTPARALLELMAPWIDGET_H
