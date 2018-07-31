#include <Windows.h>
#include "Client.h"
#include "stdio.h"
#include <iostream>
#include <vector>
#include <set>
#include <TlHelp32.h>
using namespace std;

#include "detours.h"  
#pragma comment(lib,"detours.lib") //导入detours.h和detours.lib文件

#define MAX_STACK_DEPTH 128

Client* g_pWriter = nullptr;
int gdwTlsIndex;
int g_isHookingTlsIndex;

typedef void* (__cdecl* Malloc_Func)(size_t nSize);
Malloc_Func SystemMallocFunc = NULL;
Malloc_Func SystemMallocFunc100 = NULL;
extern "C" _declspec(dllexport) void* MyHookMalloc(size_t nSize);

typedef void* (__cdecl* Realloc_Func)(void * _Memory, _In_ size_t _NewSize);
Realloc_Func SystemReallocFunc = NULL;
extern "C" _declspec(dllexport) void* MyHookRealloc(void * _Memory, _In_ size_t _NewSize);

typedef void* (__cdecl* Calloc_Func)(size_t _Count, size_t _Size);
Calloc_Func SystemCallocFunc = NULL;
extern "C" _declspec(dllexport) void* MyHookCalloc(size_t _Count, size_t _Size);

typedef void* (__cdecl* ReCalloc_Func)(void * _Memory, size_t _Count, size_t _Size);
ReCalloc_Func SystemReCallocFunc = NULL;
extern "C" _declspec(dllexport) void* MyHookReCalloc(void * _Memory, size_t _Count, size_t _Size); // 函数声明可能和原始函数不一致，导致崩溃

typedef void (__cdecl* Free_Func)(void *ptr);
Free_Func SystemFreeFunc = NULL;
extern "C" _declspec(dllexport) void MyHookFree(void *ptr);

// test hook tcmalloc
typedef void* (__cdecl* InvokeNewHook_Func)(const void* p, size_t s);
InvokeNewHook_Func TCMalloc_InvokeNewHook = NULL;
extern "C" _declspec(dllexport) void* MyInvokeNewHook(const void* p, size_t s);

typedef void (__cdecl* tcmalloc_Func)();
tcmalloc_Func TCMalloc_tcmalloc = NULL;
void MyTcmalloc();

extern "C" 
{
typedef void* (_stdcall * HeapAlloc_Func)(HANDLE hHeap,
    DWORD dwFlags,
    SIZE_T dwBytes);
extern "C" _declspec(dllexport) void* _stdcall  MyHeapAlloc(HANDLE hHeap,
    DWORD dwFlags,
    SIZE_T dwBytes);

typedef void* (_stdcall * HeapReAlloc_Func)(
    __inout HANDLE hHeap,
    __in    DWORD dwFlags,
    __deref LPVOID lpMem,
    __in    SIZE_T dwBytes
    );
extern "C" _declspec(dllexport) void* _stdcall  MyHeapReAlloc(
    __inout HANDLE hHeap,
    __in    DWORD dwFlags,
    __deref LPVOID lpMem,
    __in    SIZE_T dwBytes
    );

typedef BOOL (_stdcall * HeapFree_Func)(HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem);
extern "C" _declspec(dllexport) BOOL _stdcall  MyHeapFree(HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem);

typedef BOOL (_stdcall * HeapDestroy_Func)(HANDLE hHeap);
extern "C" _declspec(dllexport) BOOL _stdcall  MyHeapDestroy(HANDLE hHeap);


typedef void* (_stdcall * VirtualAlloc_Func)(
    LPVOID lpvAddress, 
    DWORD cbSize,
    DWORD fdwAllocationType,
    DWORD fdwProtect
    );
extern "C" _declspec(dllexport) void* _stdcall  MyVirtualAlloc(
    LPVOID lpvAddress, 
    DWORD cbSize,
    DWORD fdwAllocationType,
    DWORD fdwProtect
    );

typedef BOOL (_stdcall * VirtualFree_Func)(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );
extern "C" _declspec(dllexport) BOOL _stdcall  MyVirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

typedef void* (_stdcall * VirtualAllocEx_Func)(  //VirtualAllocExNuma？
    _In_     HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_     SIZE_T dwSize,
    _In_     DWORD  flAllocationType,
    _In_     DWORD  flProtect
    );
extern "C" _declspec(dllexport) void* _stdcall  MyVirtualAllocEx(
    _In_     HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_     SIZE_T dwSize,
    _In_     DWORD  flAllocationType,
    _In_     DWORD  flProtect
    );

typedef BOOL (_stdcall * VirtualFreeEx_Func)(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD  dwFreeType
    );
extern "C" _declspec(dllexport) BOOL _stdcall  MyVirtualFreeEx(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD  dwFreeType
    );

typedef void* (_stdcall * GlobalAlloc_Func)(
    _In_ UINT   uFlags,
    _In_ SIZE_T dwBytes
    );
extern "C" _declspec(dllexport) void* _stdcall  MyGlobalAlloc(
    _In_ UINT   uFlags,
    _In_ SIZE_T dwBytes
    );

typedef void* (_stdcall * GlobalReAlloc_Func)(
    _In_ HGLOBAL hMem,
    _In_ SIZE_T  dwBytes,
    _In_ UINT    uFlags
    );
extern "C" _declspec(dllexport) void* _stdcall  MyGlobalReAlloc(
    _In_ HGLOBAL hMem,
    _In_ SIZE_T  dwBytes,
    _In_ UINT    uFlags
    );

typedef BOOL (_stdcall * GlobalFree_Func)(
    _In_ HGLOBAL hMem
    );
extern "C" _declspec(dllexport) BOOL _stdcall  MyGlobalFree(
    _In_ HGLOBAL hMem
    );



typedef void* (_stdcall * LocalAlloc_Func)(
    _In_ UINT   uFlags,
    _In_ SIZE_T uBytes
    );
extern "C" _declspec(dllexport) void* _stdcall  MyLocalAlloc(
    _In_ UINT   uFlags,
    _In_ SIZE_T uBytes
    );

typedef void* (_stdcall * LocalReAlloc_Func)(
    _In_ HLOCAL hMem,
    _In_ SIZE_T uBytes,
    _In_ UINT   uFlags
    );
extern "C" _declspec(dllexport) void* _stdcall  MyLocalReAlloc(
    _In_ HLOCAL hMem,
    _In_ SIZE_T uBytes,
    _In_ UINT   uFlags
    );

typedef BOOL (_stdcall * LocalFree_Func)(
    _In_ HLOCAL hMem
    );
extern "C" _declspec(dllexport) BOOL _stdcall  MyLocalFree(
    _In_ HLOCAL hMem
    );

};
HeapAlloc_Func SystemHeapAlloc = NULL;
HeapReAlloc_Func SystemHeapReAlloc = NULL;
HeapAlloc_Func SystemNTHeapAlloc = NULL;
HeapReAlloc_Func SystemNTHeapReAlloc = NULL;
HeapFree_Func SystemHeapFree = NULL;
HeapDestroy_Func SystemHeapDestroy = NULL;

VirtualAllocEx_Func SystemVirtualAllocEx = NULL;
VirtualFreeEx_Func SystemVirtualFreeEx = NULL;

VirtualAlloc_Func SystemVirtualAlloc = NULL;
VirtualFree_Func SystemVirtualFree = NULL;

GlobalAlloc_Func SystemGlobalAlloc = NULL;
GlobalReAlloc_Func SystemGlobalReAlloc = NULL;
GlobalFree_Func SystemGlobalFree = NULL;

LocalAlloc_Func SystemLocalAlloc = NULL;
LocalReAlloc_Func SystemLocalReAlloc = NULL;
LocalFree_Func SystemLocalFree = NULL;


void sendEndInfo();
int walkAllHeaps();
int lockHeaps(bool isLock);

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

#pragma optimize("y", off) // disable FPO

// 轻量级锁，非内核对象
CRITICAL_SECTION g_cs;

struct ThreadSelfBlock 
{
public:
    ThreadSelfBlock()
    {
        nThreadId = -1;
        nPos = 0;
    }
public:
    int nThreadId;
    int nPos;
    char buf[1*1024*1024+256];
    HANDLE hThread;
};

//ThreadSelfBlock gArrBlock[512];
ThreadSelfBlock gArrBlock[1];
const int gnVec = sizeof(gArrBlock)/sizeof(ThreadSelfBlock);
int g_maxPos = 1*1024*1024;


void bindTreadSelfBlock(ThreadSelfBlock*& pThreadBlock)
{
    EnterCriticalSection(&g_cs);
    DWORD dwThreadId = ::GetCurrentThreadId();
    HANDLE hCur = NULL;
    DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),  GetCurrentProcess(),  
                    &hCur, 0, FALSE, DUPLICATE_SAME_ACCESS);
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te32        = {0}; 
    te32.dwSize = sizeof(THREADENTRY32);
    DWORD dwCurProcessId = GetCurrentProcessId();
    DWORD arrThreadI[1024]={0};
    int nThreadIndex = 0;
    //遍历当前进程的线程
    if (Thread32First(hThreadSnap, &te32)) 
    { 
        do 
        { 
            if (te32.th32OwnerProcessID == dwCurProcessId) 
            {
                arrThreadI[nThreadIndex] = te32.th32ThreadID;
                nThreadIndex++;
            } 
        }
        while (Thread32Next(hThreadSnap, &te32));  
    }
    CloseHandle(hThreadSnap);

    for (int i = 0; i<gnVec; i++)
    {
        bool bThreadDead = true;
        for (int j =0; j<nThreadIndex; j++)
        {
            if (gArrBlock[i].nThreadId == arrThreadI[j])
            {
                bThreadDead = false;
                break;
            }
        }
        if (bThreadDead)
        {
            if (gArrBlock[i].nPos > 0)
            {
                g_pWriter->writeData(gArrBlock[i].buf, gArrBlock[i].nPos);
                gArrBlock[i].nPos = 0;
            }

            gArrBlock[i].nThreadId = dwThreadId;
            pThreadBlock = &gArrBlock[i];
            CloseHandle(pThreadBlock->hThread);
            pThreadBlock->hThread = hCur;
            ::TlsSetValue(gdwTlsIndex, (LPVOID)pThreadBlock);
            LeaveCriticalSection(&g_cs);
            return;
        }
    }

    for (int i = 0; i<gnVec; i++)
    {
        if (gArrBlock[i].nThreadId == -1)
        {
            gArrBlock[i].nThreadId = dwThreadId;
            pThreadBlock = &gArrBlock[i];
            pThreadBlock->hThread = hCur;
            ::TlsSetValue(gdwTlsIndex, (LPVOID)pThreadBlock);
            LeaveCriticalSection(&g_cs);
            return;
        }
    }
    ::TlsSetValue(gdwTlsIndex, (LPVOID)pThreadBlock);
    LeaveCriticalSection(&g_cs);
}

//inline __int64 _GetSysTickCount64()  
//{  
//    LARGE_INTEGER TicksPerSecond = { 0 };  
//    LARGE_INTEGER Tick;  
//    if (!TicksPerSecond.QuadPart)  
//        QueryPerformanceFrequency(&TicksPerSecond);  
//    QueryPerformanceCounter(&Tick);  
//    __int64 Seconds = Tick.QuadPart / TicksPerSecond.QuadPart;  
//    __int64 LeftPart = Tick.QuadPart - (TicksPerSecond.QuadPart*Seconds);  
//    __int64 MillSeconds = LeftPart * 1000 / TicksPerSecond.QuadPart;  
//    __int64 Ret = Seconds * 1000 + MillSeconds;  
//    return Ret;  
//}; 


LARGE_INTEGER g_TicksPerSecond = { 0 };    
BOOL bRes = QueryPerformanceFrequency(&g_TicksPerSecond);    

inline __int64 _GetSysTickCount64()  
{
    return 0;
    LARGE_INTEGER TicksPerSecond = { 0 };  
    LARGE_INTEGER Tick;  
    if (!TicksPerSecond.QuadPart)  
        QueryPerformanceFrequency(&TicksPerSecond);  
    QueryPerformanceCounter(&Tick);  
    __int64 Ret = Tick.QuadPart * g_TicksPerSecond.QuadPart / TicksPerSecond.QuadPart;
    return Ret;  
}; 

inline __int64 _GetPerformanceCounter()  
{  
    LARGE_INTEGER Tick;  
    QueryPerformanceCounter(&Tick);
    return Tick.QuadPart;  
}; 

inline __int64 _GetPerformanceCounter2()  
{  
    //struct timeb timebuffer;
    //SYSTEMTIME lt; 
    FILETIME SystemTimeAsFileTime;
    GetSystemTimeAsFileTime(&SystemTimeAsFileTime);
    __int64 time = SystemTimeAsFileTime.dwHighDateTime;
    time = (time << 32) + SystemTimeAsFileTime.dwLowDateTime;
    return time;  
}; 

std::set<void*> g_ptrs;
bool g_isHooking = false;
int g_mallocCount = 0;
void *g_lastPtr = nullptr;
bool g_isFreePtr = false;
bool g_isRealloc = false;

struct ThreadData
{
    bool isHooking;
    int mallocTimes;

    ThreadData()
    {
        isHooking = false;
        mallocTimes = 0;
    }
};

ThreadData *getThreadData()
{
    ThreadData *pThreadData = (ThreadData*)TlsGetValue(g_isHookingTlsIndex);
    if (pThreadData == nullptr)
    {
        // 这里调的系统HeapAlloc，没有调用对象的构造函数，这个要特别注意，需要对成员变量进行初始化
        pThreadData = (ThreadData*) SystemHeapAlloc(GetProcessHeap(), 0, sizeof(ThreadData)); 
        pThreadData->isHooking = false;
        pThreadData->mallocTimes = 0;
        TlsSetValue(g_isHookingTlsIndex, pThreadData);
    }
    return pThreadData;
}

// 调用栈处理线程
class CallStackThread
{
public:
    #pragma optimize("y", off) // disable FPO
    void addCallStack(void *ptr, size_t nSize, __int64 tickCount)
    {
        //ThreadSelfBlock* pThreadBlock = (ThreadSelfBlock*)::TlsGetValue(gdwTlsIndex);
        //if (pThreadBlock == nullptr || pThreadBlock->nThreadId == 0)
        //{
        //    bindTreadSelfBlock(pThreadBlock);
        //}

        ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

        //// 清除重复记录
        //if (pThreadBlock->nPos > 0)
        //{
        //    RTL_STACK_TRACE_ENTRY* lastTrace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + ((pThreadBlock->nPos == 0) ? pThreadBlock->nPos : pThreadBlock->nPos - sizeof(RTL_STACK_TRACE_ENTRY)));
        //    if (lastTrace->Depth != -1 && lastTrace->Ptr == ptr)
        //    {
        //        return;
        //    }
        //}
        
        EnterCriticalSection(&g_cs);

        RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
        try 
        {
            Trace->Depth = RtlCaptureStackBackTrace(0, MAX_STACK_DEPTH, Trace->BackTrace, &Trace->Hash);
            //Trace->Depth = RtlWalkFrameChain(Trace->BackTrace, MAX_STACK_DEPTH, 0);
            //Trace->Depth = dumpStack3(Trace->BackTrace, MAX_STACK_DEPTH);
        }
        catch (...)
        {
            Trace->Depth = 0;
        }

        //Trace->Hash = 0;

        //for (int Index = 0; Index < Trace->Depth; Index += 1) {
        //    Trace->Hash += PtrToUlong (Trace->BackTrace[Index]);
        //}

        if (Trace->Depth > 0)
        {
            __int64 iTickCount64 = tickCount;
            //iTickCount64 = iTickCount64 - g_startTickCount;
            Trace->Size = nSize;
            Trace->Ptr = ptr;
            Trace->systemTime = iTickCount64;
            //Trace->Hash += nSize + Trace->Depth;
            //cout << "\r\nqingh-a, depth, size, ptr=" << (int)Trace.Depth << " " << Trace.Size << " " << (int)Trace.Ptr <<  " " << Trace.Ptr << endl;
            //for (int i=0; i<Trace.Depth; ++i)
            {
                //cout << "qingh-a, address =" << (int)Trace.BackTrace[i] << " " << Trace.BackTrace[i] << endl;
            }
            //system("pause");
            //cout << "qingh-a, depth, size, ptr=" << Trace.Depth << Trace.Size << Trace.Ptr << endl;
            // printf("qingh-a, depth=%d, size=%d, ptr=%d\r\n", Trace.Depth, Trace.Size, Trace.Ptr);

            //memcpy(pThreadBlock->buf+pThreadBlock->nPos, &Trace, sizeof(Trace));
            pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
            if (pThreadBlock->nPos >= g_maxPos)
            {
                g_pWriter->writeData(pThreadBlock->buf, pThreadBlock->nPos);
                pThreadBlock->nPos = 0;
            }

        }

        LeaveCriticalSection(&g_cs); 
    }

    void freeCallStack(void *ptr, __int64 tickCount)
    {
        __int64 iTickCount64 = tickCount;
        //ThreadSelfBlock* pThreadBlock = (ThreadSelfBlock*)::TlsGetValue(gdwTlsIndex);
        //if (pThreadBlock == nullptr || pThreadBlock->nThreadId == 0)
        //{
        //    bindTreadSelfBlock(pThreadBlock);
        //}

        ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

        if (ptr == nullptr)
        {
            return;
        }

        EnterCriticalSection(&g_cs);
        RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
        Trace->Depth = -1;
        Trace->Ptr = ptr;
        Trace->systemTime = iTickCount64;
        //memcpy(pThreadBlock->buf+pThreadBlock->nPos, &Trace, sizeof(Trace));
        pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
        if (pThreadBlock->nPos >= g_maxPos)
        {
            g_pWriter->writeData(pThreadBlock->buf, pThreadBlock->nPos);
            pThreadBlock->nPos = 0;
        }
        LeaveCriticalSection(&g_cs); 
    }
};
CallStackThread g_CallStackThread;


extern "C"
{
extern "C" _declspec(dllexport) void* _stdcall MyHeapAlloc(HANDLE hHeap,
    DWORD dwFlags,
    SIZE_T dwBytes)
{
    //void *ptr = SystemHeapAlloc(hHeap, dwFlags, dwBytes);
    //return ptr;

    {
        //ThreadData *pThreadData = getThreadData();
        //if (pThreadData->isHooking)
        //{
        //    return SystemHeapAlloc(hHeap, dwFlags, dwBytes);
        //}

        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemHeapAlloc(hHeap, dwFlags, dwBytes);
        if (ptr == nullptr || dwBytes <= 0)
        {
            return ptr;
        }

        //pThreadData->isHooking = true;
        g_CallStackThread.addCallStack(ptr, dwBytes, iTickCount64);
        //++pThreadData->mallocTimes;
        //pThreadData->isHooking = false;

        return ptr;
    }
}

extern "C" _declspec(dllexport) void* _stdcall  MyHeapReAlloc(
     __inout HANDLE hHeap,
     __in    DWORD dwFlags,
     __deref LPVOID lpMem,
     __in    SIZE_T dwBytes
     )
 {
     {
         __int64 iTickCount64 = _GetSysTickCount64();
         void *ptr = SystemHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
         // 先释放之前的
         if (lpMem)
         {
             g_CallStackThread.freeCallStack(lpMem, 0);
         }

         // 若未申请成功，则直接返回
         if (ptr == nullptr)
         {
             return ptr;
         }

         g_CallStackThread.addCallStack(ptr, dwBytes, iTickCount64);

         return ptr;
     }
 }

extern "C" _declspec(dllexport) void* _stdcall MyNTHeapAlloc(HANDLE hHeap,
     DWORD dwFlags,
     SIZE_T dwBytes)
 {
     //void *ptr = SystemHeapAlloc(hHeap, dwFlags, dwBytes);
     //return ptr;

     {
         //ThreadData *pThreadData = getThreadData();
         //if (pThreadData->isHooking)
         //{
         //    return SystemHeapAlloc(hHeap, dwFlags, dwBytes);
         //}

         __int64 iTickCount64 = _GetSysTickCount64();
         void *ptr = SystemNTHeapAlloc(hHeap, dwFlags, dwBytes);
         if (ptr == nullptr || dwBytes <= 0)
         {
             return ptr;
         }

         //pThreadData->isHooking = true;
         g_CallStackThread.addCallStack(ptr, dwBytes, iTickCount64);
         //++pThreadData->mallocTimes;
         //pThreadData->isHooking = false;

         return ptr;
     }
 }

extern "C" _declspec(dllexport) void* _stdcall  MyNTHeapReAlloc(
     __inout HANDLE hHeap,
     __in    DWORD dwFlags,
     __deref LPVOID lpMem,
     __in    SIZE_T dwBytes
     )
 {
     {
         // 先释放之前的
         if (lpMem)
         {
             g_CallStackThread.freeCallStack(lpMem, 0);
         }

         __int64 iTickCount64 = _GetSysTickCount64();
         void *ptr = SystemNTHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
         // 若未申请成功，则直接返回
         if (ptr == nullptr)
         {
             return ptr;
         }

         g_CallStackThread.addCallStack(ptr, dwBytes, iTickCount64);

         return ptr;
     }
 }

extern "C" _declspec(dllexport) BOOL _stdcall MyHeapFree(HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem)
{
    //return SystemHeapFree(hHeap, dwFlags, lpMem);

    {
        PVOID ptr = lpMem;
        if (ptr == nullptr)
        {
            return TRUE;
        }

        //ThreadData *pThreadData = getThreadData();
        //if (pThreadData->isHooking)
        //{
        //    return SystemHeapFree(hHeap, dwFlags, lpMem);
        //}

        BOOL res = SystemHeapFree(hHeap, dwFlags, lpMem);

        g_CallStackThread.freeCallStack(ptr, 0);

        return res;
    }
}

extern "C" _declspec(dllexport) BOOL _stdcall MyHeapDestroy(HANDLE hHeap)
{
    //return SystemHeapFree(hHeap, dwFlags, lpMem);

    {
        return SystemHeapDestroy(hHeap);
    }
}

extern "C" _declspec(dllexport) void* _stdcall  MyVirtualAlloc(
    LPVOID lpvAddress, 
    DWORD cbSize,
    DWORD fdwAllocationType,
    DWORD fdwProtect
    )
{
    {
        //ThreadData *pThreadData = getThreadData();
        //if (pThreadData->isHooking)
        //{
        //    return SystemHeapAlloc(hHeap, dwFlags, dwBytes);
        //}

        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemVirtualAlloc(lpvAddress, cbSize, fdwAllocationType, fdwProtect);
        if (ptr == nullptr || cbSize <= 0)
        {
            return ptr;
        }

        //pThreadData->isHooking = true;
        g_CallStackThread.addCallStack(ptr, cbSize, iTickCount64);
        //++pThreadData->mallocTimes;
        //pThreadData->isHooking = false;

        return ptr;
    }
}

extern "C" _declspec(dllexport) BOOL _stdcall MyVirtualFree( LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType )
{
    {
        PVOID ptr = lpAddress;
        if (ptr == nullptr)
        {
            return TRUE;
        }

        //ThreadData *pThreadData = getThreadData();
        //if (pThreadData->isHooking)
        //{
        //    return SystemHeapFree(hHeap, dwFlags, lpMem);
        //}

        BOOL res = SystemVirtualFree(lpAddress, dwSize, dwFreeType);
        g_CallStackThread.freeCallStack(ptr, 0);
        return res;
    }
}

/* VirtualAllocEx */
extern "C" _declspec(dllexport) void* _stdcall  MyVirtualAllocEx(
    _In_     HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_     SIZE_T dwSize,
    _In_     DWORD  flAllocationType,
    _In_     DWORD  flProtect
    )
{
    void *ptr = SystemVirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    if (ptr == nullptr || dwSize <= 0)
    {
        return ptr;
    }

    g_CallStackThread.addCallStack(ptr, dwSize, 0);
    return ptr;
}

extern "C" _declspec(dllexport) BOOL _stdcall  MyVirtualFreeEx(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD  dwFreeType
    )
{
    PVOID ptr = lpAddress;
    if (ptr == nullptr)
    {
        return TRUE;
    }

    BOOL res = SystemVirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
    g_CallStackThread.freeCallStack(ptr, 0);
    return res;
}


/* GlobalAlloc */


extern "C" _declspec(dllexport) void* _stdcall  MyGlobalAlloc(
    _In_ UINT   uFlags,
    _In_ SIZE_T dwBytes
    )
{
    void *ptr = SystemGlobalAlloc(uFlags, dwBytes);
    if (ptr == nullptr || dwBytes <= 0)
    {
        return ptr;
    }

    g_CallStackThread.addCallStack(ptr, dwBytes, 0);
    return ptr;
}

extern "C" _declspec(dllexport) void* _stdcall  MyGlobalReAlloc(
    _In_ HGLOBAL hMem,
    _In_ SIZE_T  dwBytes,
    _In_ UINT    uFlags
    )
{
    {
        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemGlobalReAlloc(hMem, dwBytes, uFlags);
        // 先释放之前的
        if (hMem)
        {
            g_CallStackThread.freeCallStack(hMem, 0);
        }
        // 若未申请成功，则直接返回
        if (ptr == nullptr)
        {
            return ptr;
        }

        g_CallStackThread.addCallStack(ptr, dwBytes, iTickCount64);

        return ptr;
    }
}

extern "C" _declspec(dllexport) BOOL _stdcall  MyGlobalFree(
    _In_ HGLOBAL hMem
    )
{
    PVOID ptr = hMem;
    if (ptr == nullptr)
    {
        return TRUE;
    }

    BOOL res = SystemGlobalFree(hMem);

    g_CallStackThread.freeCallStack(ptr, 0);
    return res;
}

extern "C" _declspec(dllexport) void* _stdcall  MyLocalAlloc(
    _In_ UINT   uFlags,
    _In_ SIZE_T uBytes
    )
{
    void *ptr = SystemLocalAlloc(uFlags, uBytes);
    if (ptr == nullptr || uBytes <= 0)
    {
        return ptr;
    }

    g_CallStackThread.addCallStack(ptr, uBytes, 0);
    return ptr;
}

extern "C" _declspec(dllexport) void* _stdcall  MyLocalReAlloc(
    _In_ HLOCAL hMem,
    _In_ SIZE_T uBytes,
    _In_ UINT   uFlags
    )
{
    {
        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemLocalReAlloc(hMem, uBytes, uFlags);
        // 先释放之前的
        if (hMem)
        {
            g_CallStackThread.freeCallStack(hMem, 0);
        }
        // 若未申请成功，则直接返回
        if (ptr == nullptr)
        {
            return ptr;
        }

        g_CallStackThread.addCallStack(ptr, uBytes, iTickCount64);

        return ptr;
    }
}

extern "C" _declspec(dllexport) BOOL _stdcall  MyLocalFree(
    _In_ HLOCAL hMem
    )
{
    PVOID ptr = hMem;
    if (ptr == nullptr)
    {
        return TRUE;
    }

    BOOL res = SystemLocalFree(hMem);

    g_CallStackThread.freeCallStack(ptr, 0);
    return res;
}

}

extern "C" _declspec(dllexport) void* MyHookMalloc100(size_t nSize)
{
    //void *ptr = SystemMallocFunc(nSize);
    //if (ptr == nullptr || nSize <= 0)
    //{
    //    return ptr;
    //}
    //g_CallStackThread.addCallStack(ptr, nSize);

    //return ptr;


    {
        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemMallocFunc100(nSize);
        if (ptr == nullptr || nSize <= 0)
        {
            return ptr;
        }

        //g_CallStackThread.addCallStack(ptr, nSize, iTickCount64);

        return ptr;
    }

}

extern "C" _declspec(dllexport) void* MyHookMalloc(size_t nSize)
{
    //void *ptr = SystemMallocFunc(nSize);
    //if (ptr == nullptr || nSize <= 0)
    //{
    //    return ptr;
    //}
    //g_CallStackThread.addCallStack(ptr, nSize);

    //return ptr;


    {
        //ThreadData *pThreadData = getThreadData();
        //if (pThreadData->isHooking)
        //{
        //    return SystemMallocFunc(nSize);
        //}

        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemMallocFunc(nSize);
        if (ptr == nullptr || nSize <= 0)
        {
            return ptr;
        }
        
        //g_CallStackThread.addCallStack(ptr, nSize, iTickCount64);

        return ptr;
    }
    
}

extern "C" _declspec(dllexport) void* MyHookRealloc(void * _Memory, _In_ size_t _NewSize)
{
    //// 先清除原来的记录
    //if (_Memory)
    //{
    //    g_CallStackThread.freeCallStack(_Memory);
    //}

    //void *ptr = SystemReallocFunc(_Memory, _NewSize);
    //if (ptr == nullptr || _NewSize <= 0 || _Memory == nullptr)
    //{
    //    return ptr;
    //}
    ////g_CallStackThread.addCallStack(ptr, _NewSize);

    //return ptr;

    {
        // 先清除原来的记录
        if (_Memory)
        {
            __int64 iTickCount64 = _GetSysTickCount64();
            //g_CallStackThread.freeCallStack(_Memory, iTickCount64);
        }

        void *ptr = SystemReallocFunc(_Memory, _NewSize);

        if (ptr == nullptr || _NewSize <= 0)
        {
            return ptr;
        }
        
        //pThreadData->isHooking = true;
        ////g_CallStackThread.addCallStack(ptr, _NewSize, iTickCount64);
        //pThreadData->isHooking = false;

        return ptr;
    }
}

extern "C" _declspec(dllexport) void* MyHookReCalloc(void * _Memory, size_t _Count, size_t _Size)
{
    //// 先清除原来的记录
    //if (_Memory)
    //{
    //    g_CallStackThread.freeCallStack(_Memory);
    //}

    //void *ptr = SystemReCallocFunc(_Memory, _Count, _Size);
    //if (ptr == nullptr || _ount * _Size <= 0)
    //{
    //    return ptr;
    //}
    //g_CallStackThread.addCallStack(ptr, _Count * _Size);

    //return ptr;

    {
        // 先清除原来的记录
        if (_Memory)
        {
            //__int64 iTickCount64 = _GetSysTickCount64();
            //g_CallStackThread.freeCallStack(_Memory, iTickCount64);
        }

        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemReCallocFunc(_Memory, _Count, _Size);

        // 强制清除
        //if (_Memory)
        //{
        //    pThreadData->isHooking = true;
        //    __int64 iTickCount64 = _GetSysTickCount64();
        //    g_CallStackThread.freeCallStack(ptr, iTickCount64);
        //    pThreadData->isHooking = false;
        //    return ptr;
        //}

        if (ptr == nullptr || _Count * _Size <= 0)
        {
            return ptr;
        }

        //g_CallStackThread.addCallStack(ptr, _Count * _Size, iTickCount64);

        return ptr;
    }
}

extern "C" _declspec(dllexport) void* MyHookCalloc(size_t _Count, size_t _Size)
{
    //void *ptr = SystemCallocFunc(_Count, _Size);

    //if (ptr == nullptr || _Count * _Size <= 0)
    //{
    //    return ptr;
    //}
    //g_CallStackThread.addCallStack(ptr, _Count * _Size);

    //return ptr;

    {
        __int64 iTickCount64 = _GetSysTickCount64();
        void *ptr = SystemCallocFunc(_Count, _Size);
        if (ptr == nullptr || _Count * _Size <= 0)
        {
            return ptr;
        }

        //g_CallStackThread.addCallStack(ptr, _Count * _Size, iTickCount64);

        return ptr;
    }
}

extern "C" _declspec(dllexport) void MyHookFree(void *ptr)
{
    //if (ptr == nullptr)
    //{
    //    return;
    //}
    //g_CallStackThread.freeCallStack(ptr);

    //return SystemFreeFunc(ptr);

    {
        if (ptr == nullptr)
        {
            return;
        }

        __int64 iTickCount64 = _GetSysTickCount64();
        //g_CallStackThread.freeCallStack(ptr, iTickCount64);

        return SystemFreeFunc(ptr);
    }
}

// test hook tcmalloc
extern "C" _declspec(dllexport) void* MyInvokeNewHook(const void* p, size_t s)
{
    ThreadData *pThreadData = getThreadData();
    if (pThreadData->isHooking)
    {
        return TCMalloc_InvokeNewHook(p, s);
    }

    __int64 iTickCount64 = _GetSysTickCount64();
    if (p == nullptr || s <= 0)
    {
        return TCMalloc_InvokeNewHook(p, s);
    }

    pThreadData->isHooking = true;
    g_CallStackThread.addCallStack((PVOID)p, s, iTickCount64);
    pThreadData->isHooking = false;

    return TCMalloc_InvokeNewHook(p, s);
}

void MyTcmalloc()
{
    return;
}

static int s_realHeapCount = 0;
static int s_realHeapSize = 0;
static int s_dealwithHeapMode = 0; // 0-hook前对内存统计；1-当前堆内存发送给统计工具。

HANDLE ghEndEvent;
#define GLOBAL_STATICS_FINISH_EVENT  ("Local\\GLOBAL_STATICS_FINISH_EVENT") 
DWORD WINAPI waitEndThread(LPVOID lp)
{
    bool bWait = false;
    while (true)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(ghEndEvent, INFINITE))
        {
            EnterCriticalSection(&g_cs);
            if (!bWait)
            {
                //bWait = true;
                lockHeaps(true);
                // 放开锁以使锁定内存堆前没有记录的内存信息进行记录，保证内存记录信息的百分之百正确，然后再锁上
                LeaveCriticalSection(&g_cs);
                ::Sleep(500);
                EnterCriticalSection(&g_cs);
                // test hook tcmalloc
                //TCMalloc_InvokeNewHook = (InvokeNewHook_Func)DetourFindFunction("libtcmalloc_minimal.dll","tc_malloc");
                //TCMalloc_InvokeNewHook = (InvokeNewHook_Func)DetourFindFunction("libtcmalloc_minimal.dll","?InvokeNewHook@MallocHook@@SAXPBXI@Z");
                //DetourAttach(&(PVOID&)TCMalloc_InvokeNewHook, MyInvokeNewHook);

                //TCMalloc_tcmalloc = (tcmalloc_Func)DetourFindFunction("libtcmalloc_minimal.dll","_tcmalloc");
                //DetourAttach(&(PVOID&)TCMalloc_tcmalloc, MyTcmalloc);

                //new char[1234];
                for (int i = 0; i<gnVec; i++)
                {
                    if (gArrBlock[i].nThreadId != -1)
                    {
                        //::SuspendThread(gArrBlock[i].hThread);
                        g_pWriter->writeData(gArrBlock[i].buf, gArrBlock[i].nPos);
                        gArrBlock[i].nPos = 0;
                    }

                }
                if (gArrBlock[0].nPos > 0)
                {
                    g_pWriter->writeData(gArrBlock[0].buf, gArrBlock[0].nPos);
                    gArrBlock[0].nPos = 0;
                }

                static bool s_bFirst = true;
                if (s_bFirst)
                {
                    s_bFirst = false;
                    s_realHeapCount = 0;
                    s_realHeapSize = 0;
                    s_dealwithHeapMode = 1;
                    walkAllHeaps();
                    // 发送给内存统计程序在hook之前已有的内存数：
                    {
                        ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

                        RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
                        Trace->Depth = -3; // -3代表传递内存数量
                        Trace->Ptr = (PVOID)s_realHeapSize; // 用ptr来存储内存量
                        Trace->Size = s_realHeapCount; // 用size来存储内存次数
                        pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
                    }
                    // 发送给内存统计程序代表数据的结尾：
                    sendEndInfo();
                    if (gArrBlock[0].nPos > 0)
                    {
                        g_pWriter->writeData(gArrBlock[0].buf, gArrBlock[0].nPos);
                        gArrBlock[0].nPos = 0;
                    }
                }
                else
                {
                    s_realHeapCount = 0;
                    s_realHeapSize = 0;
                    s_dealwithHeapMode = 1;
                    walkAllHeaps();
                    // 发送给内存统计程序在hook之前已有的内存数：
                    {
                        ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

                        RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
                        Trace->Depth = -3; // -3代表传递内存数量
                        Trace->Ptr = (PVOID)s_realHeapSize; // 用ptr来存储内存量
                        Trace->Size = s_realHeapCount; // 用size来存储内存次数
                        pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
                    }
                    // 发送给内存统计程序代表数据的结尾：
                    sendEndInfo();
                    if (gArrBlock[0].nPos > 0)
                    {
                        g_pWriter->writeData(gArrBlock[0].buf, gArrBlock[0].nPos);
                        gArrBlock[0].nPos = 0;
                    }
                }
                lockHeaps(false);
            }
            else
            {
                bWait = false;
                for (int i = 0; i<gnVec; i++)
                {
                    if (gArrBlock[i].nThreadId != -1)
                    {
                        //::ResumeThread(gArrBlock[i].hThread);
                    }
                }
                //::Sleep(100);
            }
            LeaveCriticalSection(&g_cs);
            ::ResetEvent(ghEndEvent);
        }
    }
}

int walkMyHeap()
{
    DWORD LastError;

    HANDLE hHeap;

    PROCESS_HEAP_ENTRY Entry;



    //

    // Create a new heap with default parameters.

    //

    hHeap = HeapCreate(0, 0, 0);
    //hHeap = GetProcessHeap();

    if (hHeap == NULL) {

        //_tprintf(TEXT("Failed to create a new heap with LastError %d./n"),

        //    GetLastError());

        return 1;

    }



    //

    // Lock the heap to prevent other threads from accessing the heap

    // during enumeration.

    int *p1 = (int*)HeapAlloc(hHeap, 0, 4);

    //

    if (HeapLock(hHeap) == FALSE) {

        //_tprintf(TEXT("Failed to lock heap with LastError %d./n"),

        //    GetLastError());

        return 1;

    }



    //_tprintf(TEXT("Walking heap %#p.../n/n"), hHeap);



    Entry.lpData = NULL;

    while (HeapWalk(hHeap, &Entry) != FALSE) {

        if ((Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {

            //_tprintf(TEXT("Allocated block"));



            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) != 0) {

                //_tprintf(TEXT(", movable with HANDLE %#p"), Entry.Block.hMem);

            }



            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_DDESHARE) != 0) {

                //_tprintf(TEXT(", DDESHARE"));

            }

        }

        else if ((Entry.wFlags & PROCESS_HEAP_REGION) != 0) {

            //_tprintf(TEXT("Region/n  %d bytes committed/n") /

            //    TEXT("  %d bytes uncommitted/n  First block address: %#p/n") /

            //    TEXT("  Last block address: %#p/n"),

            //    Entry.Region.dwCommittedSize,

            //    Entry.Region.dwUnCommittedSize,

            //    Entry.Region.lpFirstBlock,

            //    Entry.Region.lpLastBlock);

        }

        else if ((Entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) != 0) {

            //_tprintf(TEXT("Uncommitted range/n"));

        }

        else {

            //_tprintf(TEXT("Block/n"));

        }



        //_tprintf(TEXT("  Data portion begins at: %#p/n  Size: %d bytes/n") /

        //    TEXT("  Overhead: %d bytes/n  Region index: %d/n/n"),

        //    Entry.lpData,

        //    Entry.cbData,

        //    Entry.cbOverhead,

        //    Entry.iRegionIndex);

    }

    LastError = GetLastError();

    if (LastError != ERROR_NO_MORE_ITEMS) {

        //_tprintf(TEXT("HeapWalk failed with LastError %d./n"), LastError);

    }



    //

    // Unlock the heap to allow other threads to access the heap after

    // enumeration has completed.

    //

    if (HeapUnlock(hHeap) == FALSE) {

        //_tprintf(TEXT("Failed to unlock heap with LastError %d./n"),

        //    GetLastError());

    }



    //

    // When a process terminates, allocated memory is reclaimed by the operating

    // system so it is not really necessary to call HeapDestroy in this example.

    // However, it may be advisable to call HeapDestroy in a longer running

    // application.

    //

    if (HeapDestroy(hHeap) == FALSE) {

        //_tprintf(TEXT("Failed to destroy heap with LastError %d./n"),

        //    GetLastError());

    }



    return 0;
}


int walkHeap(HANDLE hHeap)
{
    DWORD LastError;

    PROCESS_HEAP_ENTRY Entry = {0};

    //if (HeapLock(hHeap) == FALSE) {

    //    //_tprintf(TEXT("Failed to lock heap with LastError %d./n"),

    //    //    GetLastError());

    //    return 1;

    //}

    //_tprintf(TEXT("Walking heap %#p.../n/n"), hHeap);



    Entry.lpData = NULL;
    Entry.wFlags = 0;

    PVOID lastLpData = NULL;
    while (HeapWalk(hHeap, &Entry) != FALSE) {
        //if (Entry.lpData == lastLpData)
        //{
        //    break;
        //}
        //lastLpData = Entry.lpData;

        //// 发送给内存统计程序：
        //{
        //    ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

        //    RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
        //    Trace->Depth = -1;
        //    Trace->Ptr = Entry.lpData;
        //    Trace->Size = Entry.cbData;
        //    //memcpy(pThreadBlock->buf+pThreadBlock->nPos, &Trace, sizeof(Trace));
        //    pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
        //    if (pThreadBlock->nPos >= g_maxPos)
        //    {
        //        g_pWriter->writeData(pThreadBlock->buf, pThreadBlock->nPos);
        //        pThreadBlock->nPos = 0;
        //    }
        //}

        if ((Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {

            //_tprintf(TEXT("Allocated block"));


            s_realHeapCount++;
            s_realHeapSize += Entry.cbData;
            // 发送给内存统计程序：
            if (s_dealwithHeapMode == 1)
            {
                ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

                RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
                Trace->Depth = -2;
                Trace->Ptr = Entry.lpData;
                Trace->Size = Entry.cbData;
                //memcpy(pThreadBlock->buf+pThreadBlock->nPos, &Trace, sizeof(Trace));
                pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
                if (pThreadBlock->nPos >= g_maxPos)
                {
                    g_pWriter->writeData(pThreadBlock->buf, pThreadBlock->nPos);
                    pThreadBlock->nPos = 0;
                }
            }



            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) != 0) {

                //_tprintf(TEXT(", movable with HANDLE %#p"), Entry.Block.hMem);

            }



            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_DDESHARE) != 0) {

                //_tprintf(TEXT(", DDESHARE"));

            }

        }

        else if ((Entry.wFlags & PROCESS_HEAP_REGION) != 0) {

            //_tprintf(TEXT("Region/n  %d bytes committed/n") /

            //    TEXT("  %d bytes uncommitted/n  First block address: %#p/n") /

            //    TEXT("  Last block address: %#p/n"),

            //    Entry.Region.dwCommittedSize,

            //    Entry.Region.dwUnCommittedSize,

            //    Entry.Region.lpFirstBlock,

            //    Entry.Region.lpLastBlock);

        }

        else if ((Entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) != 0) {

            //_tprintf(TEXT("Uncommitted range/n"));

        }

        else {

            //_tprintf(TEXT("Block/n"));

        }



        //_tprintf(TEXT("  Data portion begins at: %#p/n  Size: %d bytes/n") /

        //    TEXT("  Overhead: %d bytes/n  Region index: %d/n/n"),

        //    Entry.lpData,

        //    Entry.cbData,

        //    Entry.cbOverhead,

        //    Entry.iRegionIndex);

    }

    LastError = GetLastError();

    if (LastError != ERROR_NO_MORE_ITEMS) {

        //_tprintf(TEXT("HeapWalk failed with LastError %d./n"), LastError);

    }



    //

    // Unlock the heap to allow other threads to access the heap after

    // enumeration has completed.

    //

    //if (HeapUnlock(hHeap) == FALSE) {

    //    //_tprintf(TEXT("Failed to unlock heap with LastError %d./n"),

    //    //    GetLastError());

    //}



    //

    // When a process terminates, allocated memory is reclaimed by the operating

    // system so it is not really necessary to call HeapDestroy in this example.

    // However, it may be advisable to call HeapDestroy in a longer running

    // application.

    //

    //if (HeapDestroy(hHeap) == FALSE) {

    //    //_tprintf(TEXT("Failed to destroy heap with LastError %d./n"),

    //    //    GetLastError());

    //}



    return 0;
}

int lockHeaps(bool isLock)
{
    DWORD NumberOfHeaps;

    DWORD HeapsIndex;

    DWORD HeapsLength;

    HANDLE hDefaultProcessHeap;

    HRESULT Result;

    PHANDLE aHeaps;

    SIZE_T BytesToAllocate;

    NumberOfHeaps = GetProcessHeaps(0, NULL);

    if (NumberOfHeaps == 0) {
        return 1;
    }

    hDefaultProcessHeap = GetProcessHeap();

    if (hDefaultProcessHeap == NULL) {
        return 1;

    }

    BytesToAllocate = NumberOfHeaps * sizeof(*aHeaps);

    //aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    static char buf[1024*1024];
    aHeaps = (PHANDLE)buf;
    if (BytesToAllocate > 1024*1024)
    {
        aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    }

    if (aHeaps == NULL) {
        return 1;
    }

    HeapsLength = NumberOfHeaps;


    NumberOfHeaps = GetProcessHeaps(HeapsLength, aHeaps);
    if (NumberOfHeaps == 0) {
        return 1;
    }

    else if (NumberOfHeaps > HeapsLength) {
        return 1;
    }



    //_tprintf(TEXT("Process has %d heaps./n"), HeapsLength);

    for (HeapsIndex = 0; HeapsIndex < HeapsLength; ++HeapsIndex) {

        if (isLock)
        {
            HeapLock(aHeaps[HeapsIndex]);
        }
        else
        {
            HeapUnlock(aHeaps[HeapsIndex]);
        }
    }

    if (BytesToAllocate > 1024*1024)
    {
        if (HeapFree(hDefaultProcessHeap, 0, aHeaps) == FALSE) {
            //_tprintf(TEXT("Failed to free allocation from default process heap./n"));
        }
    }


    return 0;
}

int walkAllHeaps()
{
    DWORD NumberOfHeaps;

    DWORD HeapsIndex;

    DWORD HeapsLength;

    HANDLE hDefaultProcessHeap;

    HRESULT Result;

    PHANDLE aHeaps;

    SIZE_T BytesToAllocate;



    //

    // Retrieve the number of active heaps for the current process

    // so we can calculate the buffer size needed for the heap handles.

    //

    NumberOfHeaps = GetProcessHeaps(0, NULL);

    if (NumberOfHeaps == 0) {

        //_tprintf(TEXT("Failed to retrieve the number of heaps with LastError %d./n"),

        //    GetLastError());

        return 1;

    }



    //

    // Get a handle to the default process heap.

    //

    hDefaultProcessHeap = GetProcessHeap();

    if (hDefaultProcessHeap == NULL) {

        //_tprintf(TEXT("Failed to retrieve the default process heap with LastError %d./n"),

        //    GetLastError());

        return 1;

    }



    //

    // Allocate the buffer from the default process heap.

    //

    BytesToAllocate = NumberOfHeaps * sizeof(*aHeaps);

    //aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    static char buf[1024*1024];
    aHeaps = (PHANDLE)buf;
    if (BytesToAllocate > 1024*1024)
    {
        aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    }

    if (aHeaps == NULL) {

        //_tprintf(TEXT("HeapAlloc failed to allocate %d bytes./n"),

        //    BytesToAllocate);

        return 1;

    }



    //

    // Save the original number of heaps because we are going to compare it

    // to the return value of the next GetProcessHeaps call.

    //

    HeapsLength = NumberOfHeaps;



    //

    // Retrieve handles to the process heaps and print them to stdout.

    // Note that heap functions should be called only on the default heap of the process

    // or on private heaps that your component creates by calling HeapCreate.

    //

    NumberOfHeaps = GetProcessHeaps(HeapsLength, aHeaps);

    if (NumberOfHeaps == 0) {

        //_tprintf(TEXT("Failed to retrieve heaps with LastError %d./n"),

        //    GetLastError());

        return 1;

    }

    else if (NumberOfHeaps > HeapsLength) {



        //

        // Compare the latest number of heaps with the original number of heaps.

        // If the latest number is larger than the original number, another

        // component has created a new heap and the buffer is too small.

        //

        //_tprintf(TEXT("Another component created a heap between calls. ") /

        //    TEXT("Please try again./n"));

        return 1;

    }



    //_tprintf(TEXT("Process has %d heaps./n"), HeapsLength);

    for (HeapsIndex = 0; HeapsIndex < HeapsLength; ++HeapsIndex) {

        //_tprintf(TEXT("Heap %d at address: %#p./n"),

        //    HeapsIndex,

        //    aHeaps[HeapsIndex]);

        walkHeap(aHeaps[HeapsIndex]);

    }



    //

    // Release memory allocated from default process heap.

    //

    if (BytesToAllocate > 1024*1024)
    {
        if (HeapFree(hDefaultProcessHeap, 0, aHeaps) == FALSE) {
            //_tprintf(TEXT("Failed to free allocation from default process heap./n"));
        }
    }
    

    return 0;
}

HANDLE ghProcessExitEvent;
#define GLOBAL_PROCESS_EXIT_EVENT  ("Local\\GLOBAL_PROCESS_EXIT_EVENT") 
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            CreateThread(nullptr, 0, waitEndThread, nullptr, 0, nullptr);
            ::InitializeCriticalSection(&g_cs);
            ghEndEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_FINISH_EVENT);
            ghProcessExitEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_PROCESS_EXIT_EVENT);
            gdwTlsIndex = ::TlsAlloc();
            g_isHookingTlsIndex = ::TlsAlloc();
            g_pWriter = new Client();
            g_pWriter->init(1024*1024*12);
            DetourRestoreAfterWith();
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            //这里可以连续多次调用DetourAttach，表明HOOK多个函数
            // HeapAlloc
            //SystemNTHeapAlloc = (HeapAlloc_Func)DetourFindFunction("ntdll.dll","RtlAllocateHeap");
            //DetourAttach(&(PVOID&)SystemNTHeapAlloc,MyNTHeapAlloc);

            //SystemNTHeapReAlloc = (HeapReAlloc_Func)DetourFindFunction("ntdll.dll","RtlReAllocateHeap");
            //DetourAttach(&(PVOID&)SystemNTHeapReAlloc,MyNTHeapReAlloc);

            SystemHeapAlloc = (HeapAlloc_Func)DetourFindFunction("Kernel32.dll","HeapAlloc");
            DetourAttach(&(PVOID&)SystemHeapAlloc,MyHeapAlloc);

            SystemHeapReAlloc = (HeapReAlloc_Func)DetourFindFunction("Kernel32.dll","HeapReAlloc");
            DetourAttach(&(PVOID&)SystemHeapReAlloc,MyHeapReAlloc);

            SystemHeapFree = (HeapFree_Func)DetourFindFunction("Kernel32.dll","HeapFree");
            DetourAttach(&(PVOID&)SystemHeapFree,MyHeapFree);

            SystemHeapDestroy = (HeapDestroy_Func)DetourFindFunction("Kernel32.dll","HeapDestroy");
            DetourAttach(&(PVOID&)SystemHeapDestroy,MyHeapDestroy);

            // VirtualAlloc
            SystemVirtualAlloc = (VirtualAlloc_Func)DetourFindFunction("Kernel32.dll","VirtualAlloc");
            DetourAttach(&(PVOID&)SystemVirtualAlloc,MyVirtualAlloc);

            SystemVirtualFree = (VirtualFree_Func)DetourFindFunction("Kernel32.dll","VirtualFree");
            DetourAttach(&(PVOID&)SystemVirtualFree,MyVirtualFree);

            // VirtualAllocEx
            SystemVirtualAllocEx = (VirtualAllocEx_Func)DetourFindFunction("Kernel32.dll","VirtualAllocEx");
            DetourAttach(&(PVOID&)SystemVirtualAllocEx,MyVirtualAllocEx);

            SystemVirtualFreeEx = (VirtualFreeEx_Func)DetourFindFunction("Kernel32.dll","VirtualFreeEx");
            DetourAttach(&(PVOID&)SystemVirtualFreeEx,MyVirtualFreeEx);

            // GlobalAlloc
            SystemGlobalAlloc = (GlobalAlloc_Func)DetourFindFunction("Kernel32.dll","GlobalAlloc"); 
            DetourAttach(&(PVOID&)SystemGlobalAlloc,MyGlobalAlloc);

            SystemGlobalReAlloc = (GlobalReAlloc_Func)DetourFindFunction("Kernel32.dll","GlobalReAlloc"); 
            DetourAttach(&(PVOID&)SystemGlobalReAlloc,MyGlobalReAlloc);

            SystemGlobalFree = (GlobalFree_Func)DetourFindFunction("Kernel32.dll","GlobalFree");
            DetourAttach(&(PVOID&)SystemGlobalFree,MyGlobalFree);

            // LocalAlloc
            SystemLocalAlloc = (LocalAlloc_Func)DetourFindFunction("Kernel32.dll","LocalAlloc");
            DetourAttach(&(PVOID&)SystemLocalAlloc,MyLocalAlloc);

            SystemLocalReAlloc = (LocalReAlloc_Func)DetourFindFunction("Kernel32.dll","LocalReAlloc");
            DetourAttach(&(PVOID&)SystemLocalReAlloc,MyLocalReAlloc);

            SystemLocalFree = (LocalFree_Func)DetourFindFunction("Kernel32.dll","LocalFree");
            DetourAttach(&(PVOID&)SystemLocalFree,MyLocalFree);

            
            //SystemMallocFunc = (Malloc_Func)DetourFindFunction("msvcr90","malloc");
            //DetourAttach(&(PVOID&)SystemMallocFunc,MyHookMalloc);
            //SystemMallocFunc = (Malloc_Func)DetourFindFunction("msvcr120","malloc");
            //DetourAttach(&(PVOID&)SystemMallocFunc,MyHookMalloc);
            //SystemMallocFunc100 = (Malloc_Func)DetourFindFunction("msvcr100","malloc");
            //if (SystemMallocFunc100)
            //DetourAttach(&(PVOID&)SystemMallocFunc100,MyHookMalloc100);
            //SystemMallocFunc = (Malloc_Func)DetourFindFunction("msvcrt","malloc");
            //if (SystemMallocFunc)
            //    DetourAttach(&(PVOID&)SystemMallocFunc,MyHookMalloc);

            //SystemReallocFunc = (Realloc_Func)DetourFindFunction("msvcrt","realloc");
            //DetourAttach(&(PVOID&)SystemReallocFunc,MyHookRealloc);
            //SystemReallocFunc = (Realloc_Func)DetourFindFunction("msvcr90","realloc");
            //DetourAttach(&(PVOID&)SystemReallocFunc,MyHookRealloc);
            //SystemReallocFunc = (Realloc_Func)DetourFindFunction("msvcr120","realloc");
            //DetourAttach(&(PVOID&)SystemReallocFunc,MyHookRealloc);
            //SystemReallocFunc = (Realloc_Func)DetourFindFunction("msvcr100","realloc");
            //DetourAttach(&(PVOID&)SystemReallocFunc,MyHookRealloc);

            //SystemCallocFunc = (Calloc_Func)DetourFindFunction("msvcrt","calloc");
            //DetourAttach(&(PVOID&)SystemCallocFunc,MyHookCalloc);
            //SystemCallocFunc = (Calloc_Func)DetourFindFunction("msvcr90","calloc");
            //DetourAttach(&(PVOID&)SystemCallocFunc,MyHookCalloc);
            //SystemCallocFunc = (Calloc_Func)DetourFindFunction("msvcr120","calloc");
            //DetourAttach(&(PVOID&)SystemCallocFunc,MyHookCalloc);
            //SystemCallocFunc = (Calloc_Func)DetourFindFunction("msvcr100","calloc");
            //DetourAttach(&(PVOID&)SystemCallocFunc,MyHookCalloc);

            //SystemReCallocFunc = (ReCalloc_Func)DetourFindFunction("msvcrt","_recalloc"); //_recalloc?
            //DetourAttach(&(PVOID&)SystemReCallocFunc,MyHookReCalloc);
            //SystemReCallocFunc = (ReCalloc_Func)DetourFindFunction("msvcr90","_recalloc"); //_recalloc?
            //DetourAttach(&(PVOID&)SystemReCallocFunc,MyHookReCalloc);
            //SystemReCallocFunc = (ReCalloc_Func)DetourFindFunction("msvcr120","_recalloc"); //_recalloc?
            //DetourAttach(&(PVOID&)SystemReCallocFunc,MyHookReCalloc);
            //SystemReCallocFunc = (ReCalloc_Func)DetourFindFunction("msvcr100","_recalloc"); //_recalloc?
            //DetourAttach(&(PVOID&)SystemReCallocFunc,MyHookReCalloc);


            //SystemFreeFunc = (Free_Func)DetourFindFunction("msvcrt","free");
            //DetourAttach(&(PVOID&)SystemFreeFunc,MyHookFree);
            //SystemFreeFunc = (Free_Func)DetourFindFunction("msvcr90","free");
            //DetourAttach(&(PVOID&)SystemFreeFunc,MyHookFree);
            //SystemFreeFunc = (Free_Func)DetourFindFunction("msvcr120","free");
            //DetourAttach(&(PVOID&)SystemFreeFunc,MyHookFree);
            //SystemFreeFunc = (Free_Func)DetourFindFunction("msvcr100","free");
            //DetourAttach(&(PVOID&)SystemFreeFunc,MyHookFree);


            //// test hook tcmalloc
            //TCMalloc_InvokeNewHook = (InvokeNewHook_Func)DetourFindFunction("libtcmalloc_minimal.dll","?InvokeNewHook@MallocHook@@SAXPBXI@Z");
            //DetourAttach(&(PVOID&)TCMalloc_InvokeNewHook, MyInvokeNewHook);

            //TCMalloc_tcmalloc = (tcmalloc_Func)DetourFindFunction("libtcmalloc_minimal.dll","_tcmalloc");
            //DetourAttach(&(PVOID&)TCMalloc_tcmalloc, MyTcmalloc);

            DetourTransactionCommit();

            // 发送hook前内存信息
            {
                EnterCriticalSection(&g_cs);
                lockHeaps(true);
                // 放开锁以使锁定内存堆前没有记录的内存信息进行记录，保证内存记录信息的百分之百正确，然后再锁上
                LeaveCriticalSection(&g_cs);
                ::Sleep(500);
                EnterCriticalSection(&g_cs);

                s_realHeapSize = 0;
                s_realHeapCount = 0;
                s_dealwithHeapMode = 1;
                walkAllHeaps();
                // 发送给内存统计程序在hook之前已有的内存数：
                {
                    ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

                    RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
                    Trace->Depth = -3; // -3代表传递内存数量
                    Trace->Ptr = (PVOID)s_realHeapSize; // 用ptr来存储内存量
                    Trace->Size = s_realHeapCount; // 用size来存储内存次数
                    pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
                }
                if (gArrBlock[0].nPos > 0)
                {
                    g_pWriter->writeData(gArrBlock[0].buf, gArrBlock[0].nPos);
                    gArrBlock[0].nPos = 0;
                }

                lockHeaps(false);
                LeaveCriticalSection(&g_cs);
            }
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        {
            EnterCriticalSection(&g_cs);
            lockHeaps(true);
            // 放开锁以使锁定内存堆前没有记录的内存信息进行记录，保证内存记录信息的百分之百正确，然后再锁上
            LeaveCriticalSection(&g_cs);
            ::Sleep(500);
            EnterCriticalSection(&g_cs);

            s_realHeapSize = 0;
            s_realHeapCount = 0;
            s_dealwithHeapMode = 1;
            walkAllHeaps();
            // 发送给内存统计程序在hook之前已有的内存数：
            {
                ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

                RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
                Trace->Depth = -3; // -3代表传递内存数量
                Trace->Ptr = (PVOID)s_realHeapSize; // 用ptr来存储内存量
                Trace->Size = s_realHeapCount; // 用size来存储内存次数
                pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
            }
            // 发送给内存统计程序代表数据的结尾：
            sendEndInfo();
            if (gArrBlock[0].nPos > 0)
            {
                g_pWriter->writeData(gArrBlock[0].buf, gArrBlock[0].nPos);
                gArrBlock[0].nPos = 0;
            }

            ::SetEvent(ghProcessExitEvent);
            while (WAIT_OBJECT_0 == WaitForSingleObject(ghProcessExitEvent, 100))
            {
                ::Sleep(100);
            }
            lockHeaps(false);
            LeaveCriticalSection(&g_cs);

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            //这里可以连续多次调用DetourDetach,表明撤销多个函数HOOK
            DetourDetach(&(PVOID&)SystemMallocFunc,MyHookMalloc);
            DetourDetach(&(PVOID&)SystemFreeFunc,MyHookFree);
            DetourDetach(&(PVOID&)SystemReallocFunc,MyHookRealloc);
            DetourDetach(&(PVOID&)SystemCallocFunc,MyHookCalloc);
            DetourDetach(&(PVOID&)SystemReCallocFunc,MyHookReCalloc);

            DetourTransactionCommit();
            CloseHandle(ghEndEvent);
        }
    	break;
    }
    return TRUE;
}

void sendEndInfo()
{
    // 发送给内存统计程序代表数据的结尾：
    {
        ThreadSelfBlock* pThreadBlock = &gArrBlock[0];

        RTL_STACK_TRACE_ENTRY* Trace = (RTL_STACK_TRACE_ENTRY*)(pThreadBlock->buf + pThreadBlock->nPos);
        Trace->Depth = -4; // -4代表数据结尾
        Trace->Ptr = (PVOID)1; // 指针值随意，不是null就行
        Trace->Size = 1; // 随意，不是0就行
        pThreadBlock->nPos += sizeof(RTL_STACK_TRACE_ENTRY);
    }
}