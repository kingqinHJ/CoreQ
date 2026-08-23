#pragma once

#include <QWidget>
#include <memory>

class QComboBox;
class QPushButton;
class QLabel;
class QProgressBar;
class QTextEdit;
class QFormLayout;
class QGroupBox;
class AbstractDeviceHandler;

class TemplateMethodWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TemplateMethodWidget(QWidget* parent = nullptr);

private slots:
    void onRefreshClicked();
    void onUploadClicked();
    void onResetClicked();
    void onSwitchDevice(int index);

private:
    void createHandler(int deviceType);
    void appendLog(const QString& msg);
    void setBusy(bool busy);
    void updateInfoPanels();
    void rebuildSpecificPanel();

    QComboBox* m_deviceCombo;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnUpload;
    QPushButton* m_btnReset;
    QLabel* m_lblId;
    QLabel* m_lblName;
    QLabel* m_lblVersion;
    QLabel* m_lblStatus;
    QProgressBar* m_progressBar;
    QGroupBox* m_groupSpecific;
    QFormLayout* m_specificForm;
    QTextEdit* m_log;

    std::unique_ptr<AbstractDeviceHandler> m_handler;
};
