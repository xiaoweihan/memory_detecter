#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <signal.h>
#include "logger.h"
#include "shared_memory_env.h"
#include "shared_memory_data_manager.h"
#include "shared_memory_operator.h"
#include "display_memory_data.h"
#include "Type.h"
using namespace std;

const int TERMINATE_LOOP_SIGNAL = (SIGRTMIN + 6);

//循环标志
std::atomic_bool g_Loop{true};

void SignalHandler(int nSignalNo)
{
    if (nSignalNo == TERMINATE_LOOP_SIGNAL)
    {
        g_Loop = false;
    }
}

int main(int nArgc, char *argv[])
{
    std::string strLoggerConfigPath = "/etc/log4cpp.conf";
    //日志打印
    shared_ptr<CLogger> pLogger = make_shared<CLogger>(strLoggerConfigPath);
    if (!pLogger)
    {
        cerr << "Allocate Logger failed!" << endl;
        return 1;
    }
    if (!pLogger->InitLogger())
    {
        cerr << "Logger Init failed!" << endl;
        return 1;
    }

    // 创建共享内存
    string strFileName = "memory_leak_hunter";
    unsigned int uMapSize = sizeof(SHARED_MEMORY_HEADER) + sizeof(SHARED_MEMORY_DATA) * MAX_DATA_LENGTH;
    shared_ptr<CSharedMemoryEnv> pSharedMemoryEnv = make_shared<CSharedMemoryEnv>(pLogger.get(), strFileName, uMapSize);
    if (!pSharedMemoryEnv)
    {
        pLogger->GetFileLogger().error("Allocate CSharedMemoryEnv obj failed! [%s:%d]", __FILE__, __LINE__);
        return 1;
    }

    shared_ptr<CSharedMemoryDataManager> pSharedMemoryDataManager = make_shared<CSharedMemoryDataManager>(pLogger.get());
    if (!pSharedMemoryDataManager)
    {
        pLogger->GetFileLogger().error("Allocate CSharedMemoryDataManager obj failed! [%s:%d]", __FILE__, __LINE__);
        return 1;
    }

    shared_ptr<CSharedMemoryOperator> pSharedMemoryOperator = make_shared<CSharedMemoryOperator>(pLogger.get(), pSharedMemoryEnv.get(), pSharedMemoryDataManager.get());
    if (!pSharedMemoryOperator)
    {
        pLogger->GetFileLogger().error("Allocate CSharedMemoryOperator obj failed! [%s:%d]", __FILE__, __LINE__);
        return 1;
    }

    shared_ptr<CMemoryDataDisplayer> pDisplayer = make_shared<CMemoryDataDisplayer>(pSharedMemoryDataManager.get());
    if (!pDisplayer)
    {
        pLogger->GetFileLogger().error("Allocate CMemoryDataDisplayer obj failed! [%s:%d]", __FILE__, __LINE__);
        return 1;
    }

    struct sigaction Action;

    Action.sa_handler = SignalHandler;

    sigemptyset(&Action.sa_mask);

    Action.sa_flags = 0;

    if (sigaction(TERMINATE_LOOP_SIGNAL, &Action, NULL) != 0)
    {
        pLogger->GetFileLogger().error("register signal failed! [%s:%d]", __FILE__, __LINE__);
        return 1;
    }

    //开始创建线程接收
    auto pReadThreadEntry = [&pSharedMemoryOperator]()
    {
        while (g_Loop)
        {
            // 开始读取数据
            pSharedMemoryOperator->ReadSharedMemoryData();
        }
    };

    // 显示线程
    auto pDisplayThreadEntry = [&pDisplayer]()
    {
        while (g_Loop)
        {
            // 刷新显示数据
            pDisplayer->DisplayMemoryDataInfo();
            usleep(800 * 1000);
        }
    };

    thread ReadThread(pReadThreadEntry);

    thread DisplayThread(pDisplayThreadEntry);

    ReadThread.join();
    DisplayThread.join();

    return 0;
}
