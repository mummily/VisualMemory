#pragma once

#include <windows.h>
#include <QString>

class Utils
{
public:
    static double getRealMemoryRatio(int singleSize);
    static double getRealMemorySize(int singleSize);
    static BOOL PauseResumeThreadList(DWORD dwOwnerPID, bool bResumeThread);
    static BOOL ProcessList();
    static int KillProcessByName(const QString& processName, const QString& processPath);
    static int PauseResumeProcessByName(const QString& processName, bool bResume);
    static int AddProcessDirToPath(const QString& processName);
    static QString s_openAppName;

    static int getProcessIDByName(const QString& processName);

    // 递归获取指定文件夹下所有文件
    static QStringList getFileList(const QString &path);

    static void dumpStack(void);
    static int dumpStack3(PVOID *Callers, ULONG Count);
    static bool readFuncInfoFromAddressList(QSet<__int64> &addressList);
    static QString getInfoFromAddress(int address);
    static int getAddressFromFuncName(const QString funcName);

    static QVector<int> GetProcessThreadIDList();
    static QVector<HANDLE> GetProcessThreadHandleList();

    static void initProcessWindbg();
    static void cleanupWindbg();

    static bool lookHeapMemory(PVOID ptr);
    static bool lookHeapMemory2(PVOID ptr, int size);
    static bool lookHeapMemory3(PVOID ptr);
    static bool lookHeapMemory4(PVOID ptr);
    static bool clearHeapMemoryCache();

    // 这部分应该建个专用类了
    static bool initModulesInfo();
    static QString getModuleNameFromAddress(PVOID address);
    static QString getModuleNameAndRelativeAddress(PVOID address, DWORD &relativeAddress);

};

// 全局变量区，定义一些全局变量和设置，方便编码和兼容多种模式，暂时放到这里

class GlobalData
{
public:
    GlobalData()
    {
        memoryMode = 1;
        isAutoMemoryLeakMode = false;
        isPreciseMemoryLeakMode = false;
        isInMemoryLeakRange = false;
        curStep = 0;
        totalLeakMemorySize = 0;
        totalLeakMemoryTimes = 0;
        hookTotalLeakMemorySize = 0;
        hookTotalLeakMemoryTimes = 0;
        totalLeakMemoryTimes_BeforeHook = 0;
        totalLeakMemorySize_BeforeHook = 0;
        totalLeakMemoryTimes_Last = 0;
        totalLeakMemorySize_Last = 0;
        relativeTotalLeakMemoryTimes = 0;
        relativeTotalLeakMemorySize = 0;
    }

    static GlobalData *getInstance()
    {
        static GlobalData s_globalData;
        return &s_globalData;
    }

    int memoryMode; // 内存模式：0-内存泄漏模式；1-内存统计模式
    bool isPreciseMemoryLeakMode; // 是否精准内存泄漏模式，精准泄漏模式下只统计内存泄漏的根节点，而不包含关联内存泄漏
    bool isAutoMemoryLeakMode;
    bool isInMemoryLeakRange;
    int curStep; // 当前所处状态 0-初始； 1-已启动进程； 2-开始统计； 3-结束统计； 4-进程退出

    int totalLeakMemorySize;
    int totalLeakMemoryTimes;
    int hookTotalLeakMemorySize;
    int hookTotalLeakMemoryTimes;

    int totalLeakMemoryTimes_BeforeHook; // hook前已有的内存数
    int totalLeakMemorySize_BeforeHook; // hook前已有的内存量
    int totalLeakMemoryTimes_Last; // 上一次的内存数
    int totalLeakMemorySize_Last; // 上一次的内存量
    int relativeTotalLeakMemoryTimes; // 开始到结束时的总共内存增长数
    int relativeTotalLeakMemorySize; // 开始到结束时的总共内存增长量
};

// 消息容器，先放Utils下
#define GLOBAL_STATICS_START_EVENT  ("Local\\GLOBAL_STATICS_START_EVENT") 
#define GLOBAL_STATICS_STOP_EVENT  ("Local\\GLOBAL_STATICS_STOP_EVENT") 
#define GLOBAL_PROCESS_EXIT_EVENT  ("Local\\GLOBAL_PROCESS_EXIT_EVENT") 
#define GLOBAL_CATHE_RECEIVED_EVENT  ("Local\\GLOBAL_CATHE_RECEIVED_EVENT") 

class EventController
{
public:
    EventController()
    {
        hStartEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_START_EVENT);
        hStopEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_STOP_EVENT);
        hExitEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_PROCESS_EXIT_EVENT);
        hCatheReceivedEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_CATHE_RECEIVED_EVENT);
    }

    static EventController *getInstance()
    {
        static EventController s_eventController;
        return &s_eventController;
    }

    HANDLE hStartEvent;
    HANDLE hStopEvent;
    HANDLE hExitEvent;
    HANDLE hCatheReceivedEvent;
};
