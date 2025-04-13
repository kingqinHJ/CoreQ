#include "httpwidget.h"

HttpWidget::HttpWidget(QWidget *parent)
    : QWidget(parent)
    , httpManager(new HttpManager(this))
    , getUrlEdit(new QLineEdit(this))
    , postUrlEdit(new QLineEdit(this))
    , formUrlEdit(new QLineEdit(this))
    , getBtn(new QPushButton("GET请求", this))
    , postBtn(new QPushButton("POST请求", this))
    , formBtn(new QPushButton("表单测试", this))
    , mainLayout(new QVBoxLayout(this))
    , getResultEdit(new QTextEdit(this))
    , postResultEdit(new QTextEdit(this))
    , formResultEdit(new QTextEdit(this))
{
    setupUI();
    initConnections();
}

void HttpWidget::setupUI()
{
    // 设置URL输入框的占位符文本
    getUrlEdit->setPlaceholderText("输入GET请求URL地址");
    getUrlEdit->setText("http://httpbin.org/get?name=gongjianbo&age=27");
    postUrlEdit->setPlaceholderText("输入POST请求URL地址");
    postUrlEdit->setText("Mr QIN ");
    formUrlEdit->setPlaceholderText("输入表单测试URL地址");
    
    // 设置文本编辑框为只读
    getResultEdit->setReadOnly(true);
    postResultEdit->setReadOnly(true);
    formResultEdit->setReadOnly(true);
    
    // 设置最小高度
    getResultEdit->setMinimumHeight(100);
    postResultEdit->setMinimumHeight(100);
    formResultEdit->setMinimumHeight(100);
    
    // GET请求部分
    mainLayout->addWidget(getUrlEdit);
    mainLayout->addWidget(getBtn);
    mainLayout->addWidget(getResultEdit);
    
    // POST请求部分
    mainLayout->addWidget(postUrlEdit);
    mainLayout->addWidget(postBtn);
    mainLayout->addWidget(postResultEdit);
    
    // 表单测试部分
    //mainLayout->addWidget(formUrlEdit);
    mainLayout->addWidget(formBtn);
    mainLayout->addWidget(formResultEdit);
    
    mainLayout->addStretch();
    
    setLayout(mainLayout);
}

void HttpWidget::initConnections()
{
    // 按钮点击事件
    connect(getBtn, &QPushButton::clicked, [this]() {
        httpManager->get(getUrlEdit->text());
    });
    
    connect(postBtn, &QPushButton::clicked, [this]() {
        httpManager->post(postUrlEdit->text());
    });
    
    connect(formBtn, &QPushButton::clicked, [this]() {
        httpManager->formTest();
    });

    // 响应信号连接
    connect(httpManager, &HttpManager::getReplyReceived, getResultEdit, &QTextEdit::setText);
    connect(httpManager, &HttpManager::postReplyReceived, postResultEdit, &QTextEdit::setText);
    connect(httpManager, &HttpManager::formReplyReceived, formResultEdit, &QTextEdit::setText);
}
