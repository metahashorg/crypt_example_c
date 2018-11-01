#ifndef LOG_H_
#define LOG_H_

#include "log4cpp/Category.hh"
#include "log4cpp/Priority.hh"

#include <thread>

void configureLog(log4cpp::Priority::Value priority);

void configureSysLog(log4cpp::Priority::Value priority, const std::string& name, const std::string& syslogName);

void configureConsoleLog(log4cpp::Priority::Value priority);

struct Empty {
    template<typename T>
    Empty& operator <<(const T &) {
        return *this;
    }
};

extern log4cpp::Category& loggerRoot_;
extern log4cpp::Category& loggerRoot2_;

#define LOGERR loggerRoot_ << log4cpp::Priority::ERROR << std::this_thread::get_id() << " "
#define LOGWARN loggerRoot_ << log4cpp::Priority::WARN << std::this_thread::get_id() << " "
#define LOGINFO loggerRoot_ << log4cpp::Priority::INFO << std::this_thread::get_id() << " "
#define LOGDEBUG loggerRoot2_ << log4cpp::Priority::DEBUG << std::this_thread::get_id() << " "

#endif // LOG_H_