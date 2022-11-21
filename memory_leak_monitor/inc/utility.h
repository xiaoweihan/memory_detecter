#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <time.h>
#include <string>
#include <vector>
#include "Type.h"
namespace Utility
{

    // 时间转换为字符串
    std::string ConvertTimeToString(const time_t *pCovertTime);

    std::string ConvertEventToString(MEMORY_EVENT EventType);

    void CovertCallStackToString(LP_CALL_STACK_ELEMENT pCallStackArray,size_t uArraySize,std::vector<std::string>& CallStackVec);

}

#endif //__UTILITY_H__
