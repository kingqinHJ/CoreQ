#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

#define LEVEL_TRACE 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO 2
#define LEVEL_WARN 3
#define LEVEL_ERROR 4
#define LEVEL_CRITICAL 5

#define s_logger Logger::instance()

#define log_trace(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_TRACE, msg)
#define log_debug(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_DEBUG, msg)
#define log_info(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_INFO, msg)
#define log_warn(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_WARN, msg)
#define log_error(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_ERROR, msg)
#define log_critical(msg) s_logger->log(__FILE__, __LINE__, __FUNCTION__, LEVEL_CRITICAL, msg)

using LogCallback = std::function<void(const QString &str)>;

class LoggerPrivate;
class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    static Logger *instance() { return self; }

    bool initialize(bool daily_file = true, QString log_path = QString());
    void cleanup();

    void log(const char *file, int line, const char *function,
             int level, QString msg);

    void log(const char *file, int line, const char *function,
             int level, std::string msg);

    LogCallback logCallback() const;
    void setLogCallback(LogCallback cb);

    void sendLog(const QString &str);

private:
    static Logger *self;
    std::shared_ptr<LoggerPrivate> d;
};

#endif // LOGGER_H
