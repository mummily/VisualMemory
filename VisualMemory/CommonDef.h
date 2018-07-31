#pragma once

#define MAX_STACK_DEPTH 128
#define CATCH_TIMES 1000

#include <QQueue>
#include <Windows.h>
template<typename T>
class CacheDataList
{
public:
    T *getNextEndStream()
    {
        EnterCriticalSection(&m_cs);

        if (m_needDealList.size() > m_maxCacheSize)
        {
            LeaveCriticalSection(&m_cs);
            return nullptr;
        }

        if (m_freeList.isEmpty())
        {
            //qDebug() << "CallStackDataStream count=" << m_needDealList.size();
            LeaveCriticalSection(&m_cs);
            return new T();
        }
        else
        {
            T *stream = m_freeList.front();
            m_freeList.pop_front();
            LeaveCriticalSection(&m_cs);
            return stream;
        }
    }

    T *getUnsolvedStream()
    {
        EnterCriticalSection(&m_cs);
        if (m_needDealList.isEmpty())
        {
            LeaveCriticalSection(&m_cs);
            return nullptr;
        }
        else
        {
            T *stream = m_needDealList.front();
            m_needDealList.pop_front();
            LeaveCriticalSection(&m_cs);
            return stream;
        }
    }

    void addNeedDealStream(T *stream)
    {
        EnterCriticalSection(&m_cs);
        m_needDealList.push_back(stream);
        LeaveCriticalSection(&m_cs);
    }

    void addFreeStream(T *stream)
    {
        //delete stream;
        //return;
        EnterCriticalSection(&m_cs);
        m_freeList.push_back(stream);
        LeaveCriticalSection(&m_cs);
    }

    void deleteFreeStreams()
    {
        EnterCriticalSection(&m_cs);
        int beforeDeleteSize = m_freeList.size();
        while (m_freeList.size() > m_minCacheSize && beforeDeleteSize - m_freeList.size() < 100)
        {
            T *stream = m_freeList.front();
            m_freeList.pop_front();
            delete stream;
            stream = nullptr;
        }
        LeaveCriticalSection(&m_cs);
    }

    QQueue<T*> m_needDealList;
    QQueue<T*> m_freeList;
    CRITICAL_SECTION m_cs;
    int m_maxCacheSize;
    int m_minCacheSize;

    CacheDataList()
    {
        ::InitializeCriticalSection(&m_cs);
        m_minCacheSize = 100;
        m_maxCacheSize = 1000;
    }
};
