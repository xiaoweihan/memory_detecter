#ifndef __DISPLAY_MEMORY_DATA_H__
#define __DISPLAY_MEMORY_DATA_H__


class CSharedMemoryDataManager;

class CMemoryDataDisplayer
{
public:
    CMemoryDataDisplayer(CSharedMemoryDataManager* pDataManager):m_pDataManager(pDataManager)
    {

    }
    ~CMemoryDataDisplayer() = default;

    void DisplayMemoryDataInfo(void);

private:
    CSharedMemoryDataManager* m_pDataManager{nullptr};

};


#endif //__DISPLAY_MEMORY_DATA_H__
