/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>
#include <QDebug>

#include "treeitem.h"
#include "highcompare/CallStackDataController.h"
#include "Utils.h"


//! [0]
TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
    m_memoryRealLocalSize = 0;
    m_memoryLocalSize = 0;
    m_memoryCount = 0;
    m_address = 0;
    m_msTime = 0;
    m_bIsAddress2FunctionName = false;
    m_bChildIsAddress2FunctionName = false;
    m_topType = -1;
    m_branchNumber = 0;
}

TreeItem::TreeItem( TreeItem *parent /*= 0*/ )
{
    parentItem = parent;
    m_memoryRealLocalSize = 0;
    m_memoryLocalSize = 0;
    m_memoryCount = 0;
    m_address = 0;
    m_msTime = 0;
    m_bIsAddress2FunctionName = false;
    m_bChildIsAddress2FunctionName = false;
    m_topType = -1;
    m_branchNumber = 0;
}

//! [0]

//! [1]
TreeItem::~TreeItem()
{
    qDeleteAll(m_childIItemList);
}
//! [1]

void TreeItem::clear()
{
    //qDeleteAll(m_childIItemList);
    m_childIItemList.clear();
    m_childItemMap.clear();

    m_memoryLocalSize = 0;
    m_memoryRealLocalSize = 0;
    m_memoryCount = 0;
    m_branchNumber = 1;
}

//! [2]
void TreeItem::appendChild(TreeItem *item)
{
    item->parentItem = this;
    m_childIItemList.append(item);
    m_childItemMap.insert(item->m_functionName, item);

    m_addressChild.insert(item->m_address, item);
}

void TreeItem::appendChild( TreeItem *pChild, int memorySize )
{
    //if (!m_childItemMap.contains(pChild->m_functionName))
    //{
    //    m_childItemMap[pChild->m_functionName] = pChild;
    //    m_childMemorySize[pChild] = memorySize;
    //    m_childIItemList << pChild;
    //}
    //else
    //{
    //    m_childMemorySize[pChild] += memorySize;
    //}
}

//void TreeItem::appendChild( QString functionName, int memorySize )
//{
//    TreeItem *pChild = m_childItems.value(functionName, nullptr);
//    if (pChild == nullptr)
//    {
//        pChild = new TreeItem(this);
//        pChild->m_functionName = functionName;
//        pChild->m_memoryAllSize = memorySize;
//    }
//    else
//    {
//        pChild->m_memoryAllSize += memorySize;
//    }
//
//    if (m_childMemorySize.contains(pChild))
//    {
//        m_childMemorySize[pChild] = memorySize;
//    }
//    else
//    {
//        m_childMemorySize[pChild] += memorySize;
//    }
//}

void TreeItem::mergeSameSmallChild(int &nDepth, bool mergeSameTop, QMap<QString, TreeItem*> *pTopItems, QSet<TreeItem*> *parents/* = nullptr*/)
{
    nDepth++;
    if (parents)
    {
        parents->insert(this);
    }
    //qDebug() << "nDepth" << nDepth;
    if (!m_childIItemList.isEmpty())
    {
        QMultiMap<QString, TreeItem*> smallChildItemMap = m_childItemMap;
        m_childIItemList.clear();
        m_childItemMap.clear();
        TreeItem *pLastValidItem = nullptr;
        for (auto i=smallChildItemMap.begin(); i!=smallChildItemMap.end(); i++)
        {
            TreeItem *pCurItem = i.value();
            TreeItem *pValidItem = m_childItemMap.value(pCurItem->m_functionName, nullptr);
            if (pValidItem == nullptr)
            {
                pValidItem = pCurItem;
                this->appendChild(pValidItem);
            }
            else
            {
                pValidItem->m_memoryLocalSize += pCurItem->m_memoryLocalSize;
                pValidItem->m_memoryRealLocalSize += pCurItem->m_memoryRealLocalSize;
                pValidItem->m_memoryCount += pCurItem->m_memoryCount;
                pValidItem->m_msTime += pCurItem->m_msTime;
                //pValidItem->m_branchNumber += pCurItem->m_branchNumber;
                for (int i=0; i<pCurItem->childCount(); i++)
                {
                    pValidItem->appendChild(pCurItem->child(i));
                }
                pCurItem->clearChildNotDelete();
                delete pCurItem;
                pCurItem = nullptr;
            }

            // 若不是顶层节点，考虑将同名的顶层节点合并进来
            if (pLastValidItem != pValidItem && this->parentItem && this->parentItem->parentItem)
            {
                if (mergeSameTop && pTopItems)
                {
                    TreeItem *pTopItem = pTopItems->value(pValidItem->m_functionName, nullptr);
                    if (pTopItem)
                    {
                        if (pTopItem->m_topType == -1)
                        {
                            pTopItem->m_topType = 0;
                            int smallDepth = 0;
                            bool isValidTop = false;
                            TreeItem *curItem = pTopItem;
                            while (curItem)
                            {
                                ++smallDepth;
                                if (smallDepth > 5)
                                {
                                    pTopItem->m_topType = 1;
                                    break;
                                }
                                if (curItem->m_childItemMap.isEmpty())
                                {
                                    break;
                                }
                                if (curItem->m_childItemMap.size() > 1)
                                {
                                    pTopItem->m_topType = 1;
                                    break;
                                }

                                curItem = curItem->m_childItemMap.begin().value();
                            }
                        }

                        if (pTopItem->m_topType == 0)
                        {
                            continue;
                        }
                        
                        // 不能合并当前节点的顶层节点
                        TreeItem *pCurItemTopItem = this;
                        while (pCurItemTopItem)
                        {
                            if (pCurItemTopItem == pTopItem)
                            {
                                break;
                            }
                            pCurItemTopItem = pCurItemTopItem->parentItem;
                        }

                        if (pCurItemTopItem != pTopItem)
                        {
                            pValidItem->m_memoryLocalSize += pTopItem->m_memoryLocalSize;
                            pValidItem->m_memoryRealLocalSize += pTopItem->m_memoryRealLocalSize;
                            pValidItem->m_memoryCount += pTopItem->m_memoryCount;
                            pValidItem->m_msTime += pTopItem->m_msTime;
                            //pValidItem->m_branchNumber += pTopItem->m_branchNumber;
                            for (int i=0; i<pTopItem->childCount(); i++)
                            {
                                pValidItem->appendChild(pTopItem->child(i));
                            }
                            pTopItems->remove(pTopItem->m_functionName);
                        }
                    }
                }
            }

            pLastValidItem = pValidItem;
        }
    }

    for (auto i=m_childItemMap.begin(); i!=m_childItemMap.end(); i++)
    {
        TreeItem *pCurItem = i.value();
        pCurItem->mergeSameSmallChild(nDepth, mergeSameTop, pTopItems);
    }

    {
        m_childIItemList.clear();
        QMultiMap<__int64, TreeItem*> size2Function;
        for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
        {
            size2Function.insert(it.value()->m_memoryRealLocalSize, it.value());
        }
        for (auto it = size2Function.end() - 1; it!=size2Function.begin() - 1; it--)
        {
            m_childIItemList << it.value();
        }
    }

    if (parents)
    {
        parents->remove(this);
    }
    nDepth--;
}

void TreeItem::clearChildNotDelete()
{
    m_childIItemList.clear();
    m_childItemMap.clear();
}

int TreeItem::calculateMemeorySize()
{
    if (!m_childIItemList.isEmpty())
    {
        //m_memoryLocalSize = 0;
        //m_memoryRealLocalSize = 0;
        //m_memoryCount = 0;

        for (auto i=m_childItemMap.begin(); i!=m_childItemMap.end(); i++)
        {
            TreeItem *pCurItem = i.value();
            pCurItem->calculateMemeorySize();
            m_memoryLocalSize += pCurItem->m_memoryLocalSize;
            m_memoryRealLocalSize += pCurItem->m_memoryRealLocalSize;
            m_memoryCount += pCurItem->m_memoryCount;
            m_msTime += pCurItem->m_msTime;
            m_branchNumber += pCurItem->m_branchNumber;
        }

        m_childIItemList.clear();
        QMultiMap<__int64, TreeItem*> size2Function;
        for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
        {
            size2Function.insert(it.value()->m_memoryRealLocalSize, it.value());
        }
        for (auto it = size2Function.end() - 1; it!=size2Function.begin() - 1; it--)
        {
            m_childIItemList << it.value();
        }
    }

    return m_memoryLocalSize;
}

int TreeItem::getBottomItems(QList<TreeItem*> &bottomItems)
{
    if (!m_childIItemList.isEmpty())
    {
        for (auto i=m_childItemMap.begin(); i!=m_childItemMap.end(); i++)
        {
            TreeItem *pCurItem = i.value();
            pCurItem->getBottomItems(bottomItems);
        }
    }
    else
    {
        bottomItems << this;
    }

    return bottomItems.size();
}

#include <QSet>
int TreeItem::calculateFunctionAllMemory(QMap<QString, TreeItem*> &functionMemorySizeMap, QSet<QString> &functionNames)
{
    if (!functionNames.contains(m_functionName))
    {
        TreeItem *pItem = functionMemorySizeMap.value(m_functionName, nullptr);
        if (pItem == nullptr)
        {
            pItem = new TreeItem(nullptr);
            pItem->m_functionName = this->m_functionName;
            pItem->m_bIsAddress2FunctionName = true;
            pItem->m_memoryLocalSize = m_memoryLocalSize;
            pItem->m_memoryRealLocalSize = m_memoryRealLocalSize;
            pItem->m_memoryCount = m_memoryCount;
            pItem->m_msTime = m_msTime;
            pItem->m_branchNumber = m_branchNumber;
            functionMemorySizeMap.insert(m_functionName, pItem);
        }
        else
        {
            pItem->m_memoryLocalSize += m_memoryLocalSize;
            pItem->m_memoryRealLocalSize += m_memoryRealLocalSize;
            pItem->m_memoryCount += m_memoryCount;
            pItem->m_msTime += m_msTime;
            pItem->m_branchNumber += m_branchNumber;
        }
    }

    functionNames.insert(m_functionName);
    for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
    {
        it.value()->calculateFunctionAllMemory(functionMemorySizeMap, functionNames);
    }
    functionNames.remove(m_functionName);

    
    
    return 1;
}

//! [2]

//! [3]
TreeItem *TreeItem::child(int row)
{
    return m_childIItemList.value(row);
}
//! [3]

//! [4]
int TreeItem::childCount() const
{
    //TreeItem *item = (TreeItem*)this;
    //while (item->parent())
    //{
    //    item = item->parent();
    //    if (item == this)
    //    {
    //        return 0;
    //    }
    //}
    return m_childIItemList.count();
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return itemData.count();
}
//! [5]

//! [6]
QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}
//! [6]

//! [7]
TreeItem *TreeItem::parent()
{
    return parentItem;
}
//! [7]

//! [8]
int TreeItem::row() const
{
    if (parentItem)
        return parentItem->m_childIItemList.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

void TreeItem::setParent( TreeItem *pParent )
{
    parentItem = pParent;
}

bool qMapLessThanKey(const QVariant &key1, const QVariant &key2)
{
    if (key1.type() == QVariant::Double || key1.type() == QVariant::Int || key1.type() == QVariant::LongLong)
    {
        return key1.toDouble() < key2.toDouble();
    }
    else if (key1.type() == QVariant::String)
    {
        return key1.toString() < key2.toString();
    }
    else
    {
        return false;
    }
}

void TreeItem::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder*/ )
{
    {
        m_childIItemList.clear();
        QMultiMap<QVariant, TreeItem*> size2Function;
        for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
        {
            size2Function.insert(it.value()->data(column), it.value());
        }

        if (order == Qt::AscendingOrder)
        {
            for (auto it = size2Function.begin(); it!=size2Function.end(); it++)
            {
                m_childIItemList << it.value();
            }
        }
        else
        {
            for (auto it = size2Function.end() - 1; it!=size2Function.begin() - 1; it--)
            {
                m_childIItemList << it.value();
            }
        }
    }

    for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
    {
        it.value()->sort(column, order);
    }
}

TreeItem * TreeItem::addAddress( CallStackData *data, bool bIsBottom2Top)
{
    //this->m_memoryCount += data->updateTimes;
    //this->m_memoryLocalSize = data->updateSize;

    TreeItem *curItem = this;

    if (!bIsBottom2Top)
    {
        bool isValid = true;
        for (int i = data->depth - 1; i >= 0; --i)
        {
            int curAddress = (int)data->backtrace[i];
            TreeItem *childItem = curItem->m_addressChild.value(curAddress, nullptr);
            if (childItem == nullptr)
            {
                childItem = new TreeItem(curItem);
                if (GlobalData::getInstance()->memoryMode == 0)
                {
                    childItem->m_functionName = Utils::getInfoFromAddress(curAddress);
                    childItem->m_bIsAddress2FunctionName = true;
                }
                else
                {
                    childItem->m_functionName = QString::number(curAddress, 16); //Utils::getInfoFromAddress(curAddress);
                }
                
                if (childItem->m_functionName.contains("GLDCustomWidgetStyle::setIsCustomSize") ||
                    childItem->m_functionName.contains("GLDCustomWidgetStyle::GLDCustomWidgetStyle"))
                {
                    isValid = false;
                }
                if (i == data->depth - 1 && 
                    (childItem->m_functionName.contains("operator new") || 
                    childItem->m_functionName.contains("MyHookMalloc")))
                {
                    isValid = false;
                }

                //if (childItem->m_functionName.contains("-"))
                {
                    curItem->appendChild(childItem);
                }
            }

            if (i == 0 && isValid)
            {
                childItem->m_address = curAddress;
                childItem->m_memoryCount += data->updateTimes - data->times;
                childItem->m_memoryLocalSize = data->updateSize - data->size;
                if (data->updateTimes == data->times)
                {
                    childItem->m_memoryRealLocalSize = 0;
                }
                childItem->m_memoryRealLocalSize = data->updateTimes == data->times ? 0 : (data->updateTimes - data->times) * Utils::getRealMemorySize((data->updateSize - data->size)/(data->updateTimes - data->times));
                childItem->m_msTime = data->updateMsTime;
                childItem->m_branchNumber = 1;
            }

            //if (childItem->m_functionName.contains("-"))
            {
                curItem = childItem;
            }
        }
    }
    else
    {
        bool isValid = true;
        for (int i = 0; i < data->depth; ++i)
        {
            int curAddress = (int)data->backtrace[i];
            TreeItem *childItem = curItem->m_addressChild.value(curAddress, nullptr);
            if (childItem == nullptr)
            {
                childItem = new TreeItem(curItem);
                if (GlobalData::getInstance()->memoryMode == 0)
                {
                    childItem->m_functionName = Utils::getInfoFromAddress(curAddress);
                    childItem->m_bIsAddress2FunctionName = true;
                }
                else
                {
                    childItem->m_functionName = QString::number(curAddress, 16); //Utils::getInfoFromAddress(curAddress);
                }

                if (childItem->m_functionName.contains("GLDCustomWidgetStyle::setIsCustomSize") ||
                    childItem->m_functionName.contains("GLDCustomWidgetStyle::GLDCustomWidgetStyle"))
                {
                    isValid = false;
                }
                if (i == data->depth - 1 && 
                    (childItem->m_functionName.contains("operator new") || 
                    childItem->m_functionName.contains("MyHookMalloc")))
                {
                    isValid = false;
                }

                //if (childItem->m_functionName.contains("-"))
                {
                    curItem->appendChild(childItem);
                }
            }

            if (i == data->depth - 1 && isValid)
            {
                //if (!childItem->m_functionName.contains("-"))
                //{
                //    if (curItem == this)
                //    {
                //        break;
                //    }
                //    childItem = curItem;
                //}
                childItem->m_address = curAddress;
                childItem->m_memoryCount += data->updateTimes - data->times;
                childItem->m_memoryLocalSize = data->updateSize - data->size;
                if (data->updateTimes == data->times)
                {
                    childItem->m_memoryRealLocalSize = 0;
                }
                childItem->m_memoryRealLocalSize = data->updateTimes == data->times ? 0 : (data->updateTimes - data->times) * Utils::getRealMemorySize((data->updateSize - data->size)/(data->updateTimes - data->times));
                childItem->m_msTime = data->updateMsTime;
                childItem->m_branchNumber = 1;
            }

            //if (childItem->m_functionName.contains("-"))
            {
                curItem = childItem;
            }
        }
    }
    

    return curItem;
}

void TreeItem::getChildNameAndMerge()
{
    if (m_bChildIsAddress2FunctionName)
    {
        return;
    }

    if (!m_childIItemList.isEmpty())
    {
        // 预先读取函数地址并批量处理，以调高性能
        QSet<__int64> addressList;
        for (auto i=m_childItemMap.begin(); i!=m_childItemMap.end(); i++)
        {
            TreeItem *pCurItem = i.value();
            if (!pCurItem->m_bIsAddress2FunctionName)
            {
                addressList << (__int64)pCurItem->m_functionName.toLongLong(nullptr, 16);
            }
        }
        Utils::readFuncInfoFromAddressList(addressList);

        QMultiMap<QString, TreeItem*> smallChildItemMap = m_childItemMap;
        m_childIItemList.clear();
        m_childItemMap.clear();
        TreeItem *pLastValidItem = nullptr;
        for (auto i=smallChildItemMap.begin(); i!=smallChildItemMap.end(); i++)
        {
            TreeItem *pCurItem = i.value();
            pCurItem->m_functionName = pCurItem->getFunctionName();
            TreeItem *pValidItem = m_childItemMap.value(pCurItem->m_functionName, nullptr);
            if (pValidItem == nullptr)
            {
                pValidItem = pCurItem;
                this->appendChild(pValidItem);
            }
            else
            {
                pValidItem->m_memoryLocalSize += pCurItem->m_memoryLocalSize;
                pValidItem->m_memoryRealLocalSize += pCurItem->m_memoryRealLocalSize;
                pValidItem->m_memoryCount += pCurItem->m_memoryCount;
                pValidItem->m_msTime += pCurItem->m_msTime;
                pValidItem->m_branchNumber += pCurItem->m_branchNumber;
                for (int i=0; i<pCurItem->childCount(); i++)
                {
                    pValidItem->appendChild(pCurItem->child(i));
                }
                pCurItem->clearChildNotDelete();
                //delete pCurItem;
                //pCurItem = nullptr;
            }

            pLastValidItem = pValidItem;
        }
    }

    {
        m_childIItemList.clear();
        QMultiMap<__int64, TreeItem*> size2Function;
        for (auto it = m_childItemMap.begin(); it!=m_childItemMap.end(); it++)
        {
            size2Function.insert(it.value()->m_memoryRealLocalSize, it.value());
        }
        for (auto it = size2Function.end() - 1; it!=size2Function.begin() - 1; it--)
        {
            m_childIItemList << it.value();
        }
    }

    //m_functionName = Utils::getInfoFromAddress(m_functionName.toInt(nullptr, 16));
    //if (!m_childIItemList.isEmpty())
    {
        m_bChildIsAddress2FunctionName = true;
    }
}

QString TreeItem::getFunctionName()
{
    if (!m_bIsAddress2FunctionName && m_functionName != "All" && m_functionName != "")
    {
        m_functionName = Utils::getInfoFromAddress(m_functionName.toInt(nullptr, 16));
        m_bIsAddress2FunctionName = true;
    }
    return m_functionName;
}

//! [8]
