#include "ShareMemory.h"

#define GLOBAL_WRITE_EVENT_NAME  ("Local\\GLOBAL_WRITE_EVENT_NAME")
#define GLOBAL_READ_EVENT_NAME  ("Local\\GLOBAL_READ_EVENT_NAME")

#define GLOBAL_LOCK_MUTEX_NAME  ("Local\\GLOBAL_LOCK_MUTEX_NAME")

//#define  MAX_WAIT_TIME (10*1000*1000)
#define  MAX_WAIT_TIME (10*1000*1000)

ShareMemory::ShareMemory(void)
{
    m_hReadEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_READ_EVENT_NAME);
    m_hWriteEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_WRITE_EVENT_NAME);
    m_hMutex = CreateMutexA(NULL, FALSE, GLOBAL_LOCK_MUTEX_NAME);
    InitializeCriticalSection(&m_cs);
    m_nOffsetSize = 0;
}

ShareMemory::~ShareMemory(void)
{
    
    CloseHandle(m_hReadEvent);
    CloseHandle(m_hWriteEvent);
    m_hReadEvent = nullptr;
    m_hWriteEvent = nullptr;
    DeleteCriticalSection(&m_cs);
}

int ShareMemory::createShareMemory(int nSize, const char* pName)
{
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION); 
    SetSecurityDescriptorDacl(&sd,TRUE,NULL,FALSE); 
    SECURITY_ATTRIBUTES sa;
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    m_hMap = ::CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, nSize, pName);
    m_pBase = (char*)::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    memset(m_pBase, 0, nSize);
    m_pRead = m_pBase + 4;
    m_pWrite = m_pBase;
    m_nSize = nSize;
    return 0;
}

int ShareMemory::openShareMemory(int nSize, const char* pName)
{
    m_hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, pName);
    m_pBase = (char*)::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    m_pRead = m_pBase;
    m_pWrite = m_pBase + 4;
    m_nSize = nSize;
    ::SetEvent(m_hReadEvent);
    return 0;
}

#define MAX_STACK_DEPTH 128

#include <iostream>
using namespace std;

int ShareMemory::writeData(const char* pData, int nSize)
{
    if (nSize <= 0)
    {
        return nSize;
    }

    EnterCriticalSection(&m_cs);
    int nSelfSize = nSize;
//     PRTL_STACK_TRACE_ENTRY pppp = (PRTL_STACK_TRACE_ENTRY)pData;
//     if (pppp->Depth == -1)
//     {
//         nSelfSize = 4;
//     }
//     memcpy(m_szBufCached+m_nOffsetSize, pData, nSelfSize);
//     m_nOffsetSize += nSize;
//     if (m_nOffsetSize >= 1024*1024*10)
//     {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hReadEvent, MAX_WAIT_TIME))
        {
            memcpy(m_pWrite, pData, nSize);
            *((int*) m_pBase) = nSize;
            //m_nOffsetSize = 0;
            ::ResetEvent(m_hReadEvent);
            ::SetEvent(m_hWriteEvent);
        }
    //}
    LeaveCriticalSection(&m_cs);
    
    return nSize;
}

int ShareMemory::readData(const char* pData, int& nSize)
{
    if (WAIT_OBJECT_0 == WaitForSingleObject(m_hWriteEvent, MAX_WAIT_TIME))
    {
        nSize = *((int*)m_pBase);
        memcpy((char*)pData, m_pRead, nSize);
        *((int*)m_pBase) = 0;
        ::ResetEvent(m_hWriteEvent);
        ::SetEvent(m_hReadEvent);

        //nSize = 0;
        return nSize;
    }
    nSize = 0;
    return nSize;
}

int ShareMemory::closeShareMemory()
{
    m_pRead = nullptr;
    m_pWrite = nullptr;

    UnmapViewOfFile(m_pBase);
    m_pBase = nullptr;

    CloseHandle(m_hMap);
    m_hMap = nullptr;
   
    m_nSize = 0;
    return 0;
}
