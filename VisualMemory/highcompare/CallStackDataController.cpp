#include "CallStackDataController.h"
#include <qdebug.h>
#include <qt_windows.h>
#include <QThread>
#include <QSettings>
#include <QApplication>
#include "Utils.h"
#include <CommonDef.h>

// 轻量级锁，非内核对象
static CRITICAL_SECTION g_cs;

// 调用栈处理线程
class ReadPDBThread : public QThread
{
public:
    ReadPDBThread()
    {

    }

    void run()
    {
        while (true)
        {
            ::Sleep(100);
            CallStackDataController::instance()->end();
        }

    }
};
ReadPDBThread g_ReadPDBThread;

CallStackDataController::CallStackDataController(void) : m_rootItem(nullptr), m_rootItemBottom2Top(nullptr)
{
    ::InitializeCriticalSection(&g_cs);
    m_PtrDataList.m_minCacheSize = 10000;
    m_PtrDataList.m_maxCacheSize = INT_MAX;
}


CallStackDataController::~CallStackDataController(void)
{
}



void CallStackDataController::addCallStackData(CallStackData *data)
{
    // 如果是内存泄漏模式，且不再统计区间，则不记录其堆栈，都指向一个空堆栈
    int sizePerTime = data->sizePerTime;
    //if (GlobalData::getInstance()->memoryMode == 0 && !GlobalData::getInstance()->isInMemoryLeakRange)
    //{
    //    data->depth = 0;
    //    data->total = 0;
    //    data->sizePerTime = 0;
    //}
    ////QString funcName;
    ////if (data->depth > 2)
    ////{
    ////    funcName = Utils::getInfoFromAddress((int)data->backtrace[2]);
    ////}
    ////if (!funcName.contains("malloc") && !funcName.contains("realloc"))
    ////{
    ////    //return;
    ////}
    ////if (funcName.contains("free") && !funcName.contains("MSVCR100.dll"))
    ////{
    ////    for (int i=0; i<data->depth; i++)
    ////    {
    ////        funcName = Utils::getInfoFromAddress((int)data->backtrace[i]);
    ////        qDebug() << "qingh-a, funcName=" << funcName;
    ////    }
    ////}
    if (this->containsPtr(data->ptr))
    {
        //qDebug() << "qingh-a, ptr has exists:" << (int)data;
        this->freeCallStackData(data->ptr);
    }

    //EnterCriticalSection(&g_cs);
    //m_mutex.lock();
	CallStackData *sameData = this->findSameCallStackHashMyself(data);
    if (sameData->isFirst)
    {
        sameData->isFirst = false;
        sameData->copyFrom(data);
        
        //if (GlobalData::getInstance()->memoryMode == 0 && GlobalData::getInstance()->isInMemoryLeakRange == false)
        {
            sameData->size = 0;
            sameData->times = 0;
            sameData->updateSize = 0;
            sameData->updateTimes = 0;
            sameData->msTime = 0;
            sameData->updateMsTime = 0;
        }
    }
    else if (GlobalData::getInstance()->memoryMode == 1 || GlobalData::getInstance()->memoryMode == 0 && GlobalData::getInstance()->isInMemoryLeakRange == true)
    {
        //sameData->size += data->size;
        //sameData->times += data->times;
        //sameData->updateSize += data->size;
        //sameData->updateTimes += data->times;
        //sameData->msTime += data->msTime;
        //sameData->updateMsTime += data->updateMsTime;
    }


    //if (m_mapPtr2Data.size() < 10000)
    //{
        //m_mapPtr2Data[data->ptr] = sameData;
        m_tmpSystemTime = data->systemTime;
        PtrNode *ptrNode = this->addPtr(data->ptr);
        ptrNode->data = sameData;
        ptrNode->size = sizePerTime;
        ptrNode->isMemoryLeakValid = GlobalData::getInstance()->isInMemoryLeakRange;
    //}
    //if (m_mapPtr2Data.size() % 100000 == 0)
    //{
    //    //qDebug() << "qingh-a, m_mapPtr2Data.size() = " << m_mapPtr2Data.size();
    //}
    //m_mutex.unlock();
    //LeaveCriticalSection(&g_cs); 
}

void CallStackDataController::freeCallStackData( void *ptr )
{
    //EnterCriticalSection(&g_cs);
    if (ptr == nullptr)
    {
        //LeaveCriticalSection(&g_cs); 
        return;
    }

    this->freePtr(ptr, false);

    ////m_mutex.lock();
    //CallStackData *curData = m_mapPtr2Data.value(ptr, nullptr);
    //if (curData == nullptr)
    //{
    //    //LeaveCriticalSection(&g_cs); 
    //    return;
    //}
    //
    //if (curData->times != 0)
    //{
    //    int size = curData->size / curData->times;
    //    curData->size -= size;
    //    --curData->times;

    //    curData->updateSize -= size;
    //    --curData->updateTimes;
    //}
    //
    //if (curData->times <= 0)
    //{
    //    m_mapPtr2Data.remove(ptr);
    //}
    ////m_mutex.unlock();
    ////LeaveCriticalSection(&g_cs); 
}

void CallStackDataController::freeCallStackData( CallStackData *data )
{
    m_tmpSystemTime = data->systemTime;
    freeCallStackData(data->ptr);
}

void CallStackDataController::checkHeapMemory( CallStackData *data )
{
    if (GlobalData::getInstance()->totalLeakMemoryTimes_BeforeHook == 0)
    {
        m_beforeHookPtrs << data->ptr;
    }
    else if (m_beforeHookPtrs.contains(data->ptr))
    {
        return;
    }

    if (GlobalData::getInstance()->memoryMode == 0)
    {
        GlobalData::getInstance()->totalLeakMemorySize += data->size;
        GlobalData::getInstance()->totalLeakMemoryTimes++;
    }
    

    static bool s_bFirst = true;
    // 第一次时清空当前有效内存
    if (s_bFirst)
    {
        s_bFirst = false;
        //this->clearAllPtr();
        //// 清空堆栈变更内存信息
        //SameTotalDatas **pp = m_SameTotalDatas;
        //for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
        //{
        //    if (*pp == nullptr)
        //    {
        //        continue;
        //    }

        //    CallStackData *curData = (*pp)->m_data;
        //    while (curData)
        //    {
        //        curData->times = 0;
        //        curData->size = 0;
        //        curData->msTime = 0;

        //        curData->updateTimes = 0;
        //        curData->updateSize = 0;
        //        curData->updateMsTime = 0;

        //        curData = curData->next;
        //    }
        //}
    }

    // 统计当前有效内存，若没有找到记录，忽略
    {
        UINT index = ((UINT)data->ptr / 4) % Same_Ptr_Hash_Size;
        SameHashPtr *sameHashPtr = m_sameHashPtrList[index];
        if (sameHashPtr == nullptr)
        {
            return;
        }

        if (sameHashPtr->beginNode == nullptr)
        {
            return;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            // 若是内存泄漏模式，则清空所有原始值，剩余内存全为泄漏
            if (GlobalData::getInstance()->memoryMode == 0)
            {
                CallStackData *curStackData = curData->data;
                curStackData->size = 0;
                curStackData->times = 0;
            }

            if (curData->ptr == data->ptr)
            {
                curData->isValidMemory = true;
                GlobalData::getInstance()->hookTotalLeakMemorySize += curData->size;
                GlobalData::getInstance()->hookTotalLeakMemoryTimes++;
                if (GlobalData::getInstance()->memoryMode != 0)
                {
                    CallStackData *curStackData = curData->data;
                    int size = curData->size;
                    //curStackData->size += size;
                    //++curStackData->times;

                    curStackData->updateSize += size;
                    ++curStackData->updateTimes;
                }
                else if (GlobalData::getInstance()->memoryMode == 0 && curData->isMemoryLeakValid)
                {
                    CallStackData *curStackData = curData->data;
                    int size = curData->size;
                    //curStackData->size += size;
                    //++curStackData->times;

                    curStackData->updateSize += size;
                    ++curStackData->updateTimes;
                }
                else
                {
                    // nothing
                }
                return;
            }

            lastValidData = curData;
            curData = curData->next;
        }
    }
}

CallStackData *CallStackDataController::findSameCallStack(CallStackData *data)
{
	for (int i=0; i<m_simpleDatas.size(); ++i)
	{
		CallStackData *curData = m_simpleDatas[i];
		if (curData->depth != data->depth)
		{
			continue;
		}
		for (int j=0; j<data->depth; ++j)
		{
			if (data->backtrace[j] != curData->backtrace[j])
			{
				break;
			}

			if (j == data->depth - 1)
			{
				return curData;
			}
		}
	}

    CallStackData *callStackData = new CallStackData();
    callStackData->copyFrom(data);
	return callStackData;
}

CallStackData *CallStackDataController::findSameCallStackHigh(CallStackData *data)
{
    SameDepthDatas *depthData = m_datas.value(data->depth, nullptr);
    if (depthData == nullptr)
    {
        depthData = this->newSaveDepthDatas(data->depth);
        SameTotalDatas *totalDatas = depthData->newSameTotalDatas(data->total);
        CallStackData *callStackData = totalDatas->newCallStackData();
        //callStackData->copyFrom(data);

        return callStackData;
    }
    SameTotalDatas *totalDatas = depthData->m_datas.value(data->total, nullptr);
    if (totalDatas == nullptr)
    {
        totalDatas = depthData->newSameTotalDatas(data->total);
        CallStackData *callStackData = totalDatas->newCallStackData();
        //callStackData->copyFrom(data);

        return callStackData;
    }

    for (int j=0; j<totalDatas->m_datas.size(); ++j)
    {
        CallStackData *curData = totalDatas->m_datas[j];
        if (data->isSameData(curData))
        {
            return curData;
        }
    }
    
    CallStackData *callStackData = totalDatas->newCallStackData();
    //callStackData->copyFrom(data);

    return callStackData;
}

SameDepthDatas * CallStackDataController::newSaveDepthDatas( int depth )
{
    SameDepthDatas *data = new SameDepthDatas();
    m_datas[depth] = data;
    return data;
}

void CallStackDataController::start()
{
    //g_ReadPDBThread.start();
    EnterCriticalSection(&g_cs);
    qDebug() << "qingh-a, CallStackDataController::start() begin";

    //if (m_rootItem)
    //{
    //    m_rootItem->clear();
    //    m_rootItem->m_bIsAddress2FunctionName = false;
    //}
    //if (m_rootItemBottom2Top)
    //{
    //    m_rootItemBottom2Top->clear();
    //    m_rootItemBottom2Top->m_bIsAddress2FunctionName = false;
    //}

    //dealwithDelayFreeDatas();
    //for (int i=0; i<m_simpleDatas.size(); ++i)
    //{
    //    CallStackData *data = m_simpleDatas[i];
    //    data->updateTimes = 0;
    //    data->updateSize = 0;
    //    data->updateMsTime = 0;
    //}

    // 清空堆栈变更内存信息
    //GlobalData::getInstance()->totalLeakMemoryTimes_BeforeHook = GlobalData::getInstance()->relativeTotalLeakMemoryTimes;
    //GlobalData::getInstance()->totalLeakMemorySize_BeforeHook = GlobalData::getInstance()->relativeTotalLeakMemorySize;
    SameTotalDatas **pp = m_SameTotalDatas;
    for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
    {
        if (*pp == nullptr)
        {
            continue;
        }

        CallStackData *curData = (*pp)->m_data;
        while (curData)
        {
            curData->times = curData->updateTimes;
            curData->size = curData->updateSize;
            if (GlobalData::getInstance()->memoryMode == 0)
            {
                curData->times = 0;
                curData->size = 0;
            }

            curData->updateTimes = 0;
            curData->updateSize = 0;

            curData = curData->next;
        }
    }

    // 如果是内存泄漏模式，清空所有指针信息
    //if (GlobalData::getInstance()->memoryMode == 0)
    {
        resetAllPtr();
    }

    //SameTotalDatas **pp = m_SameTotalDatas;
    //for (int i=0; i<Hash_Size; ++i)
    //{
    //    SameTotalDatas *sameTotalData = m_SameTotalDatas[i];
    //    if (sameTotalData == nullptr)
    //    {
    //        continue;
    //    }
    //    auto it = sameTotalData->m_SameDepthAndSizeDatas.begin();
    //    for (; it != sameTotalData->m_SameDepthAndSizeDatas.end(); ++it)
    //    {
    //        SameDepthAndSizeData *pSameDepthAndSizeData = it.value();
    //        for (int j=0; j<pSameDepthAndSizeData->m_datas.size(); ++j)
    //        {
    //            CallStackData *curData = pSameDepthAndSizeData->m_datas[j];
    //            curData->updateTimes = 0;
    //            curData->updateSize = 0;
    //            curData->updateMsTime = 0;
    //        }
    //    }
    //}

    LeaveCriticalSection(&g_cs);
}

void CallStackDataController::end()
{
    EnterCriticalSection(&g_cs);
    //dealwithDelayFreeDatas();

    if (m_rootItem == nullptr)
    {
        m_rootItem = new TreeItem();
    }
    if (m_rootItemBottom2Top == nullptr)
    {
        m_rootItemBottom2Top = new TreeItem();
    }

    if (m_rootItem)
    {
        m_rootItem->clear();
        m_rootItem->m_bIsAddress2FunctionName = false;
    }
    if (m_rootItemBottom2Top)
    {
        m_rootItemBottom2Top->clear();
        m_rootItemBottom2Top->m_bIsAddress2FunctionName = false;
    }

    // 非泄露模式下统计hook内存总数
    if (GlobalData::getInstance()->memoryMode != 0)
    {
        GlobalData::getInstance()->hookTotalLeakMemorySize = 0;
        GlobalData::getInstance()->hookTotalLeakMemoryTimes = 0;
        SameTotalDatas **pp = m_SameTotalDatas;
        for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
        {
            if (*pp == nullptr)
            {
                continue;
            }

            CallStackData *curData = (*pp)->m_data;
            while (curData)
            {
                if (curData->updateTimes == curData->times)
                {
                    curData = curData->next;
                    continue;
                }

                GlobalData::getInstance()->hookTotalLeakMemorySize += curData->updateSize - curData->size; // 为啥curData->data->sizePerTime为0;
                GlobalData::getInstance()->hookTotalLeakMemoryTimes += curData->updateTimes - curData->times;

                curData = curData->next;
            }
        }
    }

    // 精准内存泄漏模式下过滤掉关联内存
    if (GlobalData::getInstance()->memoryMode == 0 && GlobalData::getInstance()->isPreciseMemoryLeakMode)
    {
        this->refreshPtr2();
    }

    Utils::initModulesInfo();

    // 如果是内存泄漏模式，则先预读函数名（按地址大小排序）
    if (GlobalData::getInstance()->memoryMode == 0)
    {
        // 按地址顺序读取函数名，以加快读取速度
        QSet<__int64> addressList;  // QSet<PVOID>不带排序功能，不知为啥，这里用QSet<DWORD>;
        SameTotalDatas **pp = m_SameTotalDatas;
        for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
        {
            if (*pp == nullptr)
            {
                continue;
            }

            CallStackData *curData = (*pp)->m_data;
            while (curData)
            {
                if (curData->depth < 3 || curData->updateTimes == curData->times)
                {
                    curData = curData->next;
                    continue;
                }

                curData->addFuncAddress(addressList);

                curData = curData->next;
            }
        }

        Utils::readFuncInfoFromAddressList(addressList);
    }
    

    SameTotalDatas **pp = m_SameTotalDatas;
    for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
    {
        if (*pp == nullptr)
        {
            continue;
        }

        CallStackData *curData = (*pp)->m_data;
        while (curData)
        {
            if (curData->depth < 3 || curData->updateTimes == curData->times)
            {
                curData = curData->next;
                continue;
            }
            if (curData->depth == 3 && Utils::getInfoFromAddress((int)curData->backtrace[2]).contains("malloc"))
            {
                curData = curData->next;
                continue;
            }
            if (curData->depth == 4 && Utils::getInfoFromAddress((int)curData->backtrace[3]).contains("operator new"))
            {
                curData = curData->next;
                continue;
            }

            if (GlobalData::getInstance()->memoryMode == 0 && curData->updateTimes <= curData->times)
            {
                curData = curData->next;
                continue;
            }

            // 针对内存泄漏，去除多余释放项的干扰
            //if (curData->updateTimes <= 0)
            //{
            //    curData = curData->next;
            //    continue;
            //}

            m_rootItem->addAddress(curData, false);
            m_rootItemBottom2Top->addAddress(curData, true);
            //curData->updateSize = 0;
            //curData->updateTimes = 0;

            curData = curData->next;
        }
    }

    // 将当前结果值清空，以备下一次点结束时重新计算
    /*SameTotalDatas ***/pp = m_SameTotalDatas;
    for (int i=0; i<Same_Total_Hash_Size; ++pp, ++i)
    {
        if (*pp == nullptr)
        {
            continue;
        }

        CallStackData *curData = (*pp)->m_data;
        while (curData)
        {
            curData->updateSize = 0;
            curData->updateTimes = 0;

            curData = curData->next;
        }
    }
    
    LeaveCriticalSection(&g_cs);
}

CallStackDataController * CallStackDataController::instance()
{
    static CallStackDataController s_CallStackDataController;
    return &s_CallStackDataController;
}

CallStackData * CallStackDataController::findSameCallStackHashMyself( CallStackData *data )
{
    UINT index = ((UINT)data->total / 4) % Same_Total_Hash_Size;
    //qDebug() << "total index " << data->total << index;
    SameTotalDatas *sameTotalData = m_SameTotalDatas[index];
    if (sameTotalData == nullptr)
    {
        sameTotalData = new SameTotalDatas();
        m_SameTotalDatas[index] = sameTotalData;
        sameTotalData->m_data = new CallStackData();
        return sameTotalData->m_data;
    }

    CallStackData *curData = sameTotalData->m_data;
    CallStackData *curValidData = curData;
    while (curData)
    {
        curValidData = curData;
        if (curData->isSameData(data))
        {
            return curData;
        }
        curData = curData->next;
    }

    curValidData->next = new CallStackData();
    return curValidData->next;
}

CallStackData * CallStackDataController::findSameCallStackHashMyself2( CallStackData *data )
{
    UINT index = ((UINT)data->total / 4) % Same_Total_Hash_Size;
    //qDebug() << "total index " << data->total << index;
    SameTotalDatas *sameTotalData = m_SameTotalDatas[index];
    int depthAndSizeIndex = data->depth;
    if (sameTotalData == nullptr)
    {
        sameTotalData = new SameTotalDatas();
        m_SameTotalDatas[index] = sameTotalData;
        
        SameDepthAndSizeData *depthAndSizeData = new SameDepthAndSizeData();
        sameTotalData->m_SameDepthAndSizeDatas[depthAndSizeIndex] = depthAndSizeData;
        CallStackData *callStackData = new CallStackData();
        depthAndSizeData->m_datas << callStackData;
        return callStackData;
    }

    SameDepthAndSizeData *depthAndSizeData = sameTotalData->m_SameDepthAndSizeDatas.value(depthAndSizeIndex, nullptr);
    if (depthAndSizeData == nullptr)
    {
        depthAndSizeData = new SameDepthAndSizeData();
        sameTotalData->m_SameDepthAndSizeDatas[depthAndSizeIndex] = depthAndSizeData;
        CallStackData *callStackData = new CallStackData();
        depthAndSizeData->m_datas << callStackData;
        return callStackData;
    }

    for (int i=0; i<depthAndSizeData->m_datas.size(); ++i)
    {
        CallStackData *curData = depthAndSizeData->m_datas[i];
        if (curData->isSameData(data))
        {
            return curData;
        }
    }

    CallStackData *curData = new CallStackData();
    depthAndSizeData->m_datas << curData;
    return curData;
}

PtrNode * CallStackDataController::addPtr(PVOID ptr)
{
    //m_ptrMap.insert(ptr, ptr);

    //if (m_ptrMap.count(ptr) > 1)
    //{
    //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
    //}

    UINT index = ((UINT)ptr / 4) % Same_Ptr_Hash_Size;
    SameHashPtr *sameHashPtr = m_sameHashPtrList[index];
    if (sameHashPtr == nullptr)
    {
        sameHashPtr = new SameHashPtr();
        m_sameHashPtrList[index] = sameHashPtr;
        sameHashPtr->hash = index;
        sameHashPtr->beginNode = m_PtrDataList.getNextEndStream();
        sameHashPtr->beginNode->ptr = ptr;
        sameHashPtr->beginNode->systemTime = m_tmpSystemTime;
        sameHashPtr->endNode = sameHashPtr->beginNode;
        return sameHashPtr->beginNode;
    }

    if (sameHashPtr->beginNode == nullptr)
    {
        PtrNode *curNode = m_PtrDataList.getNextEndStream();
        sameHashPtr->beginNode = curNode;
        sameHashPtr->endNode = curNode;
        curNode->ptr = ptr;
        curNode->systemTime = m_tmpSystemTime;
        return curNode;
    }
    else
    {
        PtrNode *curNode = m_PtrDataList.getNextEndStream();
        sameHashPtr->endNode->next = curNode;
        curNode->ptr = ptr;
        curNode->systemTime = m_tmpSystemTime;
        sameHashPtr->endNode = curNode;
        return curNode;
    }

    //PtrNode *returnData = nullptr;
    //if (sameHashPtr->freeNode != nullptr)
    //{
    //    returnData = sameHashPtr->freeNode;
    //    returnData->ptr = ptr;
    //    sameHashPtr->freeNode = nullptr;
    //}

    //PtrNode *curData = sameHashPtr->beginNode;
    //if (returnData != nullptr)
    //{
    //    curData = returnData->next;
    //}
    //while (curData != nullptr)
    //{
    //    if (curData->ptr == nullptr)
    //    {
    //        if (returnData != nullptr)
    //        {
    //            sameHashPtr->freeNode = curData;
    //            return returnData;
    //        }
    //        else
    //        {
    //            curData->ptr = ptr;
    //            returnData = curData;
    //            sameHashPtr->freeNode = nullptr;
    //        }
    //    }
    //    curData = curData->next;
    //}

    //if (returnData)
    //{
    //    return returnData;
    //}

    //PtrNode *curNode = new PtrNode();
    //sameHashPtr->endNode->next = curNode;
    //curNode->ptr = ptr;
    //sameHashPtr->endNode = curNode;
    //return curNode;
}

bool CallStackDataController::freePtr(PVOID ptr, bool delayFree)
{
    //if (m_ptrMap.count(ptr) > 1)
    //{
    //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
    //}

    UINT index = ((UINT)ptr / 4) % Same_Ptr_Hash_Size;
    SameHashPtr *sameHashPtr = m_sameHashPtrList[index];
    if (sameHashPtr == nullptr)
    {
        return false;
    }

    if (sameHashPtr->beginNode == nullptr)
    {
        return false;
    }

    PtrNode *curData = sameHashPtr->beginNode;
    PtrNode *lastValidData = nullptr;
    while (curData != nullptr)
    {
        if (curData->ptr == ptr)
        {
            if (curData->data)
            {
                CallStackData *curStackData = curData->data;
                ////if (curStackData && GlobalData::getInstance()->memoryMode == 1 || curStackData && GlobalData::getInstance()->memoryMode == 0 && curData->isMemoryLeakValid)
                //if (curStackData && GlobalData::getInstance()->memoryMode == 0 && curData->isMemoryLeakValid && GlobalData::getInstance()->curStep == 3)
                //{
                //    int size = curData->size;
                //    //qDebug() << "size=" << size;
                //    //curStackData->size -= size;
                //    //--curStackData->times;

                //    curStackData->updateSize -= size;
                //    --curStackData->updateTimes;
                //}

                if (curData == sameHashPtr->beginNode)
                {
                    sameHashPtr->beginNode = curData->next;
                }
                else
                {
                    lastValidData->next = curData->next;
                }
                if (sameHashPtr->endNode == curData)
                {
                    sameHashPtr->endNode = lastValidData;
                    if (sameHashPtr->beginNode != nullptr && sameHashPtr->endNode == nullptr)
                    {
                        sameHashPtr->endNode = nullptr;
                    }
                }
                //delete curData;
                curData->data = nullptr;
                curData->ptr = nullptr;
                curData->next = nullptr;
                curData->systemTime = 0;
                m_PtrDataList.addFreeStream(curData);
                curData = nullptr;

                //int count = m_ptrMap.count(ptr);
                //if (m_ptrMap.count(ptr) > 1)
                //{
                //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
                //}
                //m_ptrMap.remove(ptr);
                //for (int i=0; i<count-1; i++)
                //{
                //    m_ptrMap.insert(ptr, ptr);
                //}
                return true;
            }
        }
        lastValidData = curData;
        curData = curData->next;
    }

    //if (!delayFree)
    //{
    //    m_delayFreeDatas.push_back(FreePtrInfo(ptr, m_tmpSystemTime));
    //}

    m_beforeHookPtrs.remove(ptr);
    
    return false;
}

bool CallStackDataController::refreshPtr2()
{
    m_sameHashPtrList[Same_Ptr_Hash_Size];
    for (int i=0; i < Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            if (curData->data && curData->data->updateTimes <= curData->data->times || !curData->isValidMemory)
            {
                lastValidData = curData;
                curData = curData->next;
                continue;
            }
            Utils::lookHeapMemory4(curData->ptr);
            lastValidData = curData;
            curData = curData->next;
        }
    }
    for (int i=0; i < Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            if (curData->data && !curData->isValidMemory)
            {
                lastValidData = curData;
                curData = curData->next;
                continue;
            }
            Utils::lookHeapMemory2(curData->ptr, curData->size);
            lastValidData = curData;
            curData = curData->next;
        }
    }

    for (int i=0; i < Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            if (curData->data && curData->data->updateTimes <= curData->data->times || !curData->isValidMemory)
            {
                lastValidData = curData;
                curData = curData->next;
                continue;
            }
            if (Utils::lookHeapMemory3(curData->ptr))
            {
                //qDebug() << "curData" << (int)curData->ptr << curData->data->sizePerTime;
                //lastValidData = curData;
                //curData = curData->next;
                //continue;
                if (curData->data)
                {
                    CallStackData *curStackData = curData->data;
                    if (curStackData/* && curStackData->times != 0*/)
                    {
                        int size = curData->size;
                        //qDebug() << "size=" << size;
                        //curStackData->size -= size; // 要减都减，等于白减，所以注掉
                        //--curStackData->times;

                        curStackData->updateSize -= size;
                        --curStackData->updateTimes;
                    }

                    if (curData == sameHashPtr->beginNode)
                    {
                        sameHashPtr->beginNode = curData->next;
                    }
                    else
                    {
                        lastValidData->next = curData->next;
                    }
                    if (sameHashPtr->endNode == curData)
                    {
                        sameHashPtr->endNode = lastValidData;
                        if (sameHashPtr->beginNode != nullptr && sameHashPtr->endNode == nullptr)
                        {
                            sameHashPtr->endNode = nullptr;
                        }
                    }

                    PtrNode *curDataTemp = curData;
                    curData = curData->next;

                    //delete curData;
                    curDataTemp->data = nullptr;
                    curDataTemp->ptr = nullptr;
                    curDataTemp->next = nullptr;
                    curDataTemp->systemTime = 0;
                    m_PtrDataList.addFreeStream(curDataTemp);
                    curDataTemp = nullptr;

                    //int count = m_ptrMap.count(ptr);
                    //if (m_ptrMap.count(ptr) > 1)
                    //{
                    //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
                    //}
                    //m_ptrMap.remove(ptr);
                    //for (int i=0; i<count-1; i++)
                    //{
                    //    m_ptrMap.insert(ptr, ptr);
                    //}
                    continue;
                }
            }
            else
            {
                //qDebug() << "curData" << (int)curData->ptr << curData->data->sizePerTime;
            }
            lastValidData = curData;
            curData = curData->next;
        }
    }

    Utils::clearHeapMemoryCache();

    return true;
}

bool CallStackDataController::refreshPtr()
{
    //if (m_ptrMap.count(ptr) > 1)
    //{
    //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
    //}

    m_sameHashPtrList[Same_Ptr_Hash_Size];
    for (int i=0; i<Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            if (curData->data && curData->data->updateTimes <= curData->data->times)
            {
                lastValidData = curData;
                curData = curData->next;
                continue;
            }
            if (Utils::lookHeapMemory(curData->ptr))
            {
                qDebug() << "curData" << curData->ptr << curData->size;
                //lastValidData = curData;
                //curData = curData->next;
                //continue;
                if (curData->data)
                {
                    CallStackData *curStackData = curData->data;
                    //if (curStackData && GlobalData::getInstance()->memoryMode == 0 || curStackData && GlobalData::getInstance()->memoryMode == 0 && curData->isMemoryLeakValid)
                    {
                        int size = curData->size;
                        //qDebug() << "size=" << size;
                        curStackData->size -= size;
                        --curStackData->times;

                        curStackData->updateSize -= size;
                        --curStackData->updateTimes;
                    }

                    if (curData == sameHashPtr->beginNode)
                    {
                        sameHashPtr->beginNode = curData->next;
                    }
                    else
                    {
                        lastValidData->next = curData->next;
                    }
                    if (sameHashPtr->endNode == curData)
                    {
                        sameHashPtr->endNode = lastValidData;
                        if (sameHashPtr->beginNode != nullptr && sameHashPtr->endNode == nullptr)
                        {
                            sameHashPtr->endNode = nullptr;
                        }
                    }

                    PtrNode *curDataTemp = curData;
                    curData = curData->next;

                    //delete curData;
                    curDataTemp->data = nullptr;
                    curDataTemp->ptr = nullptr;
                    curDataTemp->next = nullptr;
                    curDataTemp->systemTime = 0;
                    m_PtrDataList.addFreeStream(curDataTemp);
                    curDataTemp = nullptr;

                    //int count = m_ptrMap.count(ptr);
                    //if (m_ptrMap.count(ptr) > 1)
                    //{
                    //    qDebug() << "qingh-a, m_ptrMap.count(ptr) = " << m_ptrMap.count(ptr);
                    //}
                    //m_ptrMap.remove(ptr);
                    //for (int i=0; i<count-1; i++)
                    //{
                    //    m_ptrMap.insert(ptr, ptr);
                    //}
                    continue;
                }
            }
            lastValidData = curData;
            curData = curData->next;
        }
    }

    return true;
}

bool CallStackDataController::resetAllPtr()
{
    GlobalData::getInstance()->totalLeakMemoryTimes_Last = GlobalData::getInstance()->totalLeakMemoryTimes;
    GlobalData::getInstance()->totalLeakMemorySize_Last = GlobalData::getInstance()->totalLeakMemorySize;

    m_sameHashPtrList[Same_Ptr_Hash_Size];
    for (int i=0; i<Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            // 清除所有指针
            if (true)
            {
                if (curData->data)
                {
                    curData->isMemoryLeakValid = false;
                    curData->data->updateSize = 0;
                    curData->data->updateTimes = 0;
                }
            }
            lastValidData = curData;
            curData = curData->next;
        }
    }

    return true;
}

bool CallStackDataController::clearAllPtr()
{
    GlobalData::getInstance()->totalLeakMemorySize = 0;
    GlobalData::getInstance()->totalLeakMemoryTimes = 0;
    GlobalData::getInstance()->hookTotalLeakMemorySize = 0;
    GlobalData::getInstance()->hookTotalLeakMemoryTimes = 0;

    for (int i=0; i < Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            {
                if (curData)
                {
                    CallStackData *curStackData = curData->data;
                    if (curStackData)
                    {
                        curStackData->size = 0;
                        curStackData->times = 0;

                        curStackData->updateSize = 0;
                        curStackData->updateTimes = 0;
                    }

                    if (curData == sameHashPtr->beginNode)
                    {
                        sameHashPtr->beginNode = curData->next;
                    }
                    else
                    {
                        lastValidData->next = curData->next;
                    }
                    if (sameHashPtr->endNode == curData)
                    {
                        sameHashPtr->endNode = lastValidData;
                        if (sameHashPtr->beginNode != nullptr && sameHashPtr->endNode == nullptr)
                        {
                            sameHashPtr->endNode = nullptr;
                        }
                    }

                    PtrNode *curDataTemp = curData;
                    curData = curData->next;

                    //delete curData;
                    curDataTemp->data = nullptr;
                    curDataTemp->ptr = nullptr;
                    curDataTemp->next = nullptr;
                    curDataTemp->systemTime = 0;
                    m_PtrDataList.addFreeStream(curDataTemp);
                    curDataTemp = nullptr;
                    continue;
                }
            }
            lastValidData = curData;
            curData = curData->next;
        }
    }

    return true;
}

bool CallStackDataController::clearCurrentMemorySize()
{
    GlobalData::getInstance()->totalLeakMemorySize = 0;
    GlobalData::getInstance()->totalLeakMemoryTimes = 0;
    GlobalData::getInstance()->hookTotalLeakMemorySize = 0;
    GlobalData::getInstance()->hookTotalLeakMemoryTimes = 0;

    m_sameHashPtrList[Same_Ptr_Hash_Size];
    for (int i=0; i<Same_Ptr_Hash_Size; i++)
    {
        SameHashPtr *sameHashPtr = m_sameHashPtrList[i];
        if (sameHashPtr == nullptr)
        {
            continue;
        }
        if (sameHashPtr->beginNode == nullptr)
        {
            continue;
        }

        PtrNode *curData = sameHashPtr->beginNode;
        PtrNode *lastValidData = nullptr;
        while (curData != nullptr)
        {
            // 清除所有指针
            if (true)
            {
                if (curData->data)
                {
                    curData->data->updateSize = 0;
                    curData->data->updateTimes = 0;
                }
            }
            lastValidData = curData;
            curData = curData->next;
        }
    }

    return true;
}

bool CallStackDataController::containsPtr( PVOID ptr )
{
    UINT index = ((UINT)ptr / 4) % Same_Ptr_Hash_Size;
    SameHashPtr *sameHashPtr = m_sameHashPtrList[index];
    if (sameHashPtr == nullptr)
    {
        return false;
    }

    if (sameHashPtr->beginNode == nullptr)
    {
        return false;
    }

    PtrNode *curData = sameHashPtr->beginNode;
    PtrNode *lastValidData = nullptr;
    while (curData != nullptr)
    {
        if (curData->ptr == ptr)
        {
            return true;
        }
        lastValidData = curData;
        curData = curData->next;
    }

    return false;
}

void CallStackDataController::lock()
{
    EnterCriticalSection(&g_cs);
}

void CallStackDataController::unlock()
{
    LeaveCriticalSection(&g_cs);
}

void CallStackDataController::dealwithDelayFreeDatas()
{
    QQueue<FreePtrInfo> m_delayFreeDatas2 = m_delayFreeDatas;
    m_delayFreeDatas.clear();
    int i = 0;
    while (i < m_delayFreeDatas2.size())
    {
        const FreePtrInfo &freePtrInfo = m_delayFreeDatas2.at(i);
        m_tmpSystemTime = freePtrInfo.systemTime;
        bool res = this->freePtr(freePtrInfo.ptr, true);
        if (res == false)
        {
            //m_delayFreeDatas2.pop_front();
        }
        else
        {
            //m_delayFreeDatas2.pop_front();
        }
        ++i;
    }
    m_delayFreeDatas2.clear();
    qDebug() << "qingh-a, m_delayFreeDatas.size()" << m_delayFreeDatas.size();
}

bool CallStackData::isSameData( CallStackData *data )
{
    //qDebug() << "data->sizePerTime sizePerTime" << data->sizePerTime << sizePerTime;
    if (data->sizePerTime == sizePerTime)
    {
        if (memcmp(data->backtrace, backtrace, depth * 4) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    //if (data->times != 0 && times != 0 && data->size/data->times != size/times)
    //{
    //    return false;
    //}

    //for (int i=0; i<depth; ++i)
    //{
    //    if (data->backtrace[i] != backtrace[i])
    //    {
    //        return false;
    //    }
    //}

    
    

    //return false;
}

void CallStackData::copyFrom( CallStackData *data )
{
    ptr = data->ptr;
    size = data->size;
    depth = data->depth;
    times = data->times;
    updateTimes = data->times;
    updateSize = data->size;
    msTime = data->msTime;
    updateMsTime = data->updateMsTime;
    sizePerTime = data->sizePerTime;
    systemTime = data->systemTime;
    for (int i=0; i<data->depth; ++i)
    {
        backtrace[i] = data->backtrace[i];
    }
}

CallStackData * SameTotalDatas::newCallStackData()
{
    CallStackData *data = new CallStackData();
    m_datas << data;
    return data;
}

SameTotalDatas * SameDepthDatas::newSameTotalDatas( int total )
{
    SameTotalDatas *data = new SameTotalDatas();
    m_datas[total] = data;
    return data;
}
