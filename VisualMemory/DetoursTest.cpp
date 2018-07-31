#include "DetoursTest.h"
#include "qt_windows.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include "Utils.h"
#include "CommonDef.h"
#include "highcompare/CallStackDataController.h"
#include "tlhelp32.h"

#include "DbgHelp.h"
#include <stdio.h>
#include <tchar.h>
#include "../IPCDll/ShareMemory.h"
#include "../IPCDll/Server.h"

Server* g_pReader;

#include "detours.h"  
#pragma comment(lib,"detours.lib") //导入detours.h和detours.lib文件


typedef void* (__cdecl* Malloc_Func)(size_t nSize);
Malloc_Func SystemMallocFunc = NULL;
extern "C" _declspec(dllexport) void* MyHookMalloc(size_t nSize);

typedef void (__cdecl* Free_Func)(void *ptr);
Free_Func SystemFreeFunc = NULL;
extern "C" _declspec(dllexport) void MyHookFree(void *ptr);


// 轻量级锁，非内核对象
CRITICAL_SECTION g_cs;
CRITICAL_SECTION g_csBeginWait;
CRITICAL_SECTION g_csEndWait;


typedef struct _RTL_STACK_TRACE_ENTRY
{
    ULONG Depth;
    ULONG Size;
    PVOID Ptr;
    ULONG Hash;
    PVOID BackTrace [MAX_STACK_DEPTH];
    __int64 systemTime;
    //struct _RTL_STACK_TRACE_ENTRY * HashChain;
    //ULONG TraceCount;
    //USHORT Index;
} RTL_STACK_TRACE_ENTRY, *PRTL_STACK_TRACE_ENTRY;

class CallStackDataStream
{
public:
    CallStackDataStream *next;
    int len;
    char buf[1024*1024*1+256];

    CallStackDataStream()
    {
        next = nullptr;
        len = 0;
    }
};

//#include <QQueue>
//class CallStackDataStreamList
//{
//public:
//    CallStackDataStream *getNextEndStream()
//    {
//        EnterCriticalSection(&g_cs);
//
//        if (m_needDealList.size() > 1000)
//        {
//            LeaveCriticalSection(&g_cs);
//            return nullptr;
//        }
//
//        if (m_freeList.isEmpty())
//        {
//            //qDebug() << "CallStackDataStream count=" << m_needDealList.size();
//            LeaveCriticalSection(&g_cs);
//            return new CallStackDataStream();
//        }
//        else
//        {
//            CallStackDataStream *stream = m_freeList.front();
//            m_freeList.pop_front();
//            LeaveCriticalSection(&g_cs);
//            return stream;
//        }
//    }
//
//    CallStackDataStream *getUnsolvedStream()
//    {
//        EnterCriticalSection(&g_cs);
//        if (m_needDealList.isEmpty())
//        {
//            LeaveCriticalSection(&g_cs);
//            return nullptr;
//        }
//        else
//        {
//            CallStackDataStream *stream = m_needDealList.front();
//            m_needDealList.pop_front();
//            LeaveCriticalSection(&g_cs);
//            return stream;
//        }
//    }
//
//    void addNeedDealStream(CallStackDataStream *stream)
//    {
//        EnterCriticalSection(&g_cs);
//        m_needDealList.push_back(stream);
//        LeaveCriticalSection(&g_cs);
//    }
//
//    void addFreeStream(CallStackDataStream *stream)
//    {
//        EnterCriticalSection(&g_cs);
//        m_freeList.push_back(stream);
//        LeaveCriticalSection(&g_cs);
//    }
//
//    void deleteFreeStreams()
//    {
//        EnterCriticalSection(&g_cs);
//        int beforeDeleteSize = m_freeList.size();
//        while (m_freeList.size() > 100 && beforeDeleteSize - m_freeList.size() < 100)
//        {
//            CallStackDataStream *stream = m_freeList.front();
//            m_freeList.pop_front();
//            delete stream;
//            stream = nullptr;
//        }
//        LeaveCriticalSection(&g_cs);
//    }
//
//    QQueue<CallStackDataStream*> m_needDealList;
//    QQueue<CallStackDataStream*> m_freeList;
//
//    CallStackDataStreamList()
//    {
//        ::InitializeCriticalSection(&g_cs);
//    }
//};
CacheDataList<CallStackDataStream> g_CallStackDataStreamList;

// 调用栈处理线程
class CallStackThread : public QThread
{
public:
    CallStackThread()
    {
        g_pReader = new Server();
        g_pReader->init(12*1024*1024);
    }

    int nSize;
    void run()
    {
        //return;
        CallStackDataStream *stream = nullptr;
        while (true)
        {
            if (stream == nullptr)
            {
                stream = g_CallStackDataStreamList.getNextEndStream();
            }
            if (stream == nullptr)
            {
                Sleep(10);
                continue;
            }
            stream->len = 0;
            g_pReader->readData(stream->buf, stream->len);
            //continue;
            if (stream->len > 0)
            {
                //static int s_addNeedDealStream = 0;
                //qDebug() << "qingh-a, s_addNeedDealStream = " << ++s_addNeedDealStream << stream->len;
                g_CallStackDataStreamList.addNeedDealStream(stream);
                stream = nullptr;
            }
        }
    }

    void addCallStack(void *ptr, size_t nSize);
    void freeCallStack(void *ptr);
};

// 调用栈处理线程

class CallStackThread2 : public QThread
{
public:
    CallStackThread2()
    {
    }

    void run()
    {
        //return;
        int objSize = sizeof(RTL_STACK_TRACE_ENTRY);
        QMap<PVOID/*ptr*/, int/*times*/> freeDatas;
        CallStackData callStackData; 
        CallStackDataStream *lastStream = nullptr;
        while (true)
        {
            CallStackDataStream *stream = g_CallStackDataStreamList.getUnsolvedStream();
            if (stream)
            {
                //static int s_getUnsolvedStream = 0;
                //qDebug() << "qingh-a, s_getUnsolvedStream = " << ++s_getUnsolvedStream << stream->len;

                if (DetoursTest::m_bIsStating)
                {
                    qDebug() << "qingh-a, stream:" << stream->len;
                }

                if (lastStream == nullptr)
                {
                    CallStackDataController::instance()->lock();
                }
                //CallStackDataController::instance()->dealwithDelayFreeDatas();
                for (int i=0; i<stream->len / objSize; ++i)
                {
                    RTL_STACK_TRACE_ENTRY *input = (RTL_STACK_TRACE_ENTRY*)(stream->buf + objSize * i);
                    callStackData.depth = (int)input->Depth;
                    // 内存释放信息
                    if (callStackData.depth == -1)
                    {
                        //if (freeDatas[input->Ptr] == 0)
                        {
                            callStackData.ptr = input->Ptr;
                            callStackData.systemTime = input->systemTime;
                            CallStackDataController::instance()->freeCallStackData(&callStackData);
                        }
                    }
                    // 当前堆内存
                    else if (callStackData.depth == -2)
                    {
                        //if (freeDatas[input->Ptr] == 0)
                        {
                            callStackData.ptr = input->Ptr;
                            callStackData.size = input->Size;
                            callStackData.systemTime = input->systemTime;
                            CallStackDataController::instance()->checkHeapMemory(&callStackData);
                        }
                    }
                    // hook前堆内存数
                    else if (callStackData.depth == -3)
                    {
                        //if (freeDatas[input->Ptr] == 0)
                        {
                            //GlobalData::getInstance()->relativeTotalLeakMemoryTimes = input->Size - GlobalData::getInstance()->totalLeakMemoryTimes_Last;
                            //GlobalData::getInstance()->relativeTotalLeakMemorySize = (int)input->Ptr - GlobalData::getInstance()->totalLeakMemorySize_Last;
                            //GlobalData::getInstance()->totalLeakMemoryTimes = input->Size;
                            //GlobalData::getInstance()->totalLeakMemorySize = (int)input->Ptr;
                            if (GlobalData::getInstance()->memoryMode != 0)
                            {
                                GlobalData::getInstance()->totalLeakMemoryTimes = input->Size;
                                GlobalData::getInstance()->totalLeakMemorySize = (int)input->Ptr;
                            }
                            if (GlobalData::getInstance()->totalLeakMemoryTimes_BeforeHook == 0)
                            {
                                GlobalData::getInstance()->totalLeakMemoryTimes_BeforeHook = input->Size;
                                GlobalData::getInstance()->totalLeakMemorySize_BeforeHook = (int)input->Ptr;
                                CallStackDataController::instance()->clearAllPtr();
                            }
                        }
                    }
                    // 缓存数据接收完毕
                    else if (callStackData.depth == -4)
                    {
                        ::SetEvent(EventController::getInstance()->hCatheReceivedEvent);
                        // 等待本阶段数据处理完毕
                        CallStackDataController::instance()->unlock();
                        while (WAIT_OBJECT_0 == WaitForSingleObject(EventController::getInstance()->hCatheReceivedEvent, 1000))
                        {
                            // 等待
                            ::Sleep(100);
                        }
                        CallStackDataController::instance()->lock();
                    }
                    // 内存申请信息
                    else if (callStackData.depth > 0)
                    {
                        if (input->Ptr == nullptr)
                        {
                            qDebug() << "qingh-a, ptr = null";
                        }
                        //if (callStackData.depth > 32 || input->Depth > 32)
                        //{
                        //    callStackData.depth = 31;

                        //}
                        //if (freeDatas[input->Ptr] > 0)
                        //{
                        //    freeDatas[callStackData.ptr] = freeDatas[callStackData.ptr] - 1;
                        //}
                        //else
                        {
                            callStackData.size = input->Size;
                            callStackData.sizePerTime = input->Size;
                            callStackData.times = 1;
                            callStackData.ptr = input->Ptr;
                            callStackData.total = input->Hash;
                            callStackData.systemTime = input->systemTime;
                            memcpy(callStackData.backtrace, input->BackTrace, input->Depth*sizeof(input->BackTrace[0]));
                            CallStackDataController::instance()->addCallStackData(&callStackData);
                        }
                    }
                }

                //CallStackDataController::instance()->unlock();
                g_CallStackDataStreamList.addFreeStream(stream);
            }
            else
            {
                CallStackDataController::instance()->m_PtrDataList.deleteFreeStreams();
                if (lastStream != nullptr)
                {
                    CallStackDataController::instance()->unlock();
                }
                g_CallStackDataStreamList.deleteFreeStreams();
                
                //SwitchToThread();
                ::Sleep(10);
            }

            lastStream = stream;
        }
    }
};

//#pragma optimize("y", off) // disable FPO ?

CallStackThread g_CallStackThread;
CallStackThread2 g_CallStackThread2;


void CallStackThread::addCallStack(void *ptr, size_t nSize)
{
    return;
    RTL_STACK_TRACE_ENTRY Trace;
    Trace.Depth = RtlCaptureStackBackTrace(1, MAX_STACK_DEPTH, Trace.BackTrace, &Trace.Hash);
    // ...

    //EnterCriticalSection(&g_cs);
    //try {
    //    // 调用堆栈收集
    //    {
    //        depth = RtlCaptureStackBackTrace(1, MAX_STACK_DEPTH, BackTrace + MAX_STACK_DEPTH * m_addTimes, &Hash);
    //        depths[m_addTimes] = depth;
    //        sizeList[m_addTimes] = nSize;
    //        ptrList[m_addTimes] = ptr;
    //        ++m_addTimes;
    //    }

    //    // 堆栈数据处理代码
    //    {
    //        static CallStackData *data = new CallStackData();
    //        for (int i=0; i<m_addTimes; i++)
    //        {
    //            data->ptr = ptrList[i];
    //            data->depth = depths[i];
    //            data->times = 1;
    //            data->size = sizeList[i]; //test
    //            data->updateTimes = 1;
    //            data->updateSize = sizeList[i]; //test
    //            data->total = 0;
    //            for (int j=0; j<depths[i]; j++)
    //            {
    //                data->backtrace[j] = BackTrace[MAX_STACK_DEPTH * i + j];
    //                data->total += (int)data->backtrace[j];
    //            }
    //            CallStackDataController::instance()->addCallStackData(data);
    //        }
    //        m_addTimes = 0;
    //    }
    //}
    //catch(...) {
    //    depth = 0;
    //}
    //
    //LeaveCriticalSection(&g_cs); 
}

void CallStackThread::freeCallStack(void *ptr)
{
    //EnterCriticalSection(&g_cs);
    //CallStackDataController::instance()->freeCallStackData(ptr);
    //LeaveCriticalSection(&g_cs); 
}

static bool g_bIsHooking = false;
extern "C" _declspec(dllexport) void* MyHookMalloc(size_t nSize)
{
    void *ptr = SystemMallocFunc(nSize);
    if (DetoursTest::m_bIsStating)
    {
        return ptr;
    }
    
    //EnterCriticalSection(&g_cs);
    //if (!g_bIsHooking)
    {
        //g_bIsHooking = true;
        g_CallStackThread.addCallStack(ptr, nSize);
        //g_bIsHooking = false;
    }
    
    //LeaveCriticalSection(&g_cs); 
    
    return ptr;
}

//QMutex g_freeMutex(QMutex::Recursive);
extern "C" _declspec(dllexport) void MyHookFree(void *ptr)
{
    if (DetoursTest::m_bIsStating)
    {
        return SystemFreeFunc(ptr);
    }

    //EnterCriticalSection(&g_cs);
    //if (!g_bIsHooking)
    {
        //g_bIsHooking = true;
        g_CallStackThread.freeCallStack(ptr);
        //g_bIsHooking = false;
    }
    //LeaveCriticalSection(&g_cs);
    return SystemFreeFunc(ptr);
}

void DetoursTest::Hook()
{
    if (m_bIsHook)
    {
        return;
    }
    ::InitializeCriticalSection(&g_cs);
    g_CallStackThread.start();

    ::Sleep(100);
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    //这里可以连续多次调用DetourAttach，表明HOOK多个函数
    SystemMallocFunc = (Malloc_Func)DetourFindFunction("msvcr100","malloc");
    DetourAttach(&(PVOID&)SystemMallocFunc,MyHookMalloc);

    SystemFreeFunc = (Free_Func)DetourFindFunction("msvcr100","free");
    DetourAttach(&(PVOID&)SystemFreeFunc,MyHookFree);

    DetourTransactionCommit();
}

void DetoursTest::UnHook()
{
    m_bIsHook = false;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    //这里可以连续多次调用DetourDetach,表明撤销多个函数HOOK
    DetourDetach(&(PVOID&)SystemMallocFunc,MyHookMalloc);
    DetourDetach(&(PVOID&)SystemFreeFunc,MyHookFree);

    DetourTransactionCommit();
}

void DetoursTest::injectDllDetours(QString cmdLine)
{
    g_CallStackThread.start();
    g_CallStackThread2.start();

    STARTUPINFO si;     
    PROCESS_INFORMATION pi;  
    ZeroMemory(&si, sizeof(STARTUPINFO));     
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));     
    si.cb = sizeof(STARTUPINFO);     
    char* DirPath = new char[MAX_PATH];     
    char* DLLPath = new char[MAX_PATH]; //testdll.dll     
    char* DetourPath = new char[MAX_PATH]; //detoured.dll   
    GetCurrentDirectory(MAX_PATH, DirPath);     
    sprintf_s(DLLPath, MAX_PATH, "%s\\IPCDll.dll", qApp->applicationDirPath().toLocal8Bit().data());     
    sprintf_s(DetourPath, MAX_PATH, "%s\\Detours.dll", qApp->applicationDirPath().toLocal8Bit().data());     

    BOOL result = DetourCreateProcessWithDll(NULL, cmdLine.toLocal8Bit().data(),  
        NULL,NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL,&si, &pi, DetourPath, DLLPath, NULL);

    // 最新detours库接口去掉了detours的路径设置
    //BOOL result = DetourCreateProcessWithDll(NULL, appFullPath.toLocal8Bit().data(),  
    //    NULL,NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL,&si, &pi, DLLPath, NULL);

    qDebug() << "result" << result;
    delete [] DirPath;     
    delete [] DLLPath;     
    delete [] DetourPath;
}

int DetoursTest::m_iShowIntevalTimes = 1;
bool DetoursTest::m_bIsStating = false;
DetoursTest::DetoursTest(void) : m_bIsHook(false)
{

}


DetoursTest::~DetoursTest(void)
{
    if (m_bIsHook)
    {
        UnHook();
    }
}

DetoursTest * DetoursTest::instance()
{
    static DetoursTest s_DetoursTest;
    return &s_DetoursTest;
}


/* DllMain */

HINSTANCE g_hDLLInst = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            g_hDLLInst = hModule;

            //::InitializeCriticalSection(&g_cs);
            
            //DetoursTest::instance()->Hook();
            //g_CallStackThread.start();
            //g_CallStackThread2.start();
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        {
            //DetoursTest::instance()->UnHook();
        }
        break;
    }
    return TRUE;
}
