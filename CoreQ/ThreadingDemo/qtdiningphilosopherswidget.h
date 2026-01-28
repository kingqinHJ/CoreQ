#pragma once

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QLabel>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>

class QtDiningPhilosophersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtDiningPhilosophersWidget(QWidget *parent = nullptr);

private slots:
    void onStartClicked();
    void onStopClicked();

private:
    void setupUi();
    void logMessage(const QString &msg);

    QPushButton *m_btnStart;
    QPushButton *m_btnStop;
    QCheckBox *m_chkResolveDeadlock;
    
    QVector<QLabel*> m_philosophers; // 5个哲学家的UI
    QTextEdit *m_logViewer;
};
