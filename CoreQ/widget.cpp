#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , navigationList(new QListWidget(this))
    , stackedWidget(new QStackedWidget(this))
    , mainLayout(new QHBoxLayout(this))
    , settingPage(new QSetting(this))
    , promisePage(new QPromise(this))
    , httpPage(new HttpWidget(this))
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
    setWindowTitle("Qt组件库");
    resize(1200, 800);

    // 设置导航列表的样式和固定宽度
    navigationList->setFixedWidth(200);
    navigationList->addItem("QSettings");
    navigationList->addItem("Promise示例");
    navigationList->addItem("HTTP示例");
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
    // 添加QSettings页面
    stackedWidget->addWidget(settingPage);
    // 添加Promise示例页面
    stackedWidget->addWidget(promisePage);
    // 添加HTTP示例页面
    stackedWidget->addWidget(httpPage);

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
