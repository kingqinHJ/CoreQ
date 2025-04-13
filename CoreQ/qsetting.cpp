#include<QDir>
#include<QDebug>
#include<QSettings>
#include<QCoreApplication>
#include <QMessageBox>
#include "qsetting.h"

QSetting::QSetting(QWidget *parent)
    : QWidget(parent)
    , settings(new QSettings("MyComponent", "CoreQ", this))
    , usernameEdit(new QLineEdit(this))
    , emailEdit(new QLineEdit(this))
    , saveButton(new QPushButton("保存设置", this))
    , loadButton(new QPushButton("加载设置", this))
    , mainLayout(new QVBoxLayout(this))
    , formLayout(new QFormLayout)
{
    setupUI();
    initConnections();
    loadSettings(); // 初始化时加载已保存的设置
}

void QSetting::setupUI()
{
    // 设置表单布局
    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("邮箱:", emailEdit);

    // 设置主布局
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(saveButton);
    mainLayout->addWidget(loadButton);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void QSetting::initConnections()
{
    connect(saveButton, &QPushButton::clicked, this, &QSetting::saveSettings);
    connect(loadButton, &QPushButton::clicked, this, &QSetting::loadSettings);
}

void QSetting::saveSettings()
{
    settings->setValue("username", usernameEdit->text());
    settings->setValue("email", emailEdit->text());
    settings->sync();

    QMessageBox::information(this, "保存成功", "设置已成功保存！");
}

void QSetting::loadSettings()
{
    usernameEdit->setText(settings->value("username").toString());
    emailEdit->setText(settings->value("email").toString());
}
