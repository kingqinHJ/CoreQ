#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

class QtPipelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtPipelineWidget(QWidget *parent = nullptr);

private slots:
    void onStartClicked();
    void onStopClicked();

private:
    void setupUi();
    void logMessage(const QString &msg);

    QPushButton *m_btnStart;
    QPushButton *m_btnStop;
    
    // 模拟三个阶段的状态显示
    QLabel *m_lblStage1; // 采集
    QLabel *m_lblStage2; // 处理
    QLabel *m_lblStage3; // 显示/存储
    
    QLabel *m_lblCount; // 总处理数
    QTextEdit *m_logViewer;
};
