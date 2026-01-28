#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class QtReadersWritersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtReadersWritersWidget(QWidget *parent = nullptr);

private slots:
    void onStartClicked();
    void onStopClicked();

private:
    void setupUi();
    void logMessage(const QString &msg);

    QSpinBox *m_spinReaderCount;
    QSpinBox *m_spinWriterCount;
    QPushButton *m_btnStart;
    QPushButton *m_btnStop;
    
    QLabel *m_lblSharedData; // 显示当前的共享数据值
    QTextEdit *m_logViewer;
};
