#include "utility.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>

void Utility::CovertCallStackToString(LP_CALL_STACK_ELEMENT pCallStackArray,size_t uArraySize,std::vector<std::string>& CallStackVec)
{

    CallStackVec.clear();
    for (size_t i = 0; i < uArraySize; ++i)
    {
        char szBuffer[1024] = {0};
        snprintf(szBuffer,sizeof(szBuffer),"%p : (%s+0x%x) [%p]", pCallStackArray[i].uPC,pCallStackArray[i].szFrameName ,pCallStackArray[i].uOffset, pCallStackArray[i].uPC);
        CallStackVec.emplace_back(szBuffer);
    }
}

std::string Utility::ConvertEventToString(MEMORY_EVENT EventType)
{
    std::string strContent;
    switch (EventType)
    {
    case MEMORY_EVENT::MALLOC_EVENT:
        strContent = "malloc";
        break;
    case MEMORY_EVENT::CALLOC_EVENT:
        strContent = "calloc";
        break;
    case MEMORY_EVENT::FREE_EVENT:
        strContent = "free";
        break;
    case MEMORY_EVENT::REALLOC_EVENT:
        strContent = "realloc";
        break;
    default:
        strContent = "unknown";
        break;
    }
    return strContent;
}

std::string Utility::ConvertTimeToString(const time_t* pCovertTime)
{
    if (nullptr == pCovertTime)
    {
        return std::string("");
    }

    tm Result;
    if (NULL == localtime_r(pCovertTime, &Result))
    {
        return std::string("");
    }

    char szTime[100] = {0};

    snprintf(szTime,sizeof(szTime),"%04d-%02d-%02d %02d:%02d:%02d", Result.tm_year + 1900, Result.tm_mon + 1,
        Result.tm_mday, Result.tm_hour, Result.tm_min, Result.tm_sec);

    return std::string(szTime);

}


