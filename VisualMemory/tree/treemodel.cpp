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
    treemodel.cpp

    Provides a simple tree model to show how to create and use hierarchical
    models.
*/

#include "treeitem.h"
#include "treemodel.h"
#include "Utils.h"

#include <QStringList>

#define Function_Name 0
#define Function_Local_Size 1
#define Function_Real_Local_Size 2
#define Function_MemoryCount 3
#define Function_AverageValue 4
#define Column_MSTime 100
#define Model_Name 101
#define Function_BranchCount 5
#define Column_Count 6


//! [0]
TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    //rootData << QString::fromLocal8Bit("函数名称") << QString::fromLocal8Bit("当前调用申请内存") << QString::fromLocal8Bit("本函数总申请内存") << QString::fromLocal8Bit("所属模块");
    m_rootItem = new TreeItem(nullptr);
    m_rootShowItem = new TreeItem(m_rootItem);
    m_rootShowItem->m_functionName = "All";

    m_rootItem->appendChild(m_rootShowItem);
    
    //setupModelData(data.split(QString("\n")), m_rootItem);
}
//! [0]

//! [1]
TreeModel::~TreeModel()
{
    delete m_rootItem;
}
//! [1]

//! [2]
int TreeModel::columnCount(const QModelIndex &parent) const
{
    return Column_Count;
    //if (parent.isValid())
    //    return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    //else
    //    return m_rootItem->columnCount();
}
//! [2]

//! [3]
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role!=Qt::EditRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr)
    {
        return QVariant();
    }

    QStringList names;
    switch (index.column())
    {
    case Function_Name:
        //if (!item->m_bIsAddress2FunctionName)
        {
            item->getChildNameAndMerge();
        }
        return item->getFunctionName();
    case Function_Local_Size:
        if (abs(item->m_memoryLocalSize) < 1024)
        {
            return QString("%1 B").arg(item->m_memoryLocalSize);
        }
        else
        {
            return QString("%1(%2 B)").arg(item->m_memoryLocalSize/1024).arg(item->m_memoryLocalSize);
        }
    case Function_Real_Local_Size:
        if (abs(item->m_memoryLocalSize) < 1024)
        {
            return QString("%1 B").arg(item->m_memoryRealLocalSize);
        }
        else
        {
            return QString("%1(%2 B)").arg(item->m_memoryRealLocalSize/1024).arg(item->m_memoryRealLocalSize);
        }
    case Function_MemoryCount:
        return item->m_memoryCount;
        break;
    case Function_AverageValue:
        if (item->m_memoryCount != 0)
            return item->m_memoryLocalSize/item->m_memoryCount;
        break;
    case Column_MSTime:
        return item->m_msTime;
    case Model_Name:
        names = item->m_functionName.split("!");
        if (names.size() != 2)
        {
            return "";
        }
        else
        {
            return names[0];
        }
    case Function_BranchCount:
        return item->m_branchNumber;
    default:
        return "";
    }

    return QVariant();
}
//! [3]

//! [4]
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled 
        | Qt::ItemIsSelectable 
        | Qt::ItemIsEditable
        | Qt::ItemIsDragEnabled
        | Qt::ItemIsDropEnabled;
}
//! [4]

//! [5]
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    //    return m_rootItem->data(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Function_Name:
            return QString::fromLocal8Bit("函数名称");
            break;
        case Function_Local_Size:
            return QString::fromLocal8Bit("当前调用总内存申请量（K）");
            break;
        case Function_Real_Local_Size:
            return QString::fromLocal8Bit("当前调用预估实际内存量（K）");
            break;
        case Function_MemoryCount:
            return QString::fromLocal8Bit("申请次数");
            break;
        case Function_AverageValue:
            return QString::fromLocal8Bit("每次申请量（B）");
            break;
        case Function_BranchCount:
            return QString::fromLocal8Bit("代码分支数");
            break;
        case Model_Name:
            return QString::fromLocal8Bit("所属模块");
            break;
        default:
            break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}
//! [5]

//! [6]
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem || parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
//! [7]

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}
//! [8]

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    //QList<TreeItem*> parents;
    //QList<int> indentations;
    //parents << parent;
    //indentations << 0;

    //int number = 0;

    //while (number < lines.count()) {
    //    int position = 0;
    //    while (position < lines[number].length()) {
    //        if (lines[number].mid(position, 1) != " ")
    //            break;
    //        position++;
    //    }

    //    QString lineData = lines[number].mid(position).trimmed();

    //    if (!lineData.isEmpty()) {
    //        // Read the column data from the rest of the line.
    //        QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
    //        QList<QVariant> columnData;
    //        for (int column = 0; column < columnStrings.count(); ++column)
    //            columnData << columnStrings[column];

    //        if (position > indentations.last()) {
    //            // The last child of the current parent is now the new parent
    //            // unless the current parent has no children.

    //            if (parents.last()->childCount() > 0) {
    //                parents << parents.last()->child(parents.last()->childCount()-1);
    //                indentations << position;
    //            }
    //        } else {
    //            while (position < indentations.last() && parents.count() > 0) {
    //                parents.pop_back();
    //                indentations.pop_back();
    //            }
    //        }

    //        // Append a new item to the current parent's list of children.
    //        parents.last()->appendChild(new TreeItem(columnData, parents.last()));
    //    }

    //    ++number;
    //}
}

TreeItem * TreeModel::rootItem()
{
    return m_rootItem;
}

TreeItem * TreeModel::rootShowItem()
{
    return m_rootShowItem;
}

void TreeModel::updateData()
{
    this->beginResetModel();
    this->endResetModel();
}

void TreeModel::mergeTopToChild()
{
    QMap<QString, TreeItem*> topItems;
    for (auto i=m_rootShowItem->m_childItemMap.begin(); i!=m_rootShowItem->m_childItemMap.end(); i++)
    {
        topItems.insert(i.key(), i.value());
    }

    int nDepth = 0;
    m_rootShowItem->mergeSameSmallChild(nDepth, true, &topItems);
    QMultiMap<QString, TreeItem*> childItemMap = m_rootShowItem->m_childItemMap;
    m_rootShowItem->clearChildNotDelete();
    for (auto i=childItemMap.begin(); i!=childItemMap.end(); i++)
    {
        if (!topItems.contains(i.key()))
        {
            continue;
        }
        m_rootShowItem->appendChild(i.value());
    }
}

void TreeModel::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder*/ )
{
    m_rootItem->sort(column, order);
    updateData();
}
