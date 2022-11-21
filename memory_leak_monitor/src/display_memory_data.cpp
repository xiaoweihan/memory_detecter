#include "display_memory_data.h"
#include <iostream>
#include <vector>
#include <new>
#include <time.h>
#include <string.h>
#include <iomanip>
#include "shared_memory_data_manager.h"
#include "utility.h"


#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

using namespace std;
// 标题头的长度
constexpr int HEADER_LEN = 120;

void CMemoryDataDisplayer::DisplayHeaderInfo()
{
    
    cout << left << BROWN << "监控进程信息" << NONE << endl;
    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 输出标题头
    cout << left << BROWN << "*" << NONE << endl;
    
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
    if (0 != Header.processID)
    {
        // 输出进程的ID
        cout << left << BROWN << "ProcessPID: " << Header.processID << NONE << endl;
        // 输出进程的开始时间
        cout << left << BROWN << "StartTime: " << strStartTime << NONE << endl;
        // 输出进程的结束时间
        cout << left << BROWN << "EndTime: " << strEndTime << NONE << endl;
    }
    else
    {
        // 输出进程的ID
        cout << left << BROWN << "ProcessPID: " << "None" << NONE << endl;
        // 输出进程的开始时间 
        cout << left << BROWN << "StartTime: " << strStartTime << NONE << endl;
        // 输出进程的结束时间
        cout << left << BROWN << "EndTime: " << strEndTime << NONE << endl;
    }

    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 分割线
    cout << left << BROWN << "*" << NONE << endl;

    cout << endl;

}

void CMemoryDataDisplayer::DisplayMemoryDataInfo(void)
{
    if (nullptr == m_pDataManager)
    {
        return;
    }
    // 输出信息描述
    DisplayHeaderInfo();

    time_t CurrentTime = time(nullptr);
    // 打印内存统计信息
    cout << left << GREEN << "内存使用信息" << Utility::ConvertTimeToString(&CurrentTime) << NONE << endl;
    
    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 分割线
    cout << WHITE << "*" << NONE << endl;

    vector<LP_SHARED_MEMORY_DATA> NormalDataArray;
    NormalDataArray.clear();
    m_pDataManager->GetMemoryLeakElement(NormalDataArray);
    //打印统计信息
    cout << left << GREEN << "Total Num: " << NormalDataArray.size() << NONE << endl;

    //打印标题头
    cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << "ProcessID" << NONE;
    cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << "OccureTime" << NONE;
    cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << "EventType" << NONE;
    cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << "Address" << NONE;
    cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << "CallStack" << NONE;

    for (auto &Element : NormalDataArray)
    {   
        cout << endl;
        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << Element->eventProcessPID << NONE;
        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << Utility::ConvertTimeToString(&Element->eventoccureTime) << NONE;
        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << Utility::ConvertEventToString(Element->eumEventType) << NONE;
        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << (void*)(Element->pEventAddress) << NONE;
        
        vector<string> CallStackVec;
        Utility::CovertCallStackToString(Element->CallStackArray,MAX_CALL_STACK_LEVEL,CallStackVec);
        if (CallStackVec.empty())
        {
            continue;
        }

        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << CallStackVec[0] << NONE;   
        for (size_t i = 1; i < CallStackVec.size(); ++i)
        {
            cout << endl;
            cout << left << GREEN << setw(HEADER_LEN / 5 * 4) << setfill(' ') << " " << setw(HEADER_LEN / 5) << setfill(' ') << CallStackVec[i] << NONE;  
        }
        
    }

    cout << endl;
    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 分割线
    cout << WHITE << "*" << NONE << endl;


    vector<LP_SHARED_MEMORY_DATA> ExceptionDataArray;
    ExceptionDataArray.clear();
    m_pDataManager->GetExceptionMemoryLeakElement(ExceptionDataArray);

    cout << endl;
    // 打印内存统计信息
    cout << left << RED << "异常内存使用信息" << Utility::ConvertTimeToString(&CurrentTime) << NONE << endl;
    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 分割线
    cout << WHITE << "*" << NONE << endl;
    //打印统计信息
    cout << left << RED << "Exception Total Num: " << ExceptionDataArray.size() << NONE << endl;


    //打印标题头
    cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << "ProcessID" << NONE;
    cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << "OccureTime" << NONE;
    cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << "EventType" << NONE;
    cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << "Address" << NONE;
    cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << "CallStack" << NONE;
    
    for (auto &Element : ExceptionDataArray)
    {
        cout << endl;
        cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << Element->eventProcessPID << NONE;
        cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << Utility::ConvertTimeToString(&Element->eventoccureTime) << NONE;
        cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << Utility::ConvertEventToString(Element->eumEventType) << NONE;
        cout << left << RED << setw(HEADER_LEN / 5) << setfill(' ') << (void*)(Element->pEventAddress) << NONE;
        

        vector<string> CallStackVec;
        Utility::CovertCallStackToString(Element->CallStackArray,MAX_CALL_STACK_LEVEL,CallStackVec);
        if (CallStackVec.empty())
        {
            continue;
        }
        cout << left << GREEN << setw(HEADER_LEN / 5) << setfill(' ') << CallStackVec[0] << NONE;   
        for (size_t i = 1; i < CallStackVec.size(); ++i)
        {
            cout << endl;
            cout << left << GREEN << setw(HEADER_LEN / 5 * 4) << setfill(' ') << " " << setw(HEADER_LEN / 5) << setfill(' ') << CallStackVec[i] << NONE;  
        }    
    }
    
    cout << endl;
    cout.width(HEADER_LEN);
    cout.fill('*'); 
    // 分割线
    cout << WHITE << "*" << NONE << endl;

    cout << endl;
    
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
