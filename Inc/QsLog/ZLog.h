#ifndef ZLOG_H
#define ZLOG_H

#include "QsLog.h"
#define LOG_PATH "./log/hcy_log.txt"
#define LOG_FILE_SIZE  512*1024
#define LOG_FILE_COUNT 5

#define ZLOG_TRACE() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream()
#define ZLOG_DEBUG() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream()
#define ZLOG_INFO()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()
#define ZLOG_WARN()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream()<<FILE_NAME<<LINE_COUNT
#define ZLOG_ERROR() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()<<FILE_NAME<<LINE_COUNT
#define ZLOG_FATAL() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream()<<FILE_NAME<<LINE_COUNT


#define FILE_NAME _FILE__
#define LINE_COUNT __LINE__

using namespace QsLogging;
class ZLog
{
public:
    static ZLog* instance;
    static ZLog* getInstance();
    static void setLoggingLevel(const Level& newLevel);
    static void setDestIsFile(const QString& path, const int& maxSize, const int& logCount);
    static void setDestIsCmd();
protected:
    ~ZLog();

private:
    ZLog();
    ZLog(const ZLog&){}
    ZLog& operator = (const ZLog&){
        return *this;
    }
private:
};

#endif // ZLOG_H
