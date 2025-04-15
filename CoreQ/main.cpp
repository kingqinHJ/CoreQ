#include "widget.h"

#include <QApplication>
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;

// 将Qt的消息处理转发到spdlog
void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 获取文件名和行号信息
    const char* file = context.file ? context.file : "";
    int line = context.line;

    // 构建日志内容，添加文件和行号信息
    QString logMessage;
    if (line > 0) {
        logMessage = QString("%1 (%2:%3)").arg(msg, file, QString::number(line));
    } else {
        logMessage = msg;
    }

    // 转发到spdlog的对应日志级别
    switch (type) {
    case QtDebugMsg:
        spdlog::debug(logMessage.toStdString());
        break;
    case QtInfoMsg:
        spdlog::info(logMessage.toStdString());
        break;
    case QtWarningMsg:
        spdlog::warn(logMessage.toStdString());
        break;
    case QtCriticalMsg:
        spdlog::error(logMessage.toStdString());
        break;
    case QtFatalMsg:
        spdlog::critical(logMessage.toStdString());
        abort(); // 致命错误时终止程序
    }
}

// 初始化日志系统
void initLogger()
{
    try {
        // 确保日志目录存在
        QDir logDir;
        if (!logDir.exists("logs")) {
            logDir.mkpath("logs");
        }

        // 创建一个控制台彩色输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // 创建文件轮转输出（1MB大小，3个备份文件）
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/application.log", 1024 * 1024, 3, false);

        // 设置日志格式
        std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] %v";
        console_sink->set_pattern(pattern);
        file_sink->set_pattern(pattern);

        // 创建多输出组合记录器
        auto logger = std::make_shared<spdlog::logger>("qt_logger",
                                                       spdlog::sinks_init_list{console_sink, file_sink});

        // 设置全局日志级别
        logger->set_level(spdlog::level::debug);

        // 设置为默认记录器
        spdlog::set_default_logger(logger);

        // 设置Qt消息处理器
        qInstallMessageHandler(qtMessageHandler);

        spdlog::info("完成日志配置");
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化日志系统
    initLogger();

    Widget w;
    w.show();
    return a.exec();
}
