#include "EnumPeb.h"


//EnumPeb::EnumPeb(void)
//{
//}
//
//
//EnumPeb::~EnumPeb(void)
//{
//}

// EnumPEB.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include<Windows.h>
#include<iostream>
#include <Strsafe.h>
using namespace std;

#define NT_SUCCESS(x) ((x) >= 0)

#define ProcessBasicInformation 0
typedef
    NTSTATUS(WINAPI *pfnNtWow64QueryInformationProcess64)
    (HANDLE ProcessHandle, UINT32 ProcessInformationClass,
    PVOID ProcessInformation, UINT32 ProcessInformationLength,
    UINT32* ReturnLength);

typedef
    NTSTATUS(WINAPI *pfnNtWow64ReadVirtualMemory64)
    (HANDLE ProcessHandle, PVOID64 BaseAddress,
    PVOID BufferData, UINT64 BufferLength,
    PUINT64 ReturnLength);

typedef
    NTSTATUS(WINAPI *pfnNtQueryInformationProcess)
    (HANDLE ProcessHandle, ULONG ProcessInformationClass,
    PVOID ProcessInformation, UINT32 ProcessInformationLength,
    UINT32* ReturnLength);

template <typename T>
struct _UNICODE_STRING_T
{
    WORD Length;
    WORD MaximumLength;
    T Buffer;
};

template <typename T>
struct _LIST_ENTRY_T
{
    T Flink;
    T Blink;
};

template <typename T, typename NGF, int A>
struct _PEB_T
{
    typedef T type;

    union
    {
        struct
        {
            BYTE InheritedAddressSpace;
            BYTE ReadImageFileExecOptions;
            BYTE BeingDebugged;
            BYTE BitField;
        };
        T dummy01;
    };
    T Mutant;
    T ImageBaseAddress;     //进程加载基地址
    T Ldr;				    
    T ProcessParameters;    //各种信息，环境变量，命令行等等
    T SubSystemData;
    T ProcessHeap;
    T FastPebLock;
    T AtlThunkSListPtr;
    T IFEOKey;
    T CrossProcessFlags;
    T UserSharedInfoPtr;
    DWORD SystemReserved;
    DWORD AtlThunkSListPtr32;
    T ApiSetMap;
    T TlsExpansionCounter;
    T TlsBitmap;
    DWORD TlsBitmapBits[2];
    T ReadOnlySharedMemoryBase;
    T HotpatchInformation;
    T ReadOnlyStaticServerData;
    T AnsiCodePageData;
    T OemCodePageData;
    T UnicodeCaseTableData;
    DWORD NumberOfProcessors;
    union
    {
        DWORD NtGlobalFlag;
        NGF dummy02;
    };
    LARGE_INTEGER CriticalSectionTimeout;
    T HeapSegmentReserve;
    T HeapSegmentCommit;
    T HeapDeCommitTotalFreeThreshold;
    T HeapDeCommitFreeBlockThreshold;
    DWORD NumberOfHeaps;
    DWORD MaximumNumberOfHeaps;
    T ProcessHeaps;
    T GdiSharedHandleTable;
    T ProcessStarterHelper;
    T GdiDCAttributeList;
    T LoaderLock;
    DWORD OSMajorVersion;
    DWORD OSMinorVersion;
    WORD OSBuildNumber;
    WORD OSCSDVersion;
    DWORD OSPlatformId;
    DWORD ImageSubsystem;
    DWORD ImageSubsystemMajorVersion;
    T ImageSubsystemMinorVersion;
    T ActiveProcessAffinityMask;
    T GdiHandleBuffer[A];
    T PostProcessInitRoutine;
    T TlsExpansionBitmap;
    DWORD TlsExpansionBitmapBits[32];
    T SessionId;
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    T pShimData;
    T AppCompatInfo;
    _UNICODE_STRING_T<T> CSDVersion;
    T ActivationContextData;
    T ProcessAssemblyStorageMap;
    T SystemDefaultActivationContextData;
    T SystemAssemblyStorageMap;
    T MinimumStackCommit;
    T FlsCallback;
    _LIST_ENTRY_T<T> FlsListHead;
    T FlsBitmap;
    DWORD FlsBitmapBits[4];
    T FlsHighIndex;
    T WerRegistrationData;
    T WerShipAssertPtr;
    T pContextData;
    T pImageHeaderHash;
    T TracingFlags;
    T CsrServerReadOnlySharedMemoryBase;
};

template <typename T>
struct _STRING_T
{
    WORD Length;
    WORD MaximumLength;
    T    Buffer;
};

template <typename T>
struct _RTL_DRIVE_LETTER_CURDIR_T
{
    WORD         Flags;
    WORD         Length;
    ULONG        TimeStamp;
    _STRING_T<T> DosPath;
};

template <typename T>
struct _CURDIR_T
{
    _UNICODE_STRING_T<T> DosPath;
    T			         Handle;
};

template <typename T>
struct _RTL_USER_PROCESS_PARAMETERS_T
{
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    T ConsoleHandle;
    ULONG  ConsoleFlags;
    T StandardInput;
    T StandardOutput;
    T StandardError;
    _CURDIR_T<T> CurrentDirectory;
    _UNICODE_STRING_T<T> DllPath;
    _UNICODE_STRING_T<T> ImagePathName; //进程完整路径
    _UNICODE_STRING_T<T> CommandLine;   //进程命令行
    T Environment;             //环境变量（地址）
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    _UNICODE_STRING_T<T> WindowTitle;
    _UNICODE_STRING_T<T> DesktopInfo;
    _UNICODE_STRING_T<T> ShellInfo;
    _UNICODE_STRING_T<T> RuntimeData;
    _RTL_DRIVE_LETTER_CURDIR_T<T> CurrentDirectores[32];
    ULONG EnvironmentSize;
};

typedef _PEB_T<DWORD, DWORD64, 34> _PEB32;
typedef _PEB_T<DWORD64, DWORD, 30> _PEB64;
typedef _RTL_USER_PROCESS_PARAMETERS_T<UINT32> _RTL_USER_PROCESS_PARAMETERS32;
typedef _RTL_USER_PROCESS_PARAMETERS_T<UINT64> _RTL_USER_PROCESS_PARAMETERS64;

typedef struct _PROCESS_BASIC_INFORMATION64 {
    NTSTATUS ExitStatus;
    UINT32 Reserved0;
    UINT64 PebBaseAddress;
    UINT64 AffinityMask;
    UINT32 BasePriority;
    UINT32 Reserved1;
    UINT64 UniqueProcessId;
    UINT64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;

typedef struct _PROCESS_BASIC_INFORMATION32 {
    NTSTATUS ExitStatus;
    UINT32 PebBaseAddress;
    UINT32 AffinityMask;
    UINT32 BasePriority;
    UINT32 UniqueProcessId;
    UINT32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION32;


typedef struct _PEB_INFO {
    UINT64 ImageBaseAddress;  //进程加载基地址
    UINT64 Ldr;               //模块加载基地址
    UINT64 ProcessHeap;       //进程默认堆
    UINT64 ProcessParameters; //进程信息
    UINT64 Environment;       //环境变量
}PEBInfo,*PPEBInfo;

int getBaseAddress(int ProcessID)
{
    HANDLE ProcessHandle = NULL;
    //获得进程句柄
    ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
    PEBInfo PebInfo = {0};
    BOOL bSource = FALSE;
    BOOL bTarget = FALSE;
    //判断自己的位数
    IsWow64Process(GetCurrentProcess(), &bSource);
    //判断目标的位数
    IsWow64Process(ProcessHandle, &bTarget);

    //自己是32  目标64
    if (bTarget == FALSE && bSource == TRUE)
    {
        HMODULE NtdllModule = GetModuleHandle("ntdll.dll");
        pfnNtWow64QueryInformationProcess64 NtWow64QueryInformationProcess64 = (pfnNtWow64QueryInformationProcess64)GetProcAddress(NtdllModule,
            "NtWow64QueryInformationProcess64");

        pfnNtWow64ReadVirtualMemory64 NtWow64ReadVirtualMemory64 = (pfnNtWow64ReadVirtualMemory64)GetProcAddress(NtdllModule, "NtWow64ReadVirtualMemory64");
        PROCESS_BASIC_INFORMATION64 pbi = { 0 };
        UINT64 ReturnLength = 0;
        NTSTATUS Status = NtWow64QueryInformationProcess64(ProcessHandle, ProcessBasicInformation,
            &pbi, (UINT32)sizeof(pbi), (UINT32*)&ReturnLength);

        if (NT_SUCCESS(Status))
        {

            _PEB64* Peb = (_PEB64*)malloc(sizeof(_PEB64));
            Status = NtWow64ReadVirtualMemory64(ProcessHandle, (PVOID64)pbi.PebBaseAddress,
                (_PEB64*)Peb, sizeof(_PEB64), &ReturnLength);	
            _RTL_USER_PROCESS_PARAMETERS64 Parameters64;
            Status = NtWow64ReadVirtualMemory64(ProcessHandle, (PVOID64)Peb->ProcessParameters,
                &Parameters64, sizeof(_RTL_USER_PROCESS_PARAMETERS64), &ReturnLength);

            BYTE* Environment = new BYTE[Parameters64.EnvironmentSize * 2];
            Status = NtWow64ReadVirtualMemory64(ProcessHandle, (PVOID64)Parameters64.Environment, Environment, Parameters64.EnvironmentSize, NULL);
            //赋值
            PebInfo.ImageBaseAddress = Peb->ImageBaseAddress;
            return (int)PebInfo.ImageBaseAddress;
            PebInfo.Ldr = Peb->Ldr;
            PebInfo.ProcessHeap = Peb->ProcessHeap;
            PebInfo.ProcessParameters = Peb->ProcessParameters;
            PebInfo.Environment = Parameters64.Environment;

            printf("ImageBaseAddress:0x%x\r\n", PebInfo.ImageBaseAddress);
            printf("Ldr:0x%x\r\n", PebInfo.Ldr);
            printf("ProcessHeap:0x%x\r\n", PebInfo.ProcessHeap);
            printf("ProcessParameters:0x%x\r\n", PebInfo.ProcessParameters);
            printf("Environment:0x%x\r\n", PebInfo.Environment);
            while (Environment != NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)Environment, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength + 1];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)Environment, -1, v1, Parameters64.EnvironmentSize * 2, NULL, FALSE);
                printf("%s\r\n", v1);
                // 指针移动到字符串末尾  
                while (*(WCHAR*)Environment != '\0')
                    Environment += 2;
                Environment += 2;
                // 是否是最后一个字符串  
                if (*Environment == '\0')
                    break;
            }
            BYTE* CommandLine = new BYTE[Parameters64.CommandLine.Length];
            Status = NtWow64ReadVirtualMemory64(ProcessHandle, (PVOID64)Parameters64.CommandLine.Buffer, CommandLine, Parameters64.CommandLine.Length, NULL);
            if (CommandLine != NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)CommandLine, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)CommandLine, -1, v1, Parameters64.CommandLine.Length, NULL, FALSE);
                printf("CommandLine:%s\r\n", v1);
            }
            BYTE* ImagePathName = new BYTE[Parameters64.ImagePathName.Length];
            Status = NtWow64ReadVirtualMemory64(ProcessHandle, (PVOID64)Parameters64.ImagePathName.Buffer, ImagePathName, Parameters64.ImagePathName.Length, NULL);
            if (ImagePathName != NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)ImagePathName, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)ImagePathName, -1, v1, Parameters64.ImagePathName.Length, NULL, FALSE);
                printf("ImagePathName:%s\r\n", v1);
            }
        }

    }
    //自己是32  目标是32
    else if (bTarget == TRUE && bSource == TRUE)
    {
        HMODULE NtdllModule = GetModuleHandle("ntdll.dll");
        pfnNtQueryInformationProcess NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(NtdllModule,
            "NtQueryInformationProcess");
        PROCESS_BASIC_INFORMATION32 pbi = { 0 };
        UINT32  ReturnLength = 0;
        NTSTATUS Status = NtQueryInformationProcess(ProcessHandle,
            ProcessBasicInformation, &pbi, (UINT32)sizeof(pbi), (UINT32*)&ReturnLength);

        if (NT_SUCCESS(Status))
        {
            _PEB32* Peb = (_PEB32*)malloc(sizeof(_PEB32));
            Status  = ReadProcessMemory(ProcessHandle, (PVOID)pbi.PebBaseAddress, (_PEB32*)Peb, sizeof(_PEB32), NULL);

            _RTL_USER_PROCESS_PARAMETERS32 Parameters32;

            Status = ReadProcessMemory(ProcessHandle, (PVOID)Peb->ProcessParameters, &Parameters32, sizeof(_RTL_USER_PROCESS_PARAMETERS32), NULL);
            BYTE* Environment = new BYTE[Parameters32.EnvironmentSize*2];
            Status = ReadProcessMemory(ProcessHandle, (PVOID)Parameters32.Environment, Environment, Parameters32.EnvironmentSize, NULL);

            //赋值
            PebInfo.ImageBaseAddress = Peb->ImageBaseAddress;
            return (int)PebInfo.ImageBaseAddress;
            PebInfo.Ldr = Peb->Ldr;
            PebInfo.ProcessHeap = Peb->ProcessHeap;
            PebInfo.ProcessParameters = Peb->ProcessParameters;
            PebInfo.Environment = Parameters32.Environment;
            printf("ImageBaseAddress:0x%x\r\n", PebInfo.ImageBaseAddress);
            printf("Ldr:0x%x\r\n", PebInfo.Ldr);
            printf("ProcessHeap:0x%x\r\n", PebInfo.ProcessHeap);
            printf("ProcessParameters:0x%x\r\n", PebInfo.ProcessParameters);
            printf("Environment:0x%x\r\n", PebInfo.Environment);
            while (Environment !=NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)Environment, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength + 1];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)Environment, -1, v1, Parameters32.EnvironmentSize * 2, NULL, FALSE);
                printf("%s\r\n", v1);
                // 指针移动到字符串末尾  
                while (*(WCHAR*)Environment != '\0')
                    Environment +=2;
                Environment += 2;
                // 是否是最后一个字符串  
                if (*Environment == '\0')
                    break;
            }

            BYTE* CommandLine = new BYTE[Parameters32.CommandLine.Length];
            Status = ReadProcessMemory(ProcessHandle, (PVOID)Parameters32.CommandLine.Buffer, CommandLine, Parameters32.CommandLine.Length, NULL);
            if (CommandLine != NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)CommandLine, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)CommandLine, -1, v1, Parameters32.CommandLine.Length, NULL, FALSE);
                printf("CommandLine:%s\r\n", v1);
            }
            BYTE* ImagePathName = new BYTE[Parameters32.ImagePathName.Length];
            Status = ReadProcessMemory(ProcessHandle, (PVOID)Parameters32.ImagePathName.Buffer, ImagePathName, Parameters32.ImagePathName.Length, NULL);
            if (ImagePathName != NULL)
            {
                char* v1 = NULL;
                int DataLength = WideCharToMultiByte(CP_ACP, NULL, (WCHAR*)ImagePathName, -1, NULL, 0, NULL, FALSE);
                v1 = new char[DataLength];
                WideCharToMultiByte(CP_OEMCP, NULL, (WCHAR*)ImagePathName, -1, v1, Parameters32.ImagePathName.Length, NULL, FALSE);
                printf("ImagePathName:%s\r\n", v1);
            }
        }
    }	

    return 0;
}
