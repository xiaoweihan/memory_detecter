#include "shared_memory_data_manager.h"
#include <new>
#include "logger.h"
#include "utility.h"
CSharedMemoryDataManager::CSharedMemoryDataManager(CLogger *pLogger) : CSharedMemoryObject(pLogger)
{
    m_pMemoryDataHeader = new (std::nothrow) MEMORY_EVENT_HEADER();

    if (nullptr == m_pMemoryDataHeader && nullptr != m_pLogger)
    {
        m_pLogger->GetFileLogger().error("Allocate MEMORY_EVENT_HEADER failed! [%s:%d]",__FILE__,__LINE__);
    }
}

CSharedMemoryDataManager::~CSharedMemoryDataManager()
{
    if (nullptr != m_pMemoryDataHeader)
    {
        delete m_pMemoryDataHeader;
        m_pMemoryDataHeader = nullptr;
    }

    for (auto &Element : m_MemoryLeakInfo)
    {
        if (nullptr != Element.second)
        {
            delete Element.second;
            Element.second = nullptr;
        }
    }
    m_MemoryLeakInfo.clear();

    for (auto &ExceptionElement : m_ExceptionLeakInfo)
    {
        if (nullptr != ExceptionElement)
        {
            delete ExceptionElement;
            ExceptionElement = nullptr;
        }
    }
    m_ExceptionLeakInfo.clear();
}

bool CSharedMemoryDataManager::AddMemoryLeakElement(const LP_SHARED_MEMORY_DATA pData)
{
    if (nullptr == pData || nullptr == m_pLogger)
    {
        return false;
    }
    bool bHasException = false;
    LP_SHARED_MEMORY_DATA pExceptionData = nullptr;

    {
        std::lock_guard<std::mutex> Locker(m_NormalLocker);

        auto Iter = m_MemoryLeakInfo.find(pData->pEventAddress);
        // 如果此时出现一样的，只有一种可能是realloc的情况
        if (Iter != m_MemoryLeakInfo.end())
        {
            if (pData->eumEventType == MEMORY_EVENT::REALLOC_EVENT)
            {
                m_MemoryLeakInfo[pData->pEventAddress] = pData;
            }
            else
            {
                pExceptionData = Iter->second;
                bHasException = true;
                m_MemoryLeakInfo[pData->pEventAddress] = pData;
            }
        }
        else
        {
            m_MemoryLeakInfo[pData->pEventAddress] = pData;
        }
    }

    if (bHasException)
    {
        //auto strContent = Utility::ConvertMemoryDataToString(pExceptionData);
        //m_pLogger->GetFileLogger().error("Exception DataInfo:%s. [%s:%d]", strContent.c_str(), __FILE__, __LINE__);
        std::lock_guard<std::mutex> Locker(m_ExceptionMutex);
        m_ExceptionLeakInfo.push_back(pExceptionData);
    }

    return true;
}

bool CSharedMemoryDataManager::DelMemoryLeakElement(const LP_SHARED_MEMORY_DATA pData)
{
    if (nullptr == pData || nullptr == m_pLogger)
    {
        return false;
    }
    bool bHasException = false;
    LP_SHARED_MEMORY_DATA pExceptionData = nullptr;

    std::lock_guard<std::mutex> Locker(m_NormalLocker);

    auto Iter = m_MemoryLeakInfo.find(pData->pEventAddress);
    if (Iter == m_MemoryLeakInfo.end())
    {
        bHasException = true;
        pExceptionData = pData;
    }
    else
    {
        m_MemoryLeakInfo.erase(Iter);
    }

    if (bHasException)
    {
        //auto strContent = Utility::ConvertMemoryDataToString(pExceptionData);
        //m_pLogger->GetFileLogger().error("Exception DataInfo:%s. [%s:%d]", strContent.c_str(), __FILE__, __LINE__);
        std::lock_guard<std::mutex> Locker(m_ExceptionMutex);
        m_ExceptionLeakInfo.push_back(pExceptionData);
    }

    return true;
}

void CSharedMemoryDataManager::GetMemoryLeakElement(std::vector<LP_SHARED_MEMORY_DATA>& ElementArray)
{
    ElementArray.clear();
    std::lock_guard<std::mutex> Locker(m_NormalLocker);
    for (auto& Element:m_MemoryLeakInfo)
    {
        LP_SHARED_MEMORY_DATA pData = new (std::nothrow) SHARED_MEMORY_DATA();

        if (nullptr == pData || nullptr == Element.second)
        {
            //打印日志
            continue;
        }
        *pData = *Element.second;
        ElementArray.push_back(pData);
    }
}

void CSharedMemoryDataManager::GetExceptionMemoryLeakElement(std::vector<LP_SHARED_MEMORY_DATA> &ElementArray)
{
    ElementArray.clear();
    std::lock_guard<std::mutex> Locker(m_ExceptionMutex);
    for (auto &Element : m_ExceptionLeakInfo)
    {
        LP_SHARED_MEMORY_DATA pData = new (std::nothrow) SHARED_MEMORY_DATA();

        if (nullptr == pData || nullptr == Element)
        {
            //打印日志
            continue;
        }
        *pData = *Element;
        ElementArray.push_back(pData);
    }
}

// 通知事件到来
void CSharedMemoryDataManager::NotifyEvent(const LP_SHARED_MEMORY_DATA pData)
{
    if (nullptr == pData)
    {
        return;
    }

    if (pData->eumEventType <= MEMORY_EVENT::EVENT_MIN || pData->eumEventType >= MEMORY_EVENT::EVENT_MAX)
    {
        return;
    }

    switch (pData->eumEventType)
    {
    case MEMORY_EVENT::MALLOC_EVENT:
    case MEMORY_EVENT::REALLOC_EVENT:
    case MEMORY_EVENT::CALLOC_EVENT:
        AddMemoryLeakElement(pData);
        break;
    case MEMORY_EVENT::FREE_EVENT:
        DelMemoryLeakElement(pData);
        break;
    default:
        break;
    }
}

void CSharedMemoryDataManager::SetMemoryDataHeader(const MEMORY_EVENT_HEADER& Header)
{

    std::lock_guard<std::mutex> Locker(m_NormalLocker);
    if (nullptr != m_pMemoryDataHeader)
    {
        *m_pMemoryDataHeader = Header;
    }
}

void CSharedMemoryDataManager::GetMemoryDataHeader(MEMORY_EVENT_HEADER& Header)
{
    std::lock_guard<std::mutex> Locker(m_NormalLocker);
    if (nullptr != m_pMemoryDataHeader)
    {
        Header = *m_pMemoryDataHeader;
    }

}
