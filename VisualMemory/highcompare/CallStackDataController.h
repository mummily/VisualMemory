#pragma once
#include "CommonDef.h"
#include "qt_windows.h"
#include <qmap.h>
#include <QVector>
#include <QMutex>
#include <QQueue>
#include <QSet>
#include "tree/treeitem.h"

//#define Same_Total_Hash_Size (1024*1024*10)
//#define Same_Ptr_Hash_Size (1024*1024*100)

#define Same_Total_Hash_Size (1024*1024*10)
#define Same_Ptr_Hash_Size (1024*1024*100)

class CallStackData
{
public:
    void *ptr;
	int times;
	int size;
    int updateTimes;
    int updateSize;
	int depth;
    int total;
    int msTime;
    int updateMsTime;
    bool isFirst;
    CallStackData *next;
    __int64 systemTime;

    int sizePerTime;
    PVOID backtrace[MAX_STACK_DEPTH];
	

    CallStackData()
    {
        times = 0;
        size = 0;
        updateTimes = 0;
        updateSize = 0;
        depth = 0;
        total = 0;
        msTime = 0;
        updateMsTime = 0;
        isFirst = true;
        next = nullptr;
        systemTime = 0;
    }
    bool isSameData(CallStackData *data);
    void copyFrom(CallStackData *data);
    void addFuncAddress(QSet<__int64> &addressList)
    {
        for (int i=0; i<depth; ++i)
        {
            if (!addressList.contains((DWORD)backtrace[i]))
            {
                addressList.insert((__int64)backtrace[i]);
            }
        }
    }
};

class PtrNode
{
public:
    PtrNode *next;
    PVOID ptr;
    CallStackData *data; // 在内存泄漏模式下，检测区间之外的可能会没有对应函数调用堆栈，故可能会赋空值。
    int size;
    __int64 systemTime;
    bool isMemoryLeakValid; // 是否内存泄漏关注指针，仅在内存泄漏模式下起作用
    bool isValidMemory; // 是否是有效内存

    PtrNode()
    {
        next = nullptr;
        ptr = nullptr;
        data = nullptr;
        size = 0;
        systemTime = 0;
        isMemoryLeakValid = true;
        isValidMemory = false;
    }
};

class FreePtrInfo
{
public:
    PVOID ptr;
    __int64 systemTime;

    FreePtrInfo()
    {
        ptr = nullptr;
        systemTime = 0;
    }

    FreePtrInfo(PVOID aPtr, int aSystemTime)
    {
        ptr = aPtr;
        systemTime = aSystemTime;
    }

    FreePtrInfo(const FreePtrInfo &other)
    {
        ptr = other.ptr;
        systemTime = other.systemTime;
    }

    FreePtrInfo &operator=(FreePtrInfo &other)
    {
        ptr = other.ptr;
        systemTime = other.systemTime;
        return *this;
    }
};

class SameDepthAndSizeData
{
public:
    int depthAndSize;
    QVector<CallStackData*> m_datas;
};

class SameTotalDatas
{
public:
	int total;
	QVector<CallStackData*> m_datas;
    CallStackData *m_data;
    QMap<int, SameDepthAndSizeData*> m_SameDepthAndSizeDatas;

    CallStackData *newCallStackData();

    SameTotalDatas()
    {
        m_data = nullptr;
    }
};

class SameHashPtr
{
public:
    int hash;
    PtrNode *beginNode; 
    PtrNode *endNode;
    PtrNode *freeNode;

    SameHashPtr()
    {
        hash = -1;
        beginNode = nullptr;
        endNode = nullptr;
        freeNode = nullptr;
    }
};

class SameDepthDatas
{
public:
	int depth;
	QMap<int /*total*/, SameTotalDatas*> m_datas;

    SameTotalDatas *newSameTotalDatas(int total);
};

class CallStackDataController
{
public:
	CallStackDataController(void);
	~CallStackDataController(void);

    static CallStackDataController *instance();

	void addCallStackData(CallStackData *data);
    void freeCallStackData(void *ptr);
    void freeCallStackData(CallStackData *data);
    void checkHeapMemory(CallStackData *data);
    void dealwithDelayFreeDatas();

    void start();
    void end();
    bool resetAllPtr();
    bool clearAllPtr();
    bool clearCurrentMemorySize();

    void lock();
    void unlock();

	QMap<int /*depth*/, SameDepthDatas*> m_datas;
	QVector<CallStackData*> m_simpleDatas;
    QMap<void*, CallStackData*> m_mapPtr2Data;

    TreeItem *m_rootItem;
    TreeItem *m_rootItemBottom2Top;

    SameTotalDatas* m_SameTotalDatas[Same_Total_Hash_Size];
    SameHashPtr *m_sameHashPtrList[Same_Ptr_Hash_Size];
    CallStackData m_nullCallStackData; // 利用空值模式，解决大量地方赋空值的情况

    CacheDataList<PtrNode> m_PtrDataList;
    QQueue<FreePtrInfo> m_delayFreeDatas;
    QSet<PVOID> m_beforeHookPtrs;

    __int64 m_tmpSystemTime;

protected:
    SameDepthDatas *newSaveDepthDatas(int depth);
	CallStackData *findSameCallStack(CallStackData *data);
    CallStackData *findSameCallStackHigh(CallStackData *data);
    CallStackData *findSameCallStackHashMyself(CallStackData *data);
    CallStackData *findSameCallStackHashMyself2(CallStackData *data);
    PtrNode *addPtr(PVOID ptr);
    bool freePtr(PVOID ptr, bool delayFree);
    bool containsPtr(PVOID ptr);
    bool refreshPtr();
    bool refreshPtr2();
};
