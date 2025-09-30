#ifndef THREADINGDEMOWIDGET_H
#define THREADINGDEMOWIDGET_H

#include <QtWidgets>
#include "qpromise.h"
#include "stdthreadwidget.h"

/**
 * @brief ThreadingDemoWidget类 - C++多线程技术演示主界面
 * 
 * 这个类作为多线程演示的主入口界面，集成了各种线程技术的演示：
 * 
 * 功能特性：
 * - 集成std::promise/future演示
 * - 提供清晰的导航界面
 * - 支持扩展更多线程技术演示
 * - 统一的界面风格和交互体验
 * 
 * 设计模式：
 * - 使用QStackedWidget管理多个演示界面
 * - 左侧导航列表，右侧内容展示
 * - 模块化设计，便于添加新的演示
 */
class ThreadingDemoWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit ThreadingDemoWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ThreadingDemoWidget();

private slots:
    /**
     * @brief 导航列表选择变化槽函数
     * @param current 当前选中项
     * @param previous 之前选中项
     */
    void onNavigationSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);

    /**
     * @brief 打开Promise/Future演示
     */
    void openPromiseDemo();

private:
    /**
     * @brief 设置用户界面
     * 创建主布局和所有UI控件
     */
    void setupUI();

    /**
     * @brief 初始化导航列表
     * 添加各种线程技术演示项目
     */
    void initNavigationList();

    /**
     * @brief 初始化信号槽连接
     */
    void initConnections();

    /**
     * @brief 创建演示页面
     * 创建并配置各个演示界面
     */
    void createDemoPages();

    // UI控件
    QHBoxLayout *mainLayout;        ///< 主水平布局
    QVBoxLayout *leftLayout;        ///< 左侧垂直布局
    
    QLabel *titleLabel;             ///< 标题标签
    QListWidget *navigationList;    ///< 左侧导航列表
    QStackedWidget *contentStack;   ///< 右侧内容堆栈
    
    // 演示页面
    QPromise *promiseDemo;          ///< Promise/Future演示页面
    StdThreadWidget *threadDemo;  ///< std::thread演示页面
    QWidget *welcomePage;           ///< 欢迎页面
    
    // 样式和布局
    QSplitter *splitter;            ///< 分割器，调整左右比例
};

#endif // THREADINGDEMOWIDGET_H