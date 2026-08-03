#ifndef CL_MODULE_H
#define CL_MODULE_H
#ifdef __cplusplus

#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QLoggingCategory>

//#define NO_LOG_OUTPUT
#ifdef NO_LOG_OUTPUT

#  undef qDebug
#  define qDebug QT_NO_QDEBUG_MACRO
#  undef qInfo
#  define qInfo QT_NO_QDEBUG_MACRO
#  undef qWarning
#  define qWarning QT_NO_QDEBUG_MACRO
#  undef qCritical
#  define qCritical QT_NO_QDEBUG_MACRO
#  undef qFatal
#  define qFatal QT_NO_QDEBUG_MACRO

#endif // NO_LOG_OUTPUT

#ifdef __GNUC__
#define __FS_METHOD_NAME__ methodName(__PRETTY_FUNCTION__).c_str()
#else
#define __FS_METHOD_NAME__ methodName(__FUNCSIG__).c_str()
#endif // __GNUC__

#define LOG_THIS() qDebug("[%s]", __FS_METHOD_NAME__)
#define CLOG_THIS(category) qCDebug(category, "[%s]", __FS_METHOD_NAME__)

// GCC 要求变参宏至少包含一个参数，而 MSVC 允许变参宏不包含参数
// #__VA_ARGS__
// ## 运算符的作用是当 __VA_ARGS__ 是空的时候，去掉前面的逗号。这样就可以在 GCC 下正常工作
#define LOGD(format, ...) qDebug().noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGI(format, ...) qInfo().noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGW(format, ...) qWarning().noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGC(format, ...) qCritical().noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)

#define CLOGD(category, format, ...) qCDebug(category).noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define CLOGI(category, format, ...) qCInfo(category).noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define CLOGW(category, format, ...) qCWarning(category).noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)
#define CLOGC(category, format, ...) qCCritical(category).noquote() << QString::asprintf("[%s] - " format, __FS_METHOD_NAME__, ##__VA_ARGS__)

#define LOGTD(tag, format, ...) qDebug().noquote() << QString::asprintf("[%s] [%s] - " format, QVariant(tag).toString().toUtf8().data(), __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGTI(tag, format, ...) qInfo().noquote() << QString::asprintf("[%s] [%s] - " format, QVariant(tag).toString().toUtf8().data(), __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGTW(tag, format, ...) qWarning().noquote() << QString::asprintf("[%s] [%s] - " format, QVariant(tag).toString().toUtf8().data(), __FS_METHOD_NAME__, ##__VA_ARGS__)
#define LOGTC(tag, format, ...) qCritical().noquote() << QString::asprintf("[%s] [%s] - " format, QVariant(tag).toString().toUtf8().data(), __FS_METHOD_NAME__, ##__VA_ARGS__)


#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define SCOPE_EXIT(code) \
    auto CONCAT(_scope_exit_, __LINE__) = \
        std::shared_ptr<void>(nullptr, [&](void*) { code; })

class ScopeBenchmark
{
public:
    ScopeBenchmark(const std::string &tag, const std::string &function)
        : tag(tag), function(function) {
        indent = ++getIndent();
        timer.start();
        qDebug().noquote() << QString::asprintf("[%32.32s] - %s%s begin", function.c_str(), prefix().c_str(), tag.c_str());
    }
    ~ScopeBenchmark() {
        qDebug().noquote() << QString::asprintf("[%32.32s] - %s%s end： %lldms", function.c_str(), prefix().c_str(), tag.c_str(), timer.elapsed());
        --getIndent();
    }

private:
    std::string tag;
    std::string function;
    QElapsedTimer timer;
    int indent;

private:
    static int &getIndent() {
        static thread_local int level = 0;
        return level;
    }
    std::string prefix() {
        return std::string(indent * 2, ' ');
    }
};

#define SCOPE_BENCHMARK(tag) \
    ScopeBenchmark scope_##tag(#tag, __FS_METHOD_NAME__);

#define DATETIME_FORMAT         "yyyy-MM-dd hh:mm:ss"
#define DATE_FORMAT             "yyyy-MM-dd"
#define TIME_FORMAT             "hh:mm:ss"

///////////////////////////////////////////////////////////////////////////////

std::string methodName(const char *str);
QString getBuildDateTime();

#endif // __cplusplus
#endif // CL_MODULE_H
