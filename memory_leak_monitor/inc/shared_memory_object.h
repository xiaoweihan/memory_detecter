#ifndef __SHARED_MEMORY_OBJECT_H__
#define __SHARED_MEMORY_OBJECT_H__

class CLogger;

class CSharedMemoryObject
{
public:
    CSharedMemoryObject(CLogger* pLogger);
    virtual ~CSharedMemoryObject();

protected:
    CLogger* m_pLogger;

};


#endif  //__SHARED_MEMORY_OBJECT_H__
