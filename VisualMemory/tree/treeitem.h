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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>

class CallStackData;


//! [0]
class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
    explicit TreeItem(TreeItem *parent = 0);
    ~TreeItem();
    void clear();

    void appendChild(TreeItem *child);
    void appendChild(TreeItem *child, int memorySize);
    //void appendChild(QString functionName, int memorySize);

    void mergeSameSmallChild(int &nDepth, bool mergeSameTop = false, QMap<QString, TreeItem*> *pTopItems = nullptr, QSet<TreeItem*> *parents = nullptr);
    void clearChildNotDelete();
    int calculateMemeorySize();
    int calculateFunctionAllMemory(QMap<QString, TreeItem*> &functionMemorySizeMap, QSet<QString> &functionNames);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();
    void setParent(TreeItem *pParent);

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    // for 调用地址堆栈
    TreeItem *addAddress(CallStackData *data, bool bIsBottom2Top);

    void getChildNameAndMerge();
    QString getFunctionName();

    int getBottomItems(QList<TreeItem*> &bottomItems);

private:
    QList<TreeItem*> m_childIItemList;
    QList<QVariant> itemData;
    TreeItem *parentItem;

public: //暂时public
    QString m_functionName;
    bool m_bIsAddress2FunctionName;
    bool m_bChildIsAddress2FunctionName;
    QString m_modelName;
    qint64 m_memoryRealLocalSize;
    qint64 m_memoryLocalSize;
    int m_memoryCount;
    int m_branchNumber; // 调用分支数（用于记录有多少条函数调用分支，可以做为内存泄漏的函数调用分支数量）
    int m_msTime;
    int m_topType; // -1 - unknow; 0 - no; 1 - yes;
    QMultiMap<QString, TreeItem*> m_childItemMap;

    // for 调用地址堆栈
    int m_address;
    QMap<int, TreeItem*> m_addressChild;
};
//! [0]

#endif // TREEITEM_H
