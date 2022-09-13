#include "display_memory_data.h"
#include <vector>
#include <new>
#include <string.h>
#include <stdio.h>
#include "shared_memory_data_manager.h"
#include "utility.h"
using std::vector;

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"

static void print_red_content(const char *szContent)
{
    if (nullptr == szContent)
    {
        return;
    }
    size_t uBufferLen = strlen(szContent) + 100;
    char *pBuffer = new (std::nothrow) char[uBufferLen];
    if (nullptr == pBuffer)
    {
        return;
    }

    snprintf(pBuffer, uBufferLen, "%s%s%s\n", RED, szContent, NONE);

    printf(pBuffer);

    delete pBuffer;
    pBuffer = nullptr;
}

static void print_green_content(const char *szContent)
{
    if (nullptr == szContent)
    {
        return;
    }
    size_t uBufferLen = strlen(szContent) + 100;
    char *pBuffer = new (std::nothrow) char[uBufferLen];
    if (nullptr == pBuffer)
    {
        return;
    }
    snprintf(pBuffer, uBufferLen, "%s%s%s\n", GREEN, szContent, NONE);
    printf(pBuffer);
    delete pBuffer;
    pBuffer = nullptr;
}

void CMemoryDataDisplayer::DisplayMemoryDataInfo(void)
{
    if (nullptr == m_pDataManager)
    {
        return;
    }

    char szBorder[] = "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    char szSpliter[] = "*********************************************************************************************";

    print_green_content(szBorder);

    // 获取数据头
    MEMORY_EVENT_HEADER Header;
    m_pDataManager->GetMemoryDataHeader(Header);
    std::string strStartTime = "None";
    std::string strEndTime = "None";

    if (Header.ProcessStartTime != 0)
    {
        strStartTime = Utility::ConvertTimeToString(&Header.ProcessStartTime);
    }

    if (Header.ProcessEndTime != 0)
    {
        strEndTime = Utility::ConvertTimeToString(&Header.ProcessEndTime);
    }

    //打印数据头
    char szBuffer[1024] = {0};

    if (0 != Header.processID)
    {
        snprintf(szBuffer, sizeof(szBuffer), "ProcessPID:[%d]    StartTime:[%s]     EndTime[%s]", Header.processID, strStartTime.c_str(), strEndTime.c_str());
    }
    else
    {
        snprintf(szBuffer, sizeof(szBuffer), "ProcessPID:[%s]    StartTime:[%s]     EndTime[%s]", "None", strStartTime.c_str(), strEndTime.c_str());
    }
    print_green_content(szBuffer);
    print_green_content(szSpliter);
    vector<LP_SHARED_MEMORY_DATA> NormalDataArray;
    NormalDataArray.clear();
    m_pDataManager->GetMemoryLeakElement(NormalDataArray);

    for (auto &Element : NormalDataArray)
    {
        auto strContent = Utility::ConvertMemoryDataToString(Element);
        print_green_content(strContent.c_str());
    }

    char szNormalTotal[260] = {0};
    snprintf(szNormalTotal, sizeof(szNormalTotal), "Total Num:[%zu]", NormalDataArray.size());
    print_green_content(szNormalTotal);

    print_green_content(szSpliter);
    vector<LP_SHARED_MEMORY_DATA> ExceptionDataArray;
    ExceptionDataArray.clear();
    m_pDataManager->GetExceptionMemoryLeakElement(ExceptionDataArray);

    for (auto &Element : ExceptionDataArray)
    {
        auto strContent = Utility::ConvertMemoryDataToString(Element);
        print_red_content(strContent.c_str());
    }

    char szExceptionTotal[260] = {0};
    snprintf(szExceptionTotal, sizeof(szExceptionTotal), "Exception Total Num:[%zu]", ExceptionDataArray.size());
    print_red_content(szExceptionTotal);
    print_green_content(szBorder);

    for (auto &Element : NormalDataArray)
    {
        if (nullptr != Element)
        {
            delete Element;
            Element = nullptr;
        }
    }
    NormalDataArray.clear();

    for (auto &Element : ExceptionDataArray)
    {
        if (nullptr != Element)
        {
            delete Element;
            Element = nullptr;
        }
    }
    ExceptionDataArray.clear();
}
