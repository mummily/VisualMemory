#include "Utils.h"
#include "math.h"

#include <windows.h>
#include <tlhelp32.h>
#include <assert.h>
#include <iostream>
#include<stdlib.h>
#include <QDir>
#include <QDebug>

#include "DbgHelp.h"
#include <stdio.h>
#include <tchar.h>

#include "GTJCommon.h"
using namespace std;
using namespace ggp;
#define ASSERT assert
//#define _T

static QMap<DWORD, QString> s_modules;


//double Utils::getRealMemoryRatio( int singleSize )
//{
//    singleSize = abs(singleSize);
//    double dRatio = 1;
//    if (singleSize <= 4)
//    {
//        dRatio = 6;
//    }
//    else if (singleSize <= 8) // 3倍
//    {
//        int size1 = 4;
//        int size2 = 8;
//        double dRatio1 = 6;
//        double dRatio2 = 3;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 16) // 2倍
//    {
//        int size1 = 8;
//        int size2 = 16;
//        double dRatio1 = 3;
//        double dRatio2 = 2;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 32) // 1.5倍
//    {
//        int size1 = 16;
//        int size2 = 32;
//        double dRatio1 = 2;
//        double dRatio2 = 1.5;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 64) // 1.26倍
//    {
//        int size1 = 32;
//        int size2 = 64;
//        double dRatio1 = 1.5;
//        double dRatio2 = 1.26;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 128) // 1.15倍
//    {
//        int size1 = 64;
//        int size2 = 128;
//        double dRatio1 = 1.26;
//        double dRatio2 = 1.15;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 256) // 1.08倍
//    {
//        int size1 = 128;
//        int size2 = 256;
//        double dRatio1 = 1.15;
//        double dRatio2 = 1.08;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 512) // 1.05倍
//    {
//        int size1 = 256;
//        int size2 = 512;
//        double dRatio1 = 1.08;
//        double dRatio2 = 1.05;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//    else if (singleSize <= 1024) // 1.04倍
//    {
//        int size1 = 512;
//        int size2 = 1024;
//        double dRatio1 = 1.05;
//        double dRatio2 = 1.04;
//        dRatio = dRatio1 - (singleSize - size1) * (dRatio1 - dRatio2)/(size2 - size1);
//    }
//
//    return dRatio;
//}

double Utils::getRealMemoryRatio( int singleSize )
{
    singleSize = abs(singleSize);
    double dRatio = 1;
    int realSize = singleSize + (8 - (singleSize % 8)) + 8;
    dRatio = (double)realSize / (double)singleSize;

    return dRatio;
}

double Utils::getRealMemorySize( int singleSize )
{
    singleSize = abs(singleSize);
    int realSize = singleSize + (8 - (singleSize % 8)) + 8;
    return realSize;
}

/** 进程挂起、唤醒、关闭相关函数 */

BOOL Utils::PauseResumeThreadList(DWORD dwOwnerPID, bool bResumeThread) 
{ 
    HANDLE        hThreadSnap = NULL; 
    BOOL          bRet        = FALSE; 
    THREADENTRY32 te32        = {0}; 

    // Take a snapshot of all threads currently in the system. 

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (hThreadSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 

    // Fill in the size of the structure before using it. 

    te32.dwSize = sizeof(THREADENTRY32); 

    // Walk the thread snapshot to find all threads of the process. 
    // If the thread belongs to the process, add its information 
    // to the display list.

    if (Thread32First(hThreadSnap, &te32)) 
    { 
        do 
        { 
            if (te32.th32OwnerProcessID == dwOwnerPID) 
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (bResumeThread)
                {
                    cout << _T("Resuming Thread 0x") << cout.setf( ios_base::hex ) << te32.th32ThreadID << '\n';
                    ResumeThread(hThread);
                }
                else
                {
                    cout << _T("Suspending Thread 0x") << cout.setf( ios_base::hex ) << te32.th32ThreadID << '\n';
                    SuspendThread(hThread);
                }
                CloseHandle(hThread);
            } 
        }
        while (Thread32Next(hThreadSnap, &te32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;          // could not walk the list of threads 

    // Do not forget to clean up the snapshot object. 
    CloseHandle (hThreadSnap); 

    return (bRet); 
} 

BOOL Utils::ProcessList() 
{
    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    //  Take a snapshot of all processes in the system. 
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 

    //  Fill in the size of the structure before using it. 
    pe32.dwSize = sizeof(PROCESSENTRY32); 

    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 

    if (Process32First(hProcessSnap, &pe32)) 
    { 
        do 
        { 
            cout << _T("PID\t") << pe32.th32ProcessID << '\t' << pe32.szExeFile << '\n';
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;    // could not walk the list of processes 

    // Do not forget to clean up the snapshot object. 

    CloseHandle (hProcessSnap); 
    return (bRet); 
} 

int Utils::KillProcessByName(const QString& processName, const QString& processPath)
{
    //return 0（进程不存在） 1（进程存在并被杀死） -1（进程存在杀不死）
    int result = 0;

    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        ASSERT(false);
        return 0; 
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); 

    if (Process32First(hProcessSnap, &pe32)) 
    { 
        do { 
            if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) == 0) {
                HANDLE tmpProc = OpenProcess(PROCESS_TERMINATE , FALSE, pe32.th32ProcessID);
                if (!tmpProc) {
                    ASSERT(false);
                    continue;
                }
                if (!processPath.isEmpty()) {
                    HANDLE hModuleSnap = INVALID_HANDLE_VALUE; 
                    MODULEENTRY32 me32; 

                    hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe32.th32ProcessID ); 
                    if( hModuleSnap == INVALID_HANDLE_VALUE ) 
                    { 
                        ASSERT(false);
                        continue;
                    } 

                    me32.dwSize = sizeof( MODULEENTRY32 ); 

                    if( !Module32First( hModuleSnap, &me32 ) ) 
                    { 
                        ASSERT(false);
                        CloseHandle( hModuleSnap );
                        continue;
                    } 

                    do { 
                        if (me32.th32ProcessID == pe32.th32ProcessID) {
                            QString realPath = me32.szExePath;
                            if (realPath.indexOf(processPath) == 0) {
                                BOOL rt = TerminateProcess(tmpProc, NULL);
                                if (rt == 0) 
                                    result = -1;
                                else if (result != -1)
                                    result = 1;
                            }
                            break;
                        }
                    } while( Module32Next( hModuleSnap, &me32 ) ); 
                    CloseHandle( hModuleSnap ); 
                } else {
                    BOOL rt = TerminateProcess(tmpProc, NULL);
                    if (rt == 0) 
                        result = -1;
                    else if (result != -1)
                        result = 1;
                }
            }
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
    } 
    else {
        ASSERT(false);
    }

    CloseHandle (hProcessSnap);
    return result;
}

int Utils::PauseResumeProcessByName(const QString& processName, bool bResume)
{
    //return 0（进程不存在） 1（进程存在并被杀死） -1（进程存在杀不死）
    int result = 0;

    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        ASSERT(false);
        return 0; 
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); 

    if (Process32First(hProcessSnap, &pe32)) 
    { 
        do { 
            if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) == 0) {
                HANDLE tmpProc = OpenProcess(PROCESS_TERMINATE , FALSE, pe32.th32ProcessID);
                if (!tmpProc) {
                    ASSERT(false);
                    continue;
                }

                // 挂起、唤醒进程 
                result = PauseResumeThreadList(pe32.th32ProcessID, bResume);
            }
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
    } 
    else {
        ASSERT(false);
    }

    CloseHandle (hProcessSnap);
    return result;
}

int Utils::getProcessIDByName(const QString& processName)
{
    //return 0（进程不存在） 1（进程存在并被杀死） -1（进程存在杀不死）
    int result = -1;

    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        ASSERT(false);
        return 0; 
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); 

    if (Process32First(hProcessSnap, &pe32)) 
    { 
        do { 
            if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) == 0) {
                result = pe32.th32ProcessID;
                break;
            }
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
    } 
    else {
        ASSERT(false);
    }

    CloseHandle (hProcessSnap);
    return result;
}

int Utils::AddProcessDirToPath(const QString& processName)
{
    //return 0（进程不存在） 1（进程存在并被杀死） -1（进程存在杀不死）
    int result = 0;

    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        ASSERT(false);
        return 0; 
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); 

    if (Process32First(hProcessSnap, &pe32)) 
    { 
        do { 
            if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) == 0)
            {
                HANDLE hModuleSnap = INVALID_HANDLE_VALUE; 
                MODULEENTRY32 me32; 

                hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe32.th32ProcessID ); 
                if( hModuleSnap == INVALID_HANDLE_VALUE ) 
                { 
                    ASSERT(false);
                    continue;
                } 

                me32.dwSize = sizeof( MODULEENTRY32 ); 

                if( !Module32First( hModuleSnap, &me32 ) ) 
                { 
                    ASSERT(false);
                    CloseHandle( hModuleSnap );
                    continue;
                } 

                do { 
                    if (me32.th32ProcessID == pe32.th32ProcessID) {
                        QDir dir(me32.szExePath);
                        dir.cdUp();
                        QString dirPath = dir.path();
                        // 加到环境变量
                        QString cmd = QString("set path=%path%;\"%QtDir%\\bin\";\"%QtDir%\\lib\";%1;").arg(dirPath);
                        //system(cmd.toLocal8Bit().data());
                        //cmd = QString("set _NT_SYMBOL_PATH=\"%windir%\\symbols\";\"%QtDir%\\bin\";\"%QtDir%\\lib\";%1;").arg(dirPath);
                        //system(cmd.toLocal8Bit().data());
                        char chBuf[0x8000]={0};
                        GetEnvironmentVariable("Path",chBuf,0x10000);
                        QString path = QString("%1;\"%2\\bin\";\"%2\\lib\";\"%3\"").arg(chBuf).arg(::getenv("QtDir")).arg(dirPath);
                        ::SetEnvironmentVariable("Path", path.toLocal8Bit().data());
                        GetEnvironmentVariable("windir",chBuf,0x10000);
                        QString Variable = QString("srv*C:\\symbols*http://msdl.microsoft.com/download/symbols;\"%1\\symbols\";\"%2\\bin\";\"%2\\lib\";\"%3\"").arg(chBuf).arg(::getenv("QtDir")).arg(dirPath);
                        SetEnvironmentVariable("_NT_SYMBOL_PATH", Variable.toLocal8Bit().data());
                        GetEnvironmentVariable("path",chBuf,0x10000);
                        GetEnvironmentVariable("_NT_SYMBOL_PATH",chBuf,0x10000);
                        break;
                    }
                } while( Module32Next( hModuleSnap, &me32 ) ); 
                CloseHandle( hModuleSnap ); 
            }
        } 
        while (Process32Next(hProcessSnap, &pe32)); 
    } 
    else {
        ASSERT(false);
    }

    CloseHandle (hProcessSnap);
    return result;
}

QStringList Utils::getFileList(const QString &path)
{
    QDir dir(path);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folderInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList filePathList;
    for(int i = 0; i != fileInfoList.size(); i++)
    {
        QString name = fileInfoList.at(i).absoluteFilePath();
        filePathList << name;
    }

    for(int i = 0; i != folderInfoList.size(); i++)
    {
        QString name = folderInfoList.at(i).absoluteFilePath();
        filePathList << getFileList(name);
    }

    return filePathList;
}

//#include <stdio.h>
//#include <Windows.h>
//
//typedef ULONG (WINAPI *RTLWALKFRAMECHAIN)(OUT PVOID *Callers, IN ULONG Count, IN ULONG Flags);
//
//RTLWALKFRAMECHAIN RtlWalkFrameChain=(RTLWALKFRAMECHAIN)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"RtlWalkFrameChain");
//
//void PrintStack()
//{
//    PVOID ary[MAX_PATH]={0}; //这里应该动态分配内存实现，直接用数组是偷懒的办法。
//    ULONG StackCount;
//    StackCount=RtlWalkFrameChain(ary,MAX_PATH,0);
//    printf("\nStackCount=%ld\n",StackCount);
//    for(ULONG i=0;i<StackCount;i++)
//        printf("Stack[%d]=%p\n",i,ary[i]);
//    //DebugBreak();
//}

// 用户栈回溯
#define MAX_STACK_DEPTH 128
typedef struct _RTL_STACK_TRACE_ENTRY {

    struct _RTL_STACK_TRACE_ENTRY * HashChain;

    ULONG TraceCount;
    USHORT Index;
    USHORT Depth;

    PVOID BackTrace [MAX_STACK_DEPTH];

} RTL_STACK_TRACE_ENTRY, *PRTL_STACK_TRACE_ENTRY;

#pragma optimize("y", off) // disable FPO
void dumpStack2(void)
{
    const UINT max_name_length = 256;	// Max length of symbols' name.
#ifdef WIN64
    WOW64_CONTEXT context;
#else
    CONTEXT context;			// Store register addresses.
#endif
    STACKFRAME64 stackframe;		// Call stack.
    HANDLE process, thread;			// Handle to current process & thread.
    // Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.
    PSYMBOL_INFO symbol;			// Debugging symbol's information.
    IMAGEHLP_LINE64 source_info;		// Source information (file name & line number)
    DWORD displacement;			// Source line displacement.

    // Initialize PSYMBOL_INFO structure.
    // Allocate a properly-sized block.
    symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));	
    memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);	// SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
    symbol->MaxNameLen = max_name_length;

    // Initialize IMAGEHLP_LINE64 structure.
    memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
    source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Initialize STACKFRAME64 structure.
    // Get context.
#ifdef WIN64
    Wow64GetThreadContext(NULL,&context);
#else
    RtlCaptureContext(&context);
#endif
    memset(&stackframe, 0, sizeof(STACKFRAME64));
    stackframe.AddrPC.Offset = context.Eip;		// Fill in register addresses (EIP, ESP, EBP).
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;

    STACKFRAME64 stackframeOld = stackframe;

    process = GetCurrentProcess();	// Get current process & thread.
    thread = GetCurrentThread();

    // Initialize dbghelp library.
    if(!SymInitialize(process, NULL, TRUE))
        return ;

    //_putts(__T("Call stack: \n\n"));

    PVOID BackTrace [MAX_STACK_DEPTH];
    RTL_STACK_TRACE_ENTRY Trace;
    ULONG Hash;
    int depth = RtlCaptureStackBackTrace(1, MAX_STACK_DEPTH, BackTrace, &Hash);
    for (int i=0; i<depth; i++)
    {
        PVOID address = BackTrace + i;
        QString str = QString::number(int(*(BackTrace + i)), 16);
        qDebug() << "str" << str;
    }
    for (int j=0; j<1000000; j++)
    {
        depth = RtlCaptureStackBackTrace(3, MAX_STACK_DEPTH, BackTrace, &Hash);
        for (int i=0; i<depth; i++)
        {
            PVOID address = BackTrace + i;
            QString str = QString::number(int(*(BackTrace + i)), 16);
            //qDebug() << "str" << str;
        }
    }
    //CaptureStackBackTrace()
    //for (int i=0; i<depth; i++)
    //{
    //    PVOID address = BackTrace + i;
    //    QString str = QString::number(int(*(BackTrace + i)), 16);
    //    qDebug() << "str" << str;
    //}

    //for (int j=0; j<1000000; j++)
    {
#ifdef WIN64
        Wow64GetThreadContext(NULL,&context);
#else
        RtlCaptureContext(&context);
#endif
        memset(&stackframe, 0, sizeof(STACKFRAME64));
        stackframe.AddrPC.Offset = context.Eip;		// Fill in register addresses (EIP, ESP, EBP).
        stackframe.AddrPC.Mode = AddrModeFlat;
        stackframe.AddrStack.Offset = context.Esp;
        stackframe.AddrStack.Mode = AddrModeFlat;
        stackframe.AddrFrame.Offset = context.Ebp;
        stackframe.AddrFrame.Mode = AddrModeFlat;
        // Enumerate call stack frame.
        int depth2 = 0;
        while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, 
            &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
        {
            depth2++;
            if (depth2 > 3)
            {
                break;
            }
            if(stackframe.AddrFrame.Offset == 0)	// End reaches.
                break;

            if(SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol))// Get symbol.
                _tprintf(__T(" > %s\n"), symbol->Name);

            if(SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, 
                &displacement, &source_info)) {				// Get source information.
                    _tprintf(__T("\t[%s:%d] at addr 0x%08LX\n"), 
                        source_info.FileName, 
                        source_info.LineNumber,
                        stackframe.AddrPC.Offset);
            } else {
                if(GetLastError() == 0x1E7) {		// If err_code == 0x1e7, no symbol was found.
                    _tprintf(__T("\tNo debug symbol loaded for this function.\n"));
                }
            }
        }
    }

    // Enumerate call stack frame.
    while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, 
        &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
    {
        //if(stackframe.AddrFrame.Offset == 0)	// End reaches.
        //    break;

        //if(SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol))// Get symbol.
        //    /*_tprintf(__T(" > %s\n"), symbol->Name)*/;

        //if(SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, 
        //    &displacement, &source_info)) {				// Get source information.
        //        /*_tprintf(__T("\t[%s:%d] at addr 0x%08LX\n"), 
        //            source_info.FileName, 
        //            source_info.LineNumber,
        //            stackframe.AddrPC.Offset)*/;
        //} else {
        //    if(GetLastError() == 0x1E7) {		// If err_code == 0x1e7, no symbol was found.
        //        /*_tprintf(__T("\tNo debug symbol loaded for this function.\n"))*/;
        //    }
        //}
    }

    SymCleanup(process);	// Clean up and exit.
    free(symbol);
}

void Utils::dumpStack(void)
{
    const UINT max_name_length = 256;	// Max length of symbols' name.

#ifdef WIN64
    WOW64_CONTEXT context;
#else
    CONTEXT context;			// Store register addresses.
#endif
    STACKFRAME64 stackframe;		// Call stack.
    HANDLE process, thread;			// Handle to current process & thread.
    // Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.
    PSYMBOL_INFO symbol;			// Debugging symbol's information.
    IMAGEHLP_LINE64 source_info;		// Source information (file name & line number)
    DWORD displacement;			// Source line displacement.

    // Initialize PSYMBOL_INFO structure.
    // Allocate a properly-sized block.
    symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));	
    memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);	// SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
    symbol->MaxNameLen = max_name_length;

    // Initialize IMAGEHLP_LINE64 structure.
    memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
    source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Initialize STACKFRAME64 structure.
#ifdef WIN64
    Wow64GetThreadContext(NULL,&context);
#else
    RtlCaptureContext(&context);
#endif
    memset(&stackframe, 0, sizeof(STACKFRAME64));
    stackframe.AddrPC.Offset = context.Eip;		// Fill in register addresses (EIP, ESP, EBP).
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;

    process = GetCurrentProcess();	// Get current process & thread.
    thread = GetCurrentThread();

    // Initialize dbghelp library.
    if(!SymInitialize(process, NULL, TRUE))
        return ;

    _putts(__T("Call stack: \n\n"));

    // Enumerate call stack frame.
    while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, 
        &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
    {
        if(stackframe.AddrFrame.Offset == 0)	// End reaches.
            break;

        if(SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol))// Get symbol.
            _tprintf(__T(" > %s\n"), symbol->Name);

        if(SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, 
            &displacement, &source_info)) {				// Get source information.
                _tprintf(__T("\t[%s:%d] at addr 0x%08LX\n"), 
                    source_info.FileName, 
                    source_info.LineNumber,
                    stackframe.AddrPC.Offset);
        } else {
            if(GetLastError() == 0x1E7) {		// If err_code == 0x1e7, no symbol was found.
                _tprintf(__T("\tNo debug symbol loaded for this function.\n"));
            }
        }
    }

    SymCleanup(process);	// Clean up and exit.
    free(symbol);
}

int Utils::dumpStack3(PVOID *Callers, ULONG Count)
{
    int depth = 0;
    const UINT max_name_length = 256;	// Max length of symbols' name.

#ifdef WIN64
    WOW64_CONTEXT context;
#else
    CONTEXT context;			// Store register addresses.
#endif
    STACKFRAME64 stackframe;		// Call stack.
    HANDLE process, thread;			// Handle to current process & thread.
    // Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.
    static PSYMBOL_INFO symbol;			// Debugging symbol's information.
    IMAGEHLP_LINE64 source_info;		// Source information (file name & line number)
    DWORD displacement;			// Source line displacement.

    // Initialize PSYMBOL_INFO structure.
    // Allocate a properly-sized block.
    if (symbol == nullptr)
    {
        symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));	
    }
    memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);	// SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
    symbol->MaxNameLen = max_name_length;

    // Initialize IMAGEHLP_LINE64 structure.
    memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
    source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Initialize STACKFRAME64 structure.
#ifdef WIN64
    Wow64GetThreadContext(NULL,&context);
#else
    RtlCaptureContext(&context);
#endif
    memset(&stackframe, 0, sizeof(STACKFRAME64));
    stackframe.AddrPC.Offset = context.Eip;		// Fill in register addresses (EIP, ESP, EBP).
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;

    process = GetCurrentProcess();	// Get current process & thread.
    thread = GetCurrentThread();

    // Initialize dbghelp library.
    static bool s_bFirst = true;
    if (s_bFirst)
    {
        if(!SymInitialize(process, NULL, TRUE))
            return depth;
        s_bFirst = false;
    }
    

    _putts(__T("Call stack: \n\n"));

    // Enumerate call stack frame.
    while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, 
        &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
    {
        if(stackframe.AddrFrame.Offset == 0)	// End reaches.
            break;

        Callers[depth] = (PVOID)stackframe.AddrFrame.Offset;

        ++depth;
        if (depth >= Count)
        {
            break;
        }
    }

    //SymCleanup(process);	// Clean up and exit.
    //free(symbol);

    return 0;
}

QString Utils::s_openAppName;
QMap<int, QString> s_funcNames;
int Utils::getAddressFromFuncName( const QString funcName )
{
    return s_funcNames.key(funcName, -1);
}

bool Utils::readFuncInfoFromAddressList(QSet<__int64> &addressList)
{
    // 按顺序函数信息，以提高性能
    QList<__int64> sortList = addressList.toList();
    qSort(sortList.begin(), sortList.end());
    for (auto it=sortList.begin(); it!=sortList.end(); it++)
    {
        qDebug() << "address2=" << (int)*it;
        getInfoFromAddress((int)*it);
    }
    return true;
}

QString Utils::getInfoFromAddress(int address)
{
    //return QString::number(address, 16);
    QString funcName = s_funcNames.value(address);
    if (!funcName.isEmpty())
    {
        return funcName;
    }
    const UINT max_name_length = 1024;	// Max length of symbols' name.

    // Initialize PSYMBOL_INFO structure.
    // Allocate a properly-sized block.
    PSYMBOL_INFO symbol = nullptr;	
    if (symbol == nullptr)
    {
        symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
        memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);	// SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
        symbol->MaxNameLen = max_name_length;
    }

    static HANDLE process = nullptr;
    if (process == nullptr)
    {
        //process = ::GetCurrentProcess(); 
        //process = ExeOpt::getProcessHandle("GAEA.exe");
        process = ExeOpt::getProcessHandle(s_openAppName.toLocal8Bit().data());
        // Initialize dbghelp library.
        if(!SymInitialize(process, NULL, TRUE))
            return funcName;
    }

    if(SymFromAddr(process, address, NULL, symbol))// Get symbol.
    {
        funcName = symbol->Name;
    }

    //跨进程获取调试信息 caob 2017-12-1 17:29:11
    //{
    //    QString filepath = "G:\\myProject\\Blackhouse\\Win32\\Release\\ProcessAddressAnalysis.exe";
    //    QString strExeFullPara = "\""+ filepath + "\"" + " GAEA.exe " + QString::number(address);

    //    int nRet = WinExec(strExeFullPara.toLocal8Bit().data(), SW_SHOWNORMAL);
    //}

    //SymCleanup(process);	// Clean up and exit.
    //free(symbol);

    if (GlobalData::getInstance()->memoryMode == 0 && !funcName.isEmpty())
    {
        IMAGEHLP_LINE64 source_info;		// Source information (file name & line number)
        DWORD displacement;			// Source line displacement.

        // Initialize IMAGEHLP_LINE64 structure.
        memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
        source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        if(SymGetLineFromAddr64(process, address, 
            &displacement, &source_info)) {				// Get source information.
                //funcName += QString("-%1-%2").arg(source_info.FileName).arg(source_info.LineNumber);
                funcName += QString("-%1").arg(source_info.LineNumber);
        } else {
            //if(GetLastError() == 0x1E7) {		// If err_code == 0x1e7, no symbol was found.
            //    _tprintf(__T("\tNo debug symbol loaded for this function.\n"));
        }
    }

    //

    if (!funcName.isEmpty())
    {
        funcName = getModuleNameFromAddress((PVOID)address) + "-" + funcName;
    }
    else
    {
        DWORD relativeAddress = 0;
        QString moduleName = getModuleNameAndRelativeAddress((PVOID)address, relativeAddress);
        funcName = moduleName + "-" + QString::number(relativeAddress, 16);
    }

    if (funcName.isEmpty())
    {
        funcName = QString::number(address, 16);
    }

    s_funcNames[address] = funcName;

    if (GlobalData::getInstance()->memoryMode == 0)
    {
        if (s_modules.isEmpty())
        {
            initModulesInfo();
        }
        static auto s_lastModuleIt = s_modules.begin() + 1;
        if ((DWORD)address >= s_lastModuleIt.key())
        {
            SymUnloadModule64(process, (DWORD64)(s_lastModuleIt - 1).key());
            ++s_lastModuleIt;
        }
    }
    else
    {
        if (s_funcNames.size() % 1000 == 0)
        {
            SymCleanup(process);
            process = nullptr;
        }
    }
    
    return funcName;
}


#include <Windows.h>  
#include <iostream>  
#include "tlhelp32.h"  
#include "stdio.h"  
#include <map>  
#include <QVector>
using namespace std;  

QVector<int> Utils::GetProcessThreadIDList()
{
    QVector<int> idList;
    DWORD th32ProcessID=GetCurrentProcessId();  
    HANDLE hThreadSnap;   
    THREADENTRY32 th32;   
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, th32ProcessID);   
    if (hThreadSnap == INVALID_HANDLE_VALUE)   
    {   
        return idList;   
    }   
    th32.dwSize = sizeof(THREADENTRY32);   
    if (!Thread32First(hThreadSnap, &th32))   
    {   
        CloseHandle(hThreadSnap);   
        return idList;   
    }   
    do   
    {   
        if (th32.th32OwnerProcessID == th32ProcessID)   
        {   
            idList.append(th32.th32ThreadID);  
        }   
    }while(Thread32Next(hThreadSnap, &th32));  

    CloseHandle(hThreadSnap);   
    return idList;   
}

QVector<HANDLE> Utils::GetProcessThreadHandleList()  
{   
    QVector<int> idList = GetProcessThreadIDList();
    QVector<HANDLE> handleList;
    for (int i=0; i<idList.size(); ++i)
    {
        HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, idList[i] );
        handleList.append(hThread);
    }
    return handleList;
}

void Utils::initProcessWindbg()
{
    HANDLE process = GetCurrentProcess();

    // Initialize dbghelp library.
    SymInitialize(process, NULL, TRUE);
}

void Utils::cleanupWindbg()
{
    HANDLE process = GetCurrentProcess();
    SymCleanup(process);	// Clean up and exit.
}

//////////////////////////////////////////////////////////////////////////
//dwProcessID:进程ID；pvModuleRemote进程中指定模块的地址
//返回模块首选的基地址
PVOID GetModulePreferredBaseAddr(DWORD dwProcessID, PVOID pvModuleRemote){
    PVOID pvModulePreferredBaseAddr = NULL;
    IMAGE_DOS_HEADER  idh;
    IMAGE_NT_HEADERS  inth;

    //读取远程模块的DOS头
    Toolhelp32ReadProcessMemory(dwProcessID, pvModuleRemote, &idh, sizeof(idh), NULL);

    //验证是否是DOS头部
    if (idh.e_magic == IMAGE_DOS_SIGNATURE){
        //读取远程模块的NT头
        Toolhelp32ReadProcessMemory(dwProcessID, (PBYTE)pvModuleRemote + idh.e_lfanew, &inth,
            sizeof(inth), NULL);
        //验证是否是NT image header
        if (inth.Signature == IMAGE_NT_SIGNATURE){
            //有效的NT头，获取首先的基地址
            pvModulePreferredBaseAddr = (PVOID)inth.OptionalHeader.ImageBase;
        }
    }

    return (pvModulePreferredBaseAddr);
}

HMODULE fnGetProcessBase(DWORD PID)
{
    //获取进程基址
    HANDLE hSnapShot;
    //通过CreateToolhelp32Snapshot和线程ID，获取进程快照
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    if (hSnapShot == INVALID_HANDLE_VALUE)
    {
        //GetLastErrorBox(NULL,"无法创建快照");
        return NULL;
    }
    MODULEENTRY32 ModuleEntry32;
    ModuleEntry32.dwSize = sizeof(ModuleEntry32);

    HMODULE res = (HMODULE)2000000000;
    if (Module32First(hSnapShot, &ModuleEntry32))
    {
        do 
        {
            //TCHAR szExt[5];
            //strcpy(szExt, ModuleEntry32.szExePath + strlen(ModuleEntry32.szExePath) - 4);
            //for (int i = 0;i < 4;i++)
            //{
            //    if ((szExt[i] >= 'a')&&(szExt[i] <= 'z'))
            //    {
            //        szExt[i] = szExt[i] - 0x20;
            //    }
            //}
            //if (!strcmp(szExt, ".EXE"))
            //{
            //    CloseHandle(hSnapShot);
            //    return ModuleEntry32.hModule;
            //}
            //int baseAddress = (int)GetModulePreferredBaseAddr(PID, ModuleEntry32.modBaseAddr);
            int baseAddress = (int)ModuleEntry32.modBaseAddr;
            qDebug() << "moduleinfo" << (int)ModuleEntry32.modBaseAddr << (int)ModuleEntry32.modBaseSize << QString::fromLocal8Bit(ModuleEntry32.szModule) << QString::fromLocal8Bit(ModuleEntry32.szExePath) << (int) ModuleEntry32.hModule << (int)ModuleEntry32.th32ModuleID;
            res = (HMODULE)min((int)res, (int)baseAddress);
            qDebug() << "minbaseaddress" << res;
        } while (Module32Next(hSnapShot, &ModuleEntry32));
    }
    CloseHandle(hSnapShot);

    return res;

}

bool Utils::initModulesInfo()
{
    if (!s_modules.isEmpty())
    {
        return true;
    }
    int PID = getProcessIDByName(s_openAppName);
    //获取进程基址
    HANDLE hSnapShot;
    //通过CreateToolhelp32Snapshot和线程ID，获取进程快照
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    if (hSnapShot == INVALID_HANDLE_VALUE)
    {
        //GetLastErrorBox(NULL,"无法创建快照");
        return NULL;
    }
    MODULEENTRY32 ModuleEntry32;
    ModuleEntry32.dwSize = sizeof(ModuleEntry32);

    if (Module32First(hSnapShot, &ModuleEntry32))
    {
        do 
        {
            // DWORD baseAddress = (DWORD)GetModulePreferredBaseAddr(PID, ModuleEntry32.modBaseAddr);
            DWORD baseAddress = (DWORD)ModuleEntry32.modBaseAddr;
            s_modules[baseAddress] = ModuleEntry32.szModule;
            qDebug() << "moduleinfo" << (int)ModuleEntry32.modBaseAddr << (int)ModuleEntry32.modBaseSize << QString::fromLocal8Bit(ModuleEntry32.szModule) << QString::fromLocal8Bit(ModuleEntry32.szExePath) << (int) ModuleEntry32.hModule << (int)ModuleEntry32.th32ModuleID;
        } while (Module32Next(hSnapShot, &ModuleEntry32));
    }
    CloseHandle(hSnapShot);

    for (auto it=s_modules.begin(); it!=s_modules.end(); it++)
    {
        qDebug() << "moduleinfo" << (int)it.key() << it.value();
    }

    return true;
}

QString Utils::getModuleNameFromAddress(PVOID address)
{
    if (s_modules.isEmpty())
    {
        Utils::initModulesInfo();
    }
    DWORD dwAddress = (DWORD)address;
    // 二分查找
    // 先用笨方法查找
    // 二分查找
    // 先用笨方法查找
    DWORD lastBaseAddress = s_modules.begin().key();
    QString lastModuleName = s_modules.begin().value();
    for (auto it=s_modules.begin(); it!=s_modules.end(); it++)
    {
        // dwAddress < it.key()说明应该是上一个；it == s_modules.end()说明应该是最后一个；
        if (dwAddress < it.key())
        {
            return lastModuleName;
        }
        lastBaseAddress = it.key();
        lastModuleName = it.value();
    }

    return lastModuleName;
}

QString Utils::getModuleNameAndRelativeAddress(PVOID address, DWORD &relativeAddress)
{
    if (s_modules.isEmpty())
    {
        Utils::initModulesInfo();
    }
    DWORD dwAddress = (DWORD)address;
    // 二分查找
    // 先用笨方法查找
    DWORD lastBaseAddress = s_modules.begin().key();
    QString lastModuleName = s_modules.begin().value();
    for (auto it=s_modules.begin(); it!=s_modules.end(); it++)
    {
        // dwAddress < it.key()说明应该是上一个；it == s_modules.end()说明应该是最后一个；
        if (dwAddress < it.key())
        {
            relativeAddress = ((DWORD)address) - lastBaseAddress;
            return lastModuleName;
        }
        lastBaseAddress = it.key();
        lastModuleName = it.value();
    }

    relativeAddress = ((DWORD)address) - lastBaseAddress;
    return lastModuleName;
}


#include <Windows.h>
#include "EnumPeb.h"
#define TH32CS_PROCESS TH32CS_SNAPPROCESS
#define INVALD_HANDLE_VALUE 0

// static QSet<PVOID> s_ptrs;
static QMap<DWORD, DWORD> s_ptrs;

struct HeapRange
{
    HeapRange(int start, int end)
    {
        _start = start;
        _end = end;
    }

    HeapRange()
    {
        _start = 0;
        _end = 0;
    }

    HeapRange(const HeapRange& other)
    {
        _start = other._start;
        _end = other._end;
    }

    int _start;
    int _end;

    bool operator<(const HeapRange &heapRange2)
    {
        if (_start < heapRange2._start && _end <= heapRange2._start)
        {
            return true;
        }
        else 
        {
            return false;
        }
    }

    bool operator==(const HeapRange &heapRange2)
    {
        if (_start < heapRange2._start && _end > heapRange2._start
            || heapRange2._start < _start && heapRange2._end < _start)
        {
            return true;
        }
        else 
        {
            return false;
        }
    }

    HeapRange &operator=(const HeapRange &other)
    {
        _start = other._start;
        _end = other._end;
        return *this;
    }
};

bool operator==(const HeapRange &heapRange1, const HeapRange &heapRange2)
{
    if (heapRange1._start < heapRange2._start && heapRange1._end > heapRange2._start
        || heapRange2._start < heapRange1._start && heapRange2._end < heapRange1._start)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

bool operator<(HeapRange &heapRange1, HeapRange &heapRange2)
{
    if (heapRange1._start <= heapRange2._start && heapRange1._end <= heapRange2._start)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

Q_DECLARE_METATYPE(HeapRange)

//qRegisterMetaType<HeapRange>();


//QSet<HeapRange> g_heapRanges;
int showheap( DWORD pid )
{
    HEAPLIST32 hl;

    HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, pid);

    hl.dwSize = sizeof(HEAPLIST32);

    if ( hHeapSnap == INVALID_HANDLE_VALUE )
    {
        printf ("CreateToolhelp32Snapshot failed (%d)\n", GetLastError());
        return 1;
    }

    if( Heap32ListFirst( hHeapSnap, &hl ) )
    {
        do
        {
            HEAPENTRY32 he;
            ZeroMemory(&he, sizeof(HEAPENTRY32));
            he.dwSize = sizeof(HEAPENTRY32);

            if( Heap32First( &he, pid, hl.th32HeapID ) )
            {
                printf( "\nHeap ID: %d\n", hl.th32HeapID );
                do
                {
                    printf( "Block size: %d\n", he.dwBlockSize );
                    s_ptrs.insert((DWORD)(he.dwAddress - 16), 0);
                    qDebug() <<  he.th32HeapID << he.dwAddress << he.dwBlockSize << he.hHandle;

                    he.dwSize = sizeof(HEAPENTRY32);
                } while( Heap32Next(&he) );
            }
            hl.dwSize = sizeof(HEAPLIST32);
        } while (Heap32ListNext( hHeapSnap, &hl ));
    }
    else printf ("Cannot list first heap (%d)\n", GetLastError());

    CloseHandle(hHeapSnap); 

    return 0;
}


bool Utils::lookHeapMemory(PVOID ptr)
{
    if (!s_ptrs.isEmpty())
    {
        return s_ptrs.contains((DWORD)ptr);
    }

    qRegisterMetaType<HeapRange>();

    HANDLE         hProcessSnap = NULL; 
    BOOL           bRet      = FALSE; 
    PROCESSENTRY32 pe32      = {0}; 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(INVALD_HANDLE_VALUE == hProcessSnap )
    {
        return false;
    }
    pe32.dwSize = sizeof(pe32);

    if(!Process32First(hProcessSnap,&pe32))
    {
        return false;
    }
    QString processName = s_openAppName;
    do
    {
        if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        HANDLE hProcessHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, pe32.th32ProcessID);
        if(INVALD_HANDLE_VALUE == hProcessHeapSnap)
            continue;
        //HANDLE hProcess2 = OpenProcess(PROCESS_QUERY_INFORMATION,

        //    FALSE,

        //    pe32.th32ProcessID);
        //if (hProcess2 == nullptr)
        //{
        //    continue;
        //}

        PVOID baseAddress = fnGetProcessBase(pe32.th32ProcessID);
        //PVOID baseAddress = (PVOID)getBaseAddress(pe32.th32ProcessID);
       


        //PBYTE pAddress = NULL;
        //while (true)
        //{
        //    MEMORY_BASIC_INFORMATION mbi;
        //    SIZE_T res = VirtualQueryEx(hProcess2, (PVOID)pAddress, &mbi, sizeof(mbi));
        //    if (res == sizeof(mbi) && mbi.AllocationBase <= ptr && ptr < (PBYTE)mbi.BaseAddress + mbi.RegionSize)
        //    {
        //        //return true;
        //        //s_ptrs.insert((PVOID)mbi.AllocationBase);
        //        //qDebug() << (int)mbi.AllocationBase << (int)mbi.BaseAddress << mbi.RegionSize;
        //        //return true;//
        //    }
        //    else
        //    {
        //        break;
        //    }
        //    pAddress = ((PBYTE)mbi.BaseAddress + mbi.RegionSize);
        //}

        HEAPLIST32 hl32      = {0};
        hl32.dwSize = sizeof(HEAPLIST32);
        hl32.th32HeapID = 0;
        if(!Heap32ListFirst(hProcessHeapSnap, &hl32))
        {
            continue;
        }
        do
        {
            HEAPENTRY32 he32      = {0};
            ZeroMemory(&he32, sizeof(HEAPENTRY32));
            he32.dwSize = sizeof(HEAPENTRY32);
            he32.dwAddress = 0;

            //print heap list info
            if(!Heap32First(&he32, pe32.th32ProcessID, hl32.th32HeapID))
            {
                continue;
            }
            //baseAddress = fnGetProcessBase(pe32.th32ProcessID);
            do
            {
                //print heap info
                //here u can use ToolhelpReadProcessMemory  or other api to read other process memory.
                //qDebug() << pe32.th32ProcessID << hl32.th32HeapID << he32.th32HeapID << he32.dwAddress << he32.dwBlockSize << he32.hHandle;
                //he32.dwSize = sizeof(HEAPENTRY32);
                //MEMORY_BASIC_INFORMATION mbi;
                //SIZE_T res = VirtualQueryEx(hProcess2, (PVOID)he32.dwAddress, &mbi, sizeof(mbi));
                ////if (res == sizeof(mbi))
                //{
                //    //return true;
                //    if (!s_ptrs.contains((PVOID)((int)he32.dwAddress - (int)baseAddress)))
                //    {
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress));
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress - (int)baseAddress));
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress - (int)baseAddress + 8));
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress - (int)baseAddress + 16));
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress + 8));
                //        s_ptrs.insert((PVOID)((int)he32.dwAddress + 16));

                //        qDebug() << pe32.th32ProcessID << hl32.th32HeapID << he32.th32HeapID << he32.dwAddress << he32.dwBlockSize << (int)mbi.AllocationBase << (int)mbi.BaseAddress << mbi.RegionSize << (he32.dwAddress + (int)mbi.AllocationBase) << ((int)he32.dwAddress - (int)baseAddress) << ((int)he32.dwAddress - (int)mbi.AllocationBase);
                //    }
                //    //s_ptrs.insert((PVOID)(he32.dwAddress + ((int)mbi.BaseAddress - (int)mbi.AllocationBase)));
                //    //qDebug() << pe32.th32ProcessID << hl32.th32HeapID << he32.th32HeapID << he32.dwAddress << he32.dwBlockSize << (int)mbi.AllocationBase << (int)mbi.BaseAddress << mbi.RegionSize << (he32.dwAddress + ((int)mbi.BaseAddress - (int)mbi.AllocationBase));
                //
                //    
                //
                //}
                //s_ptrs.insert((PVOID)(he32.dwAddress));
                //he32.dwSize = sizeof(HEAPENTRY32);

                //DWORD dwReaded;
                //static BYTE byData[1024*1024];
                //Toolhelp32ReadProcessMemory(pe32.th32ProcessID, (PVOID)he32.dwAddress, (LPVOID)byData, min(sizeof(byData), he32.dwBlockSize), &dwReaded);
                //for (int i=0; i<sizeof(byData)/4; i++)
                //{
                //    s_ptrs.insert((PVOID)*(int*)(byData + i * 4));
                //}

                MEMORY_BASIC_INFORMATION mbi;
                //查询地址空间中内存地址的信息
                //参数2：表示查询内存的地址，参数3：用于接收内存信息
                //int res = VirtualQueryEx(hProcessHeapSnap, (PVOID)he32.dwAddress, &mbi, sizeof(mbi));
                //if (res == sizeof(mbi))
                //{
                //    g_heapRanges.insert(HeapRange((int)mbi.AllocationBase, (int)mbi.AllocationBase + (int)mbi.RegionSize));
                //}


                s_ptrs.insert((DWORD)(he32.dwAddress - (int)baseAddress), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress - (int)baseAddress + 16), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress - (int)baseAddress + 8), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress - (int)pe32.th32ProcessID), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress - (int)hl32.th32HeapID), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress + (int)baseAddress), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress + (int)baseAddress + 16), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress + (int)baseAddress + 8), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress + (int)pe32.th32ProcessID), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress + (int)hl32.th32HeapID), 0);
                s_ptrs.insert((DWORD)(he32.dwAddress), 0);
                he32.dwSize = sizeof(HEAPENTRY32);
                qDebug() << pe32.th32DefaultHeapID << pe32.th32ProcessID << hl32.th32HeapID << (PVOID)he32.th32HeapID << (PVOID)he32.dwAddress << he32.dwBlockSize << he32.hHandle << (he32.dwAddress - (int)baseAddress);

            }while(Heap32Next(&he32));
            hl32.dwSize = sizeof(HEAPLIST32);
            
        }while(Heap32ListNext(hProcessHeapSnap, &hl32));
        CloseHandle(hProcessHeapSnap);
        pe32.dwSize = sizeof(pe32);
        showheap(pe32.th32ProcessID);
    }while(Process32Next(hProcessSnap,&pe32));
    CloseHandle(hProcessSnap);


    return s_ptrs.contains((DWORD)ptr);
}


//QSet<PVOID> s_needPtrs;
QMap<DWORD, DWORD> s_needPtrs;
bool Utils::lookHeapMemory2(PVOID ptr, int size)
{
    static HANDLE hProcessHeapSnap = nullptr;
    static DWORD s_processID = -1;
    if (hProcessHeapSnap == nullptr)
    {
        HANDLE         hProcessSnap = NULL; 
        BOOL           bRet      = FALSE; 
        PROCESSENTRY32 pe32      = {0}; 

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_PROCESS,0);
        if(INVALD_HANDLE_VALUE == hProcessSnap )
        {
            return false;
        }
        pe32.dwSize = sizeof(pe32);

        if(!Process32First(hProcessSnap,&pe32))
        {
            return false;
        }
        QString processName = s_openAppName;
        do
        {
            if (processName.compare(pe32.szExeFile, Qt::CaseInsensitive) != 0)
            {
                continue;
            }
            hProcessHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, pe32.th32ProcessID);
            s_processID = pe32.th32ProcessID;
            
            CloseHandle(hProcessHeapSnap);
            //pe32.dwSize = sizeof(pe32);
            break;
        }while(Process32Next(hProcessSnap,&pe32));
        CloseHandle(hProcessSnap);
    }


    static DWORD dwReaded;
    //static BYTE byData[1024*1024*100];
    static int s_dataSize = 1024*1024;
    static BYTE *byData = (BYTE*)realloc(byData, s_dataSize);
    if (s_dataSize < size)
    {
        s_dataSize = size;
        byData = (BYTE*)realloc(byData, s_dataSize);
    }
    if (size < 4)
    {
        return false;
    }

    //VirtualQueryEx();
    //BOOL bSuccess = ReadProcessMemory(hProcessHeapSnap, (PVOID)ptr, (LPVOID)byData, size, &dwReaded);
    BOOL bSuccess = Toolhelp32ReadProcessMemory(s_processID, (PVOID)ptr, (LPVOID)byData, size, &dwReaded);
    if (bSuccess == FALSE || dwReaded != size)
    {
        s_ptrs.insert((DWORD)ptr, 0);
        return false;
    }
    if (size > dwReaded)
    {
        qDebug() << "Toolhelp32ReadProcessMemory" << size << dwReaded;
    }
    for (int i=0; i<dwReaded/4; i++)
    {
        int value = *(int*)(byData + i*4);
        if (!s_needPtrs.contains((DWORD)value) || s_ptrs.contains((DWORD)value))
        {
            continue;
        }
        s_ptrs.insert((DWORD)value, 0);
    }

    //for (int i=0; i<(int)dwReaded - 3; i++)
    //{
    //    int value = *(int*)(byData + i);
    //    if (!s_needPtrs.contains((PVOID)value) || s_ptrs.contains((PVOID)value))
    //    {
    //        continue;
    //    }
    //    s_ptrs.insert((PVOID)value);
    //}

    return true;
}

bool Utils::lookHeapMemory3(PVOID ptr)
{
    return s_ptrs.contains((DWORD)ptr);
}

bool Utils::lookHeapMemory4(PVOID ptr)
{
    s_needPtrs.insert((DWORD)ptr, (DWORD)ptr);
    return true;
}

bool Utils::clearHeapMemoryCache()
{
    //s_needPtrs.swap(QSet<PVOID>());
    //s_ptrs.swap(QSet<PVOID>());
    s_needPtrs.swap(QMap<DWORD, DWORD>());
    s_ptrs.swap(QMap<DWORD, DWORD>());
    return true;
}
