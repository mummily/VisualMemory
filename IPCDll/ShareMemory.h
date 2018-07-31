#pragma once
#include <Windows.h>
class ShareMemory
{
public:
    ShareMemory(void);
    virtual ~ShareMemory(void);

public:
    int createShareMemory(int nSize, const char* pName);
    int openShareMemory(int nSize, const char* pName);
    int writeData(const char* pData, int nSize);
    int readData(const char* pData, int& nSize);
    int closeShareMemory();

protected:
    HANDLE m_hMap;
    char* m_pBase;
    char* m_pRead;
    char* m_pWrite;
    int m_nSize;
    char m_szBufCached[1024*1024*12];
    int m_nOffsetSize;

protected:
    HANDLE m_hReadEvent;
    HANDLE m_hWriteEvent;
    HANDLE m_hMutex;
    CRITICAL_SECTION m_cs;
};

