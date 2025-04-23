#include "widget.h"
#include "./ui_widget.h"
#include "PageFactory.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , navigationList(new QListWidget(this))
    , stackedWidget(new QStackedWidget(this))
    , mainLayout(new QHBoxLayout(this))
{
    ui->setupUi(this);
    setupUI();
    createPages();
    initConnections();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupUI()
{
    // 设置窗口标题和大小
//    setWindowTitle("Qt组件库");
    setWindowTitle("Qt component");
    resize(1200, 800);

    // 设置导航列表的样式和固定宽度
    navigationList->setFixedWidth(200);
    navigationList->addItem("QSettings");
    navigationList->addItem("Promise示例");
    navigationList->addItem("HTTP示例");
    navigationList->addItem("callback");
    navigationList->addItem("JSON操作");
    // TODO: 添加更多功能项
    
    // 设置主布局
    mainLayout->addWidget(navigationList);
    mainLayout->addWidget(stackedWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    setLayout(mainLayout);
}

void Widget::createPages()
{
    QStringList pageTypes = {"Settings", "Promise", "Http", "Callback", "Json"};
    for (const QString &type : pageTypes) {
        QWidget *page = PageFactory::CreateWidget(type, this);
        if (page) {
            mpages[type] = page;
            stackedWidget->addWidget(page);
        }
    }
    
    // 设置初始页面为JSON页面
    navigationList->setCurrentRow(4);
}

void Widget::initConnections()
{
    connect(navigationList, &QListWidget::currentRowChanged,
            this, &Widget::onNavigationItemClicked);
}

void Widget::onNavigationItemClicked(int row)
{
    stackedWidget->setCurrentIndex(row);
}
