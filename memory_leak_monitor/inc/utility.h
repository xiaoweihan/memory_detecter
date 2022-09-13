#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <time.h>
#include <string>
#include "Type.h"
namespace Utility
{

    // 时间转换为字符串
    std::string ConvertTimeToString(const time_t *pCovertTime);

    std::string ConvertMemoryDataToString(const LP_SHARED_MEMORY_DATA pData);

}

#endif //__UTILITY_H__
