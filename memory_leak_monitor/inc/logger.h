#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <string>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>
class CLogger
{
public:
    CLogger(const std::string &strLogConfigPath);
    ~CLogger();

    bool InitLogger(void);
    void UnInitLogger(void);

    log4cpp::Category &GetConsoleLogger(void);
    log4cpp::Category &GetFileLogger(void);

private:
    std::string m_strLogConfigPath;
};

#endif //__LOGGER_H__
