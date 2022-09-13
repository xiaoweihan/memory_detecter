#include "logger.h"
#include <iostream>
CLogger::CLogger(const std::string &strLogConfigPath) : m_strLogConfigPath(strLogConfigPath)
{
}

CLogger::~CLogger()
{
    UnInitLogger();
}

log4cpp::Category &CLogger::GetConsoleLogger(void)
{
    return log4cpp::Category::getRoot();
}

log4cpp::Category &CLogger::GetFileLogger(void)
{
    return log4cpp::Category::getInstance("Memory_Leak_Monitor");
}

bool CLogger::InitLogger(void)
{
    using namespace std;
    try
    {
        log4cpp::PropertyConfigurator::configure(m_strLogConfigPath.c_str());
        return true;
    }
    catch (log4cpp::ConfigureFailure &f)
    {
        std::cerr << "configure problem " << f.what() << std::endl;
    }
    return false;
}

void CLogger::UnInitLogger(void)
{
    log4cpp::Category::shutdown();
}
