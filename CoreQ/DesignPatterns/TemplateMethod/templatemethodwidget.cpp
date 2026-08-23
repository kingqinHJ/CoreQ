#include "templatemethodwidget.h"
#include "AbstractDeviceHandler.h"
#include "DeviceAHandler.h"
#include "DeviceBHandler.h"
#include "CommonInfo.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFont>

TemplateMethodWidget::TemplateMethodWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // ===== 顶部工具栏 =====
    auto* toolbarLayout = new QHBoxLayout();
    m_deviceCombo = new QComboBox(this);
    m_deviceCombo->addItem(QStringLiteral("设备A(简易版)"));
    m_deviceCombo->addItem(QStringLiteral("设备B(高级版)"));

    m_btnRefresh = new QPushButton(QStringLiteral("刷新"), this);
    m_btnUpload = new QPushButton(QStringLiteral("上传固件"), this);
    m_btnReset = new QPushButton(QStringLiteral("重置"), this);

    toolbarLayout->addWidget(new QLabel(QStringLiteral("设备:"), this));
    toolbarLayout->addWidget(m_deviceCombo);
    toolbarLayout->addWidget(m_btnRefresh);
    toolbarLayout->addWidget(m_btnUpload);
    toolbarLayout->addWidget(m_btnReset);
    toolbarLayout->addStretch();
    mainLayout->addLayout(toolbarLayout);

    // ===== 信息面板区域 =====
    auto* infoLayout = new QHBoxLayout();

    // 公共信息组
    auto* groupCommon = new QGroupBox(QStringLiteral("公共信息 (CommonInfo)"), this);
    auto* commonForm = new QFormLayout(groupCommon);
    m_lblId = new QLabel(QStringLiteral("—"), this);
    m_lblName = new QLabel(QStringLiteral("—"), this);
    m_lblVersion = new QLabel(QStringLiteral("—"), this);
    m_lblStatus = new QLabel(QStringLiteral("未连接"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    commonForm->addRow(QStringLiteral("设备ID:"), m_lblId);
    commonForm->addRow(QStringLiteral("名称:"), m_lblName);
    commonForm->addRow(QStringLiteral("版本:"), m_lblVersion);
    commonForm->addRow(QStringLiteral("状态:"), m_lblStatus);
    commonForm->addRow(QStringLiteral("进度:"), m_progressBar);
    infoLayout->addWidget(groupCommon);

    // 设备特有信息组（动态填充）
    m_groupSpecific = new QGroupBox(QStringLiteral("设备特有信息"), this);
    m_specificForm = new QFormLayout(m_groupSpecific);
    auto* placeholder = new QLabel(QStringLiteral("请选择设备后执行操作"), this);
    placeholder->setStyleSheet(QStringLiteral("color: gray;"));
    m_specificForm->addRow(placeholder);
    infoLayout->addWidget(m_groupSpecific);

    mainLayout->addLayout(infoLayout);

    // ===== 日志区域 =====
    auto* logLabel = new QLabel(QStringLiteral("执行日志:"), this);
    mainLayout->addWidget(logLabel);
    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setFont(QFont(QStringLiteral("Consolas"), 9));
    m_log->setStyleSheet(QStringLiteral("QTextEdit { background: #1e1e1e; color: #d4d4d4; }"));
    mainLayout->addWidget(m_log, 1);

    // ===== 模式说明 =====
    auto* descLabel = new QLabel(
        QStringLiteral("模板方法模式: 基类定义固定流程骨架(步骤顺序)，子类覆盖个别步骤实现设备差异。\n"
                       "公共步骤(如checkConnection)由基类实现，纯虚步骤子类必须实现，钩子(hook)方法子类可选覆盖。"),
        this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QStringLiteral("color: #666; font-size: 11px; padding: 4px;"));
    mainLayout->addWidget(descLabel);

    // ===== 连接信号 =====
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TemplateMethodWidget::onSwitchDevice);
    connect(m_btnRefresh, &QPushButton::clicked, this, &TemplateMethodWidget::onRefreshClicked);
    connect(m_btnUpload, &QPushButton::clicked, this, &TemplateMethodWidget::onUploadClicked);
    connect(m_btnReset, &QPushButton::clicked, this, &TemplateMethodWidget::onResetClicked);

    // 初始创建设备A
    createHandler(0);
    updateInfoPanels();
}

void TemplateMethodWidget::createHandler(int deviceType)
{
    HandlerCallbacks cb;
    cb.context = this;
    cb.logMessage = [this](const QString& msg) { appendLog(msg); };
    cb.onDataChanged = [this]() { updateInfoPanels(); };
    cb.onFinished = [this](bool ok) {
        updateInfoPanels();
        appendLog(ok ? QStringLiteral("========== 流程完成（成功）==========")
                     : QStringLiteral("========== 流程失败 =========="));
        setBusy(false);
    };

    if (deviceType == 0) {
        m_handler = std::make_unique<DeviceAHandler>(std::move(cb));
    } else {
        m_handler = std::make_unique<DeviceBHandler>(std::move(cb));
    }

    appendLog(QStringLiteral("--- 已切换到: %1 ---").arg(m_handler->deviceName()));
    rebuildSpecificPanel();
}

void TemplateMethodWidget::onSwitchDevice(int index)
{
    if (m_handler && m_handler->isBusy()) {
        appendLog(QStringLiteral("⚠  流程执行中，无法切换设备"));
        m_deviceCombo->blockSignals(true);
        m_deviceCombo->setCurrentIndex(index == 0 ? 1 : 0);
        m_deviceCombo->blockSignals(false);
        return;
    }
    createHandler(index);
    updateInfoPanels();
}

void TemplateMethodWidget::onRefreshClicked()
{
    if (!m_handler) return;
    setBusy(true);
    m_log->clear();
    m_handler->startRefresh();
}

void TemplateMethodWidget::onUploadClicked()
{
    if (!m_handler) return;
    setBusy(true);
    m_log->clear();
    m_handler->startUpload();
}

void TemplateMethodWidget::onResetClicked()
{
    if (!m_handler) return;
    m_handler->reset();
    m_log->clear();
    appendLog(QStringLiteral("--- 已重置 ---"));
    updateInfoPanels();
}

void TemplateMethodWidget::appendLog(const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    m_log->append(QStringLiteral("[%1] %2").arg(timestamp, msg));
}

void TemplateMethodWidget::setBusy(bool busy)
{
    m_btnRefresh->setEnabled(!busy);
    m_btnUpload->setEnabled(!busy);
    m_deviceCombo->setEnabled(!busy);
}

void TemplateMethodWidget::updateInfoPanels()
{
    if (!m_handler) return;

    const CommonInfo& c = m_handler->commonInfo();
    m_lblId->setText(c.id.isEmpty() ? QStringLiteral("—") : c.id);
    m_lblName->setText(c.name.isEmpty() ? QStringLiteral("—") : c.name);
    m_lblVersion->setText(c.version.isEmpty() ? QStringLiteral("—") : c.version);
    m_lblStatus->setText(c.status.isEmpty() ? QStringLiteral("—") : c.status);
    m_progressBar->setValue(c.progress);

    rebuildSpecificPanel();
}

void TemplateMethodWidget::rebuildSpecificPanel()
{
    while (m_specificForm->count() > 0) {
        QLayoutItem* item = m_specificForm->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    if (!m_handler) {
        auto* placeholder = new QLabel(QStringLiteral("请选择设备"), this);
        placeholder->setStyleSheet(QStringLiteral("color: gray;"));
        m_specificForm->addRow(placeholder);
        return;
    }

    m_groupSpecific->setTitle(QStringLiteral("%1 - 特有信息").arg(m_handler->deviceName()));

    QVariantMap titles = m_handler->deviceSpecificTitles();
    QVariantMap info = m_handler->deviceSpecificInfo();

    if (titles.isEmpty()) {
        auto* placeholder = new QLabel(QStringLiteral("(无特有数据)"), this);
        placeholder->setStyleSheet(QStringLiteral("color: gray;"));
        m_specificForm->addRow(placeholder);
        return;
    }

    for (auto it = titles.constBegin(); it != titles.constEnd(); ++it) {
        QString key = it.key();
        QString labelText = it.value().toString() + QStringLiteral(":");
        QString valueText = info.value(key).toString();
        if (valueText.isEmpty()) valueText = QStringLiteral("—");
        m_specificForm->addRow(labelText, new QLabel(valueText, this));
    }
}
