#include <QtGlobal>
#include <QQueue>
#include <QRegExp>
#include "common/Utils.h"

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

#include "Logger.h"

#include <memory>
#include <QMap>
#include <QDir>

// #define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#if defined(Q_OS_ANDROID)
#include <spdlog/sinks/android_sink.h>
#endif

#define HISTORY_COUNT 200

namespace spdlog {
namespace sinks {

#if defined(Q_OS_WIN)

template<typename Mutex>
class windows_sink : public base_sink<Mutex>
{
public:
    // Formatting codes
    const string_view_t reset = "\033[m";
    const string_view_t bold = "\033[1m";
    const string_view_t dark = "\033[2m";
    const string_view_t underline = "\033[4m";
    const string_view_t blink = "\033[5m";
    const string_view_t reverse = "\033[7m";
    const string_view_t concealed = "\033[8m";
    const string_view_t clear_line = "\033[K";

    // Foreground colors
    const string_view_t black = "\033[30m";
    const string_view_t red = "\033[31m";
    const string_view_t green = "\033[32m";
    const string_view_t yellow = "\033[33m";
    const string_view_t blue = "\033[34m";
    const string_view_t magenta = "\033[35m";
    const string_view_t cyan = "\033[36m";
    const string_view_t white = "\033[37m";

    /// Background colors
    const string_view_t on_black = "\033[40m";
    const string_view_t on_red = "\033[41m";
    const string_view_t on_green = "\033[42m";
    const string_view_t on_yellow = "\033[43m";
    const string_view_t on_blue = "\033[44m";
    const string_view_t on_magenta = "\033[45m";
    const string_view_t on_cyan = "\033[46m";
    const string_view_t on_white = "\033[47m";

    /// Bold colors
    const string_view_t yellow_bold = "\033[33m\033[1m";
    const string_view_t red_bold = "\033[31m\033[1m";
    const string_view_t bold_on_red = "\033[1m\033[41m";

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        static std::array<std::string, level::n_levels> colors_ {
            std::string(white.data(), white.size()), // level::trace
            std::string(cyan.data(), cyan.size()), // level::debug
            std::string(green.data(), green.size()), // level::info
            std::string(yellow_bold.data(), yellow_bold.size()), // level::warn
            std::string(red_bold.data(), red_bold.size()), // level::err
            std::string(bold_on_red.data(), bold_on_red.size()), // level::critical
            std::string(reset.data(), reset.size()), // level::off
        };

        msg.color_range_start = 0;
        msg.color_range_end = 0;

        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        formatted.push_back('\0');

        QString tmp;
        auto print_range_ = [&tmp](const memory_buf_t &formatted, size_t start, size_t end){
            tmp.append(QByteArray(formatted.data() + start, static_cast<int>(end - start)));
        };

        auto print_ccode_ = [&tmp](const string_view_t &color_code){
            tmp.append(QByteArray(color_code.data(), color_code.size()));
        };

        if (msg.color_range_end > msg.color_range_start) {
            // before color range
            print_range_(formatted, 0, msg.color_range_start);
            // in color range
            print_ccode_(colors_[static_cast<size_t>(msg.level)]);
            print_range_(formatted, msg.color_range_start, msg.color_range_end);
            print_ccode_(reset);
            // after color range
            print_range_(formatted, msg.color_range_end, formatted.size());
        }
        else {
            print_range_(formatted, 0, formatted.size());
        }

        OutputDebugStringW(reinterpret_cast<const wchar_t *>(tmp.utf16()));
    }

    void flush_() override {}
};

using windows_sink_mt = windows_sink<std::mutex>;
using windows_sink_st = windows_sink<details::null_mutex>;

#endif

template<typename Mutex>
class proxy_sink : public base_sink<Mutex>
{
public:
    // Formatting codes
    const string_view_t reset = "\033[m";
    const string_view_t bold = "\033[1m";
    const string_view_t dark = "\033[2m";
    const string_view_t underline = "\033[4m";
    const string_view_t blink = "\033[5m";
    const string_view_t reverse = "\033[7m";
    const string_view_t concealed = "\033[8m";
    const string_view_t clear_line = "\033[K";

    // Foreground colors
    const string_view_t black = "\033[30m";
    const string_view_t red = "\033[31m";
    const string_view_t green = "\033[32m";
    const string_view_t yellow = "\033[33m";
    const string_view_t blue = "\033[34m";
    const string_view_t magenta = "\033[35m";
    const string_view_t cyan = "\033[36m";
    const string_view_t white = "\033[37m";

    /// Background colors
    const string_view_t on_black = "\033[40m";
    const string_view_t on_red = "\033[41m";
    const string_view_t on_green = "\033[42m";
    const string_view_t on_yellow = "\033[43m";
    const string_view_t on_blue = "\033[44m";
    const string_view_t on_magenta = "\033[45m";
    const string_view_t on_cyan = "\033[46m";
    const string_view_t on_white = "\033[47m";

    /// Bold colors
    const string_view_t yellow_bold = "\033[33m\033[1m";
    const string_view_t red_bold = "\033[31m\033[1m";
    const string_view_t bold_on_red = "\033[1m\033[41m";

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        static std::array<std::string, level::n_levels> colors_ {
            std::string(white.data(), white.size()), // level::trace
            std::string(cyan.data(), cyan.size()), // level::debug
            std::string(green.data(), green.size()), // level::info
            std::string(yellow_bold.data(), yellow_bold.size()), // level::warn
            std::string(red_bold.data(), red_bold.size()), // level::err
            std::string(bold_on_red.data(), bold_on_red.size()), // level::critical
            std::string(reset.data(), reset.size()), // level::off
        };

        if (s_logger) {
            msg.color_range_start = 0;
            msg.color_range_end = 0;

            memory_buf_t formatted;
            base_sink<Mutex>::formatter_->format(msg, formatted);
            formatted.push_back('\0');

            QString tmp;
            auto print_range_ = [&tmp](const memory_buf_t &formatted, size_t start, size_t end){
                tmp.append(QByteArray(formatted.data() + start, static_cast<int>(end - start)));
            };

            auto print_ccode_ = [&tmp](const string_view_t &color_code){
                tmp.append(QByteArray(color_code.data(), color_code.size()));
            };

            if (msg.color_range_end > msg.color_range_start) {
                // before color range
                print_range_(formatted, 0, msg.color_range_start);
                // in color range
                print_ccode_(colors_[static_cast<size_t>(msg.level)]);
                print_range_(formatted, msg.color_range_start, msg.color_range_end);
                print_ccode_(reset);
                // after color range
                print_range_(formatted, msg.color_range_end, formatted.size());
            }
            else {
                print_range_(formatted, 0, formatted.size());
            }

            s_logger->sendLog(tmp);
        }
    }
    void flush_() override {}
};

using proxy_sink_mt = proxy_sink<details::null_mutex>;
using proxy_sink_st = proxy_sink<details::null_mutex>;

}
}

class LoggerPrivate
{
public:
#if defined(Q_OS_WIN)
    std::shared_ptr<spdlog::sinks::windows_sink_mt> console_sink;
#elif defined(Q_OS_ANDROID)
    std::shared_ptr<spdlog::sinks::android_sink_mt> console_sink;
#else
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
#endif
    std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> rotating_file_sink;
    std::shared_ptr<spdlog::sinks::daily_file_sink_mt> daily_file_sink;
    std::shared_ptr<spdlog::sinks::proxy_sink_mt> proxy_sink;
    std::shared_ptr<spdlog::async_logger> logger;

    QQueue<QString> history_log_list;
    LogCallback log_cb;

    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

void LoggerPrivate::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    auto logger = spdlog::default_logger();
    if (!logger)
        return;

    auto tmp = msg;
    // tmp = tmp.remove(QRegExp("::<lambda_*>::operator ()",
    //                          Qt::CaseSensitive, QRegExp::Wildcard));

    static QMap<QtMsgType, spdlog::level::level_enum> type_map {
        { QtDebugMsg, spdlog::level::trace },
        { QtWarningMsg, spdlog::level::warn },
        { QtCriticalMsg, spdlog::level::critical },
        { QtFatalMsg, spdlog::level::err },
        { QtInfoMsg, spdlog::level::info },
    };

    logger->log(spdlog::source_loc{context.file, context.line, context.function},
                type_map.value(type, spdlog::level::trace),
                tmp.toStdString());
}


Logger *Logger::self = nullptr;
Logger::Logger(QObject *parent)
    : QObject(parent)
{
    d.reset(new LoggerPrivate);
    self = this;
}

Logger::~Logger()
{
    LOG_THIS();
    cleanup();
    d.reset();
    self = nullptr;
}

bool Logger::initialize(bool daily_file, QString log_path)
{
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_level(spdlog::level::trace);
    spdlog::init_thread_pool(8192, 1);

    std::vector<spdlog::sink_ptr> sinks;

    bool disable_console_log = qgetenv("disable_console_log") == "1";
    if (!disable_console_log) {
#if defined(Q_OS_WIN)
        d->console_sink = std::make_shared<spdlog::sinks::windows_sink_mt>();
#elif defined(Q_OS_ANDROID)
        d->console_sink = std::make_shared<spdlog::sinks::android_sink_mt>();
#else
        d->console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(spdlog::color_mode::always);
#endif
        d->console_sink->set_level(spdlog::level::trace);
        sinks.push_back(d->console_sink);
    }

    if (log_path.isEmpty())
        log_path = Utils::applicationDirPath() + "/cache/log";
    QDir().mkpath(log_path);
    std::string base_filename = QString("%1/%2.log").arg(log_path, Utils::applicationName()).toLocal8Bit().toStdString();

    // if (daily_file) {
    //     // NOTE: 每天00:00更新日志文件，最多保留最近7天的日志
    //     int rotation_hour = 0;
    //     int rotation_minute = 0;
    //     bool turncate = false;
    //     int max_files = 7;

    //     d->daily_file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
    //                 base_filename, rotation_hour, rotation_minute, turncate, max_files);
    //     d->daily_file_sink->set_level(spdlog::level::trace);
    //     sinks.push_back(d->daily_file_sink);
    // }
    // else {
    //     std::size_t max_size = 10*1024*1024;
    //     std::size_t max_files = 7;
    //     bool rotate_on_open = true;
    //     d->rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    //                 base_filename, max_size, max_files, rotate_on_open);
    //     d->rotating_file_sink->set_level(spdlog::level::trace);
    //     sinks.push_back(d->rotating_file_sink);
    // }

    d->proxy_sink = std::make_shared<spdlog::sinks::proxy_sink_mt>();
    d->proxy_sink->set_level(spdlog::level::trace);
    sinks.push_back(d->proxy_sink);

    d->logger = std::make_shared<spdlog::async_logger>("multi_logger",
                                                       sinks.begin(), sinks.end(),
                                                       spdlog::thread_pool(),
                                                       spdlog::async_overflow_policy::block);
    // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    d->logger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [T%t] [%L] %v%$");
    d->logger->set_level(spdlog::level::trace);
    spdlog::register_logger(d->logger);
    spdlog::set_default_logger(d->logger);

    qInstallMessageHandler(&LoggerPrivate::qtMessageHandler);

    LOGI("ensure path: %s", qUtf8Printable(log_path));
    return true;
}

void Logger::cleanup()
{
    qInstallMessageHandler(nullptr);

    d->logger->flush();

    if (d->console_sink) {
        d->console_sink->flush();
        d->console_sink.reset();
    }

    if (d->rotating_file_sink) {
        d->rotating_file_sink->flush();
        d->rotating_file_sink.reset();
    }

    if (d->daily_file_sink) {
        d->daily_file_sink->flush();
        d->daily_file_sink.reset();
    }

    if (d->proxy_sink) {
        d->proxy_sink->flush();
        d->proxy_sink.reset();
    }

    d->logger.reset();

    spdlog::shutdown();
}

void Logger::log(const char *file, int line, const char *function, int level, QString msg)
{
    log(file, line, function, level, msg.toStdString());
}

void Logger::log(const char *file, int line, const char *function, int level, std::string msg)
{
    auto logger = spdlog::default_logger();
    if (!logger)
        return;

    logger->log(spdlog::source_loc{file, line, function},
                (spdlog::level::level_enum)level, msg);
}

LogCallback Logger::logCallback() const
{
    return d->log_cb;
}

void Logger::setLogCallback(LogCallback cb)
{
    d->log_cb = cb;

    if (cb) {
        for (int i=0; i<d->history_log_list.size(); i++)
            cb(d->history_log_list[i]);
    }
}

void Logger::sendLog(const QString &str)
{
    if (d->log_cb) {
        d->log_cb(str);
    }
    else {
        d->history_log_list.enqueue(str);
        if (d->history_log_list.size() > HISTORY_COUNT)
            d->history_log_list.pop_front();
    }
}
