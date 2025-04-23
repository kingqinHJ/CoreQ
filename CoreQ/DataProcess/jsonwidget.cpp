#include "jsonwidget.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QDebug>

JsonWidget::JsonWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout(this))
    , generateJsonBtn(new QPushButton("生成JSON", this))
    , writeJsonFileBtn(new QPushButton("写入JSON文件", this))
    , resultLabel(new QLabel(this))
{
    setupUI();

    // 连接信号和槽
    connect(generateJsonBtn, &QPushButton::clicked, this, &JsonWidget::onGenerateJsonClicked);
    connect(writeJsonFileBtn, &QPushButton::clicked, this, &JsonWidget::onWriteJsonFileClicked);
}

JsonWidget::~JsonWidget()
{
}

void JsonWidget::setupUI()
{
    // 设置窗口标题
    setWindowTitle("JSON操作示例");

    // 添加标题标签
    QLabel *titleLabel = new QLabel("JSON操作演示", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // 设置结果标签
    resultLabel->setFrameShape(QFrame::Panel);
    resultLabel->setFrameShadow(QFrame::Sunken);
    resultLabel->setMinimumHeight(100);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setText("点击按钮执行JSON操作");

    // 添加到布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(generateJsonBtn);
    mainLayout->addWidget(writeJsonFileBtn);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void JsonWidget::onGenerateJsonClicked()
{
    // 创建JSON对象
    QJsonObject jsonObj;
    jsonObj["name"] = "示例名称";
    jsonObj["value"] = 100;
    jsonObj["enabled"] = true;

    // 转换为JSON文档
    QJsonDocument doc(jsonObj);
    QString jsonString = doc.toJson(QJsonDocument::Indented);

    // 显示生成的JSON
    resultLabel->setText(jsonString);
    qDebug() << "生成的JSON:" << jsonString;
}

void JsonWidget::onWriteJsonFileClicked()
{
    // 创建JSON对象
    QJsonObject jsonObj;
    jsonObj["name"] = "示例名称";
    jsonObj["value"] = 100;
    jsonObj["enabled"] = true;
    jsonObj["timestamp"] = QDateTime::currentDateTime().toString();

    // 转换为JSON文档
    QJsonDocument doc(jsonObj);
    QString jsonString = doc.toJson(QJsonDocument::Indented);

    // 获取保存文件路径
    QString filePath = QFileDialog::getSaveFileName(this, "保存JSON文件",
                                                    QDir::homePath(),
                                                    "JSON文件 (*.json)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(jsonString.toUtf8());
            file.close();
            resultLabel->setText("JSON文件已保存到:\n" + filePath);
            qDebug() << "JSON文件已保存到:" << filePath;
        } else {
            resultLabel->setText("保存文件失败: " + file.errorString());
            qDebug() << "保存文件失败:" << file.errorString();
        }
    }
}
