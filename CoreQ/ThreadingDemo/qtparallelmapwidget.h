#pragma once

#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

class QtParallelMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtParallelMapWidget(QWidget *parent = nullptr);

private slots:
    void onLoadData();
    void onProcessClicked();
    void onCancelClicked();

private:
    void setupUi();
    void logMessage(const QString &msg);

    QPushButton *m_btnLoadData;
    QPushButton *m_btnProcess;
    QPushButton *m_btnCancel;
    QProgressBar *m_progressBar;
    QListWidget *m_listResults;
    QTextEdit *m_logViewer;
};
