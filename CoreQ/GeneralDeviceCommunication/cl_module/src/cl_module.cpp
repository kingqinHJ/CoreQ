#include "cl_module.h"
#include <string>
#include <QUuid>
#include <mutex>

std::string methodName(const char *msg)
{
#ifndef NO_LOG_OUTPUT
    // int tst_rknn(int, char**)
    // tst_rknn(int, char**)::<lambda()>
    // virtual Logger::~Logger()
    // Logger::~Logger()::<lambda()>
#ifdef Q_OS_LINUX

    std::string prettyFunction(msg);
    size_t pos = prettyFunction.find("(");
    if (pos != std::string::npos)
        prettyFunction = prettyFunction.substr(0, pos);

    pos = prettyFunction.rfind(" ");
    if (pos != std::string::npos)
        prettyFunction = prettyFunction.substr(pos+1);

    return prettyFunction;
#else
    // __cdecl HttpServicePrivate::handleTask
    // void __cdecl HttpHelper::post::<lambda_1e011a5f7caf73830c92c786dc6bcf3b>::operator
    // __cdecl fs::DeviceEnginePrivate::onDeviceOnlineStateChanged(struct fs::Device,bool)
    // auto __cdecl FSP2PService::start(int,const class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > &) const

    std::string prettyFunction(msg);
    size_t pos = prettyFunction.find("::<lambda");
    if (pos != std::string::npos)
        prettyFunction = prettyFunction.substr(0, pos);

    pos = prettyFunction.find("(");
    if (pos != std::string::npos)
        prettyFunction = prettyFunction.substr(0, pos);

    pos = prettyFunction.rfind(" ");
    if (pos != std::string::npos)
        prettyFunction = prettyFunction.substr(pos+1);

    return prettyFunction;

#endif

#else
    return std::string();
#endif
}

QString getBuildDateTime()
{
    QString date = QLocale::c().toDate(QString(__DATE__).replace("  ", " "), "MMM d yyyy").toString("yyyy-MM-dd");
    return QString::asprintf("%s %s", qUtf8Printable(date), __TIME__);
}
