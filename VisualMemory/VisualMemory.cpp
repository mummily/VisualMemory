#include "VisualMemory.h"
#include "ui_VisualMemory.h"
#include "tree/treemodel.h"
#include "tree/treeitem.h"
#include "Utils.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMap>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QTableWidget>
#include <QToolBar>
#include <QInputDialog>
#include <QSettings>
#include <QTabWidget>
#include <QTreeView>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QThread>
#include <QProgressDialog>
#include <QComboBox>

#include <windows.h>
#include <tlhelp32.h>
#include <assert.h>
#include <iostream>
#include<stdlib.h>

// windbg
//#include "windbg/DebugSession.h"
//#include "windbg/Command.h"
//#include "windbg/CmdHandlers.h"

// Detours
#include "DetoursTest.h"
#include "highcompare/CallStackDataController.h"

using namespace std;

#define ASSERT assert
#define _T

static int s_curFileIndex = 3;

//QTimer p_VisualMemoryInstanceTimer;
//VisualMemory* p_pTempInstance = nullptr;
VisualMemory* VisualMemory::m_pInstance = nullptr;

QThread *g_pMainThread = nullptr;
class MyThread : public QThread
{
public:
    MyThread(QObject *parent = nullptr) : QThread(parent)
    {

    }

    virtual void run() override
    {
        ::Sleep(3000);
        if (VisualMemory::m_pInstance == nullptr)
        {
            MainWindowStarter *pStarter = new MainWindowStarter();
            pStarter->moveToThread(qApp->thread());
            QObject::connect(pStarter, SIGNAL(sigShowMainWindow()), pStarter, SLOT(slotShowMainWindow()));
            pStarter->showMainWindow();
        }
    }
};

void MainWindowStarter::showMainWindow()
{
    emit sigShowMainWindow();
}

DetoursTest g_DetoursTest;
void MainWindowStarter::slotShowMainWindow()
{
    VisualMemory::m_pInstance = new VisualMemory();
    VisualMemory::m_pInstance->moveToThread(qApp->thread());
    VisualMemory::m_pInstance->show();
    g_DetoursTest.m_iShowIntevalTimes = 100000;
    //g_DetoursTest.Hook();
}

VisualMemory* VisualMemoryInstanceInit()
{
    if (VisualMemory::m_pInstance)
    {
        return VisualMemory::m_pInstance;
    }
    else
    {
        //system("pause");
        qDebug() << "VisualMemoryInstanceInit 1";
        //system("pause");
        //VisualMemory::m_pInstance = new VisualMemory();
        qDebug() << "VisualMemoryInstanceInit 2";
        //system("pause");
        g_pMainThread = QThread::currentThread();
        //MyThread *pThread = new MyThread();
        //pThread->start();
        return nullptr;
    }
}

VisualMemory* g_tempInstance = VisualMemoryInstanceInit();

static VMEventReceiver *g_VMEventReceiver = nullptr;

VisualMemory::VisualMemory(QWidget *parent)
    : QMainWindow(parent), m_strAppName(""), m_hasReadItemCount(0), m_validFileTotal(0), ui(new Ui::VisualMemoryClass())
{
    g_VMEventReceiver = new VMEventReceiver(this);
    connect(g_VMEventReceiver, SIGNAL(sigStart()), SLOT(slotStartStatistics()), Qt::BlockingQueuedConnection);
    connect(g_VMEventReceiver, SIGNAL(sigStop()), SLOT(slotStopStatistics()), Qt::BlockingQueuedConnection);
    g_VMEventReceiver->start();
    m_pInstance = this;
    ui->setupUi(this);

    m_pConfigFile = new QSettings(qApp->applicationDirPath() + "/Config.ini", QSettings::IniFormat);
    m_strAppFilePath = m_pConfigFile->value("Default/AppFilePath").toString();

    m_pAttachAppAct = new QAction(QString::fromLocal8Bit("启动程序"), this);
    m_pAttachAppAct->setVisible(true);
    connect(m_pAttachAppAct, SIGNAL(triggered()), this, SLOT(slotAttachApp()));

    m_pStartStaAct = new QAction(QString::fromLocal8Bit("开始"), this);
    m_pStartStaAct->setEnabled(true);
    connect(m_pStartStaAct, SIGNAL(triggered()), this, SLOT(slotStartStatistics()));

    m_pStopStaAct = new QAction(QString::fromLocal8Bit("结束"), this);
    m_pStopStaAct->setEnabled(true);
    connect(m_pStopStaAct, SIGNAL(triggered()), this, SLOT(slotStopStatistics()));

    QAction *saveAct = new QAction(QString::fromLocal8Bit("保存统计结果"), this);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(slotSaveResult()));

    QAction *openAct = new QAction(QString::fromLocal8Bit("打开统计结果"), this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(slotOpenResult()));

    QAction *batchOpenAct = new QAction(QString::fromLocal8Bit("批量打开统计结果"), this);
    connect(batchOpenAct, SIGNAL(triggered()), this, SLOT(slotBatchOpenResult()));

    QComboBox *statModeComboBox = new QComboBox(this);
    statModeComboBox->addItem(QString::fromLocal8Bit("精准内存泄漏模式"));
    statModeComboBox->addItem(QString::fromLocal8Bit("常规内存泄漏模式"));
    statModeComboBox->addItem(QString::fromLocal8Bit("增量内存统计模式"));
    connect(statModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotStaModeChanged(int)));
    this->slotStaModeChanged(0);

    m_pPauseAppAct = new QAction(QString::fromLocal8Bit("暂停应用"), this);
    m_pPauseAppAct->setEnabled(true);
    m_pPauseAppAct->setCheckable(true);
    m_pPauseAppAct->setChecked(false);
    connect(m_pPauseAppAct, SIGNAL(triggered()), this, SLOT(slotPauseApp()));

    QAction *pAboutAct = new QAction(QString::fromLocal8Bit("关于软件"), this);
    connect(pAboutAct, SIGNAL(triggered()), this, SLOT(slotAbout()));

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->addWidget(statModeComboBox);
    toolBar->addAction(m_pAttachAppAct);
    toolBar->addAction(m_pStartStaAct);
    toolBar->addAction(m_pStopStaAct);
    toolBar->addAction(m_pPauseAppAct);
    toolBar->addAction(saveAct);
    toolBar->addAction(openAct);
    toolBar->addAction(batchOpenAct);
    toolBar->addAction(pAboutAct);
    this->addToolBar(toolBar);

    //m_tableWidget = new QTableWidget(this);
    //m_tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    //QStringList names = QStringList() 
    //    << QString::fromLocal8Bit("函数名")
    //    << QString::fromLocal8Bit("总内存申请量（K）")
    //    << QString::fromLocal8Bit("预估实际内存量（K）")
    //    << QString::fromLocal8Bit("申请次数")
    //    << QString::fromLocal8Bit("每次申请量（B）")
    //    << QString::fromLocal8Bit("参考耗时")
    //    << QString::fromLocal8Bit("所在模块");
    //m_tableWidget->setColumnCount(names.size());
    //m_tableWidget->setHorizontalHeaderLabels(names);
    //m_tableWidget->setColumnWidth(0, 300);
    //m_tableWidget->setColumnWidth(1, 200);
    //m_tableWidget->setColumnWidth(2, 200);
    //m_tableWidget->setColumnWidth(3, 100);
    //m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    //m_tableWidget->setAlternatingRowColors(true);
    //m_tableWidget->setSortingEnabled(true);


    QTabWidget *tabWidget = new QTabWidget(this);
    //tabWidget->addTab(m_tableWidget, QString::fromLocal8Bit("函数总内存列表"));

    // 函数调用树（自上而下）
    {
        QFile file(qApp->applicationDirPath() + "/default.txt");
        file.open(QIODevice::ReadOnly);
        m_model_Top2Bottom = new TreeModel(file.readAll());
        file.close();

        QTreeView *view = new QTreeView(this);
        view->setEditTriggers(QAbstractItemView::DoubleClicked);
        view->header()->setStretchLastSection(true);
        view->setModel(m_model_Top2Bottom);
        view->setWindowTitle(QString::fromLocal8Bit("函数调用关系树（自上而下）"));
        tabWidget->addTab(view, QString::fromLocal8Bit("函数调用关系树（自上而下）"));
        view->setColumnWidth(0, 300);
        view->setColumnWidth(1, 200);
        view->setColumnWidth(2, 200);
        view->setColumnWidth(3, 100);
        view->setAlternatingRowColors(true);
        //view->setSortingEnabled(true);
    }

    // 函数调用树（自下而上）
    {
        QFile file(qApp->applicationDirPath() + "/default.txt");
        file.open(QIODevice::ReadOnly);
        m_model_Bottom2Top = new TreeModel(file.readAll());
        file.close();

        QTreeView *view = new QTreeView(this);
        view->setEditTriggers(QAbstractItemView::DoubleClicked);
        view->header()->setStretchLastSection(true);
        view->setModel(m_model_Bottom2Top);
        view->setWindowTitle(QString::fromLocal8Bit("函数调用关系树（自下而上）"));
        tabWidget->addTab(view, QString::fromLocal8Bit("函数调用关系树（自下而上）"));
        view->setColumnWidth(0, 300);
        view->setColumnWidth(1, 200);
        view->setColumnWidth(2, 200);
        view->setColumnWidth(3, 100);
        view->setAlternatingRowColors(true);
        //view->setSortingEnabled(true);
    }

    m_model_StartMemory = new TreeModel("");
    m_model_EndMemory = new TreeModel("");
    m_model_DiffMemory = new TreeModel("");
    

    this->setCentralWidget(tabWidget);

    this->setWindowTitle(QString::fromLocal8Bit("VisualMemory 2018"));
    this->resize(800, 600);

    //g_DetoursTest.Hook();

    //Utils::getInfoFromAddress(0);
}

VisualMemory::~VisualMemory()
{
    if (m_pConfigFile)
    {
        delete m_pConfigFile;
        m_pConfigFile = nullptr;
    }
}

void VisualMemory::slotAttachApp()
{
    //Utils::dumpStack();
    QString filePath = QFileDialog::getOpenFileName(this, "SelectApp", m_strAppFilePath);
    if (filePath.isEmpty())
    {
        return;
    }

    m_strAppFilePath = QDir::toNativeSeparators(filePath);
    m_strAppName = m_strAppFilePath.mid(m_strAppFilePath.lastIndexOf(QDir::separator()) + 1);
    Utils::s_openAppName = m_strAppName;
    QDir dir(m_strAppFilePath);
    dir.cdUp();
    QDir::setCurrent(dir.path());
    g_DetoursTest.injectDllDetours(filePath);
    Utils::AddProcessDirToPath(m_strAppName);
    
    m_pConfigFile->setValue("Default/AppFilePath", m_strAppFilePath);
    //this->close();

    CallStackDataController::instance()->m_rootItem = m_model_Top2Bottom->rootShowItem();
    CallStackDataController::instance()->m_rootItemBottom2Top = m_model_Bottom2Top->rootShowItem();

    //slotStartStatistics();

    GlobalData::getInstance()->curStep = 1;
}

#define GLOBAL_STATICS_FINISH_EVENT  ("Local\\GLOBAL_STATICS_FINISH_EVENT") 
#define  MAX_WAIT_TIME (10*1000*1000)
void VisualMemory::slotStartStatistics()
{
    qDebug() << Q_FUNC_INFO;
    QDir dir(m_strAppFilePath);
    dir.cdUp();
    QString fileName = dir.path() + "/Dump_Result1.txt";
    m_logFile.setFileName(fileName);
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    m_logFile.write("VisualMemory 1.0\n\n");
    
    this->writeLog("VisualMemory::slotStartStatistics start");


    // 自主统计模式
    {
        DetoursTest::m_bIsStating = true;
        HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_FINISH_EVENT);
        ::SetEvent(hEvent);
        while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
        {
            // 等待
            ::Sleep(100);
        }
        if (WAIT_OBJECT_0 == WaitForSingleObject(EventController::getInstance()->hCatheReceivedEvent, MAX_WAIT_TIME))
        {
            //::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
        }
        //VisualMemory::slotPauseApp();
        CallStackDataController::instance()->m_rootItem = m_model_Top2Bottom->rootShowItem();
        CallStackDataController::instance()->m_rootItemBottom2Top = m_model_Bottom2Top->rootShowItem();
        qDebug() << "qingh-a, CallStackDataController::instance()->start()";
        GlobalData::getInstance()->isInMemoryLeakRange = true;
        CallStackDataController::instance()->start();
        DetoursTest::m_bIsStating = false;
        //::SetEvent(hEvent);
        //while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
        //{
        //    // 等待
        //    ::Sleep(100);
        //}
        // 工具继续处理下一阶段数据
        ::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
    }

    //{
    //    HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_PROCESS_EXIT_EVENT);
    //    if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, MAX_WAIT_TIME))
    //    {
    //        ::ResetEvent(hEvent);
    //    }

    //    // 调用结束操作
    //    slotStopStatistics();
    //    if (GlobalData::getInstance()->memoryMode == 0)
    //    {
    //        qApp->processEvents();
    //        ::SetEvent(hEvent);
    //    }
    //}

    GlobalData::getInstance()->curStep = 2;
}


void VisualMemory::slotStopStatistics()
{
    qDebug() << Q_FUNC_INFO;
    this->writeLog("VisualMemory::slotStopStatistics start");

    if (GlobalData::getInstance()->isInMemoryLeakRange == true && GlobalData::getInstance()->memoryMode == 0)
    {
        DetoursTest::m_bIsStating = true;
        HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_FINISH_EVENT);
        ::SetEvent(hEvent);
        while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
        {
            // 等待
            ::Sleep(100);
        }
        if (WAIT_OBJECT_0 == WaitForSingleObject(EventController::getInstance()->hCatheReceivedEvent, MAX_WAIT_TIME))
        {
            //::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
        }

        GlobalData::getInstance()->isInMemoryLeakRange = false;

        DetoursTest::m_bIsStating = false;
        //::SetEvent(hEvent);
        //while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
        //{
        //    // 等待
        //    ::Sleep(100);
        //}
        CallStackDataController::instance()->clearCurrentMemorySize();

        GlobalData::getInstance()->curStep = 3;

        // 工具继续处理下一阶段数据
        ::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);

        // 等待程序结束
        {
            this->writeLog("Wait for Exit Event.");
            QMessageBox::information(this, QString::fromLocal8Bit("提示信息"), QString::fromLocal8Bit("点击确定后，关闭程序，等待工具统计完毕。\r\n（注：内存泄漏模式下需关闭程序才会进行统计，程序需正常关闭，而不是被强杀）"));
            HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_PROCESS_EXIT_EVENT);
            if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, MAX_WAIT_TIME))
            {
                //::ResetEvent(hEvent); // 处理完再重置
            }

            if (WAIT_OBJECT_0 == WaitForSingleObject(EventController::getInstance()->hCatheReceivedEvent, MAX_WAIT_TIME))
            {
                //::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
            }

            this->writeLog("Get Exit Event.");

            // 调用结束操作
            slotStopStatistics();

            // 工具继续处理下一阶段数据
            ::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);

            if (GlobalData::getInstance()->memoryMode == 0)
            {
                qApp->processEvents();
                ::ResetEvent(hEvent);
            }
        }

        if (GlobalData::getInstance()->isAutoMemoryLeakMode)
        {
            //qApp->exit(0);
        }

        GlobalData::getInstance()->curStep = 4;

        return;
    }


    // 自主统计模式
    {
        DetoursTest::m_bIsStating = true;
        HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_FINISH_EVENT);
        if (GlobalData::getInstance()->memoryMode != 0)
        {
            ::SetEvent(hEvent);
            while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
            {
                // 等待
                ::Sleep(100);
            }
            if (WAIT_OBJECT_0 == WaitForSingleObject(EventController::getInstance()->hCatheReceivedEvent, MAX_WAIT_TIME))
            {
                //::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
            }
        }

        QDir dir(m_strAppFilePath);
        dir.cdUp();
        QDir::setCurrent(dir.path());
        //if (GlobalData::getInstance()->memoryMode == 0)
        //{
        //    Utils::getInfoFromAddress(1);
        //    ::ResetEvent(EventController::getInstance()->hExitEvent);
        //}
        CallStackDataController::instance()->end();
        GlobalData::getInstance()->isInMemoryLeakRange = false;
        this->showStatisticsResult("CallStack_Result1_statistics.txt");
        //CallStackDataController::instance()->end();
        //this->showStatisticsResult("CallStack_Result1_statistics.txt");
        DetoursTest::m_bIsStating = false;
        if (GlobalData::getInstance()->memoryMode != 0)
        {
            //::SetEvent(hEvent);
            //while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100))
            //{
            //    // 等待
            //    ::Sleep(100);
            //}

            // 工具继续处理下一阶段数据
            ::ResetEvent(EventController::getInstance()->hCatheReceivedEvent);
        }

        if (!GlobalData::getInstance()->isAutoMemoryLeakMode)
        {
            if (GlobalData::getInstance()->memoryMode == 0)
            {
                //GlobalData::getInstance()->totalLeakMemoryTimes -= GlobalData::getInstance()->totalLeakMemoryTimes_BeforeHook;
                //GlobalData::getInstance()->totalLeakMemorySize -= GlobalData::getInstance()->totalLeakMemorySize_BeforeHook;
                int ratio = GlobalData::getInstance()->totalLeakMemorySize_Last > 1024*1024 ? 1024 : 1;
                QString info = QString::fromLocal8Bit("统计完毕。有效覆盖内存泄漏次数为 %1次/%2次=%3\%；有效覆盖内存泄漏量为 %4字节/%5字节=%6\%。").
                    arg(GlobalData::getInstance()->hookTotalLeakMemoryTimes).
                    arg(GlobalData::getInstance()->totalLeakMemoryTimes).
                    arg((double)GlobalData::getInstance()->hookTotalLeakMemoryTimes * 100 / GlobalData::getInstance()->totalLeakMemoryTimes, 0, 'g', 3).
                    arg(GlobalData::getInstance()->hookTotalLeakMemorySize).
                    arg(GlobalData::getInstance()->totalLeakMemorySize).
                    arg((double)GlobalData::getInstance()->hookTotalLeakMemorySize/ratio*100 / (GlobalData::getInstance()->totalLeakMemorySize/ratio), 0, 'g', 3);
                // 因为统计信息未包含hook加载前的内存，故这里不再提示内存覆盖率。
                //info = QString::fromLocal8Bit("统计完毕。");
                QMessageBox::information(this, QString::fromLocal8Bit("提示信息"), info);
            }
            else
            {
                GlobalData::getInstance()->relativeTotalLeakMemoryTimes = GlobalData::getInstance()->totalLeakMemoryTimes - GlobalData::getInstance()->totalLeakMemoryTimes_Last;
                GlobalData::getInstance()->relativeTotalLeakMemorySize = GlobalData::getInstance()->totalLeakMemorySize - GlobalData::getInstance()->totalLeakMemorySize_Last;
                int ratio = GlobalData::getInstance()->relativeTotalLeakMemorySize > 1024*1024 ? 1024 : 1;
                QString info = QString::fromLocal8Bit("统计完毕。有效覆盖内存申请次数为 %1次/%2次=%3\%；有效覆盖内存申请量为 %4字节/%5字节=%6\%。").
                    arg(GlobalData::getInstance()->hookTotalLeakMemoryTimes).
                    arg(GlobalData::getInstance()->relativeTotalLeakMemoryTimes).
                    arg((double)GlobalData::getInstance()->hookTotalLeakMemoryTimes * 100 / GlobalData::getInstance()->relativeTotalLeakMemoryTimes, 0, 'g', 3).
                    arg(GlobalData::getInstance()->hookTotalLeakMemorySize).
                    arg(GlobalData::getInstance()->relativeTotalLeakMemorySize).
                    arg((double)GlobalData::getInstance()->hookTotalLeakMemorySize/ratio*100 / (GlobalData::getInstance()->relativeTotalLeakMemorySize/ratio), 0, 'g', 3);
                QMessageBox::information(this, QString::fromLocal8Bit("提示信息"), info);
            }
        }
        
    }

    if (GlobalData::getInstance()->isAutoMemoryLeakMode)
    {

        QDir dir(m_strAppFilePath);
        dir.cdUp();
        QString fileName = dir.path() + "/Dump_Result1.txt";
        saveToFile(fileName);
    }

    GlobalData::getInstance()->curStep = 3;
}

//// 等待进程关闭是统计
//void VisualMemory::slotStopStatistics()
//{
//    HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_PROCESS_EXIT_EVENT);
//    if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, MAX_WAIT_TIME))
//    {
//        //::ResetEvent(hEvent); // 处理完再重置
//    }
//
//    if (GlobalData::getInstance()->memoryMode == 0)
//    {
//        qApp->processEvents();
//        //::ResetEvent(hEvent);
//    }
//
//
//    // 自主统计模式
//    {
//        DetoursTest::m_bIsStating = true;
//        HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_FINISH_EVENT);
//
//        QDir dir(m_strAppFilePath);
//        dir.cdUp();
//        QDir::setCurrent(dir.path());
//        CallStackDataController::instance()->end();
//        this->showStatisticsResult("CallStack_Result1_statistics.txt");
//        //CallStackDataController::instance()->end();
//        //this->showStatisticsResult("CallStack_Result1_statistics.txt");
//        DetoursTest::m_bIsStating = false;
//        //QMessageBox::information(this, QString::fromLocal8Bit("提示信息"), QString::fromLocal8Bit("分析完毕,请品阅。"));
//    }
//
//    if (GlobalData::getInstance()->isAutoMemoryLeakMode)
//    {
//
//        QDir dir(m_strAppFilePath);
//        dir.cdUp();
//        QString fileName = dir.path() + "/Dump_Result1.txt";
//        saveToFile(fileName);
//    }
//
//    GlobalData::getInstance()->curStep = 3;
//}

void VisualMemory::slotPauseApp()
{
    Utils::PauseResumeProcessByName(m_strAppName, !m_pPauseAppAct->isChecked());
}

void VisualMemory::showStatisticsResult(const QString &statisticsFile)
{
    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
    TreeItem *rootItem_Bottom2Top = m_model_Bottom2Top->rootShowItem();

    // 合并树形结构的重复项（自上而下）
    int nDepth = 0;
    rootItem_Top2Bottom->mergeSameSmallChild(nDepth);
    //m_model_Top2Bottom->mergeTopToChild();
    rootItem_Top2Bottom->calculateMemeorySize();
    m_model_Top2Bottom->updateData();
    // 合并树形结构的重复项（自下而上）
    nDepth = 0;
    rootItem_Bottom2Top->mergeSameSmallChild(nDepth);
    //m_model_Bottom2Top->mergeTopToChild();
    rootItem_Bottom2Top->calculateMemeorySize();
    m_model_Bottom2Top->updateData();

    QSet<QString> functions;
    rootItem_Top2Bottom->calculateFunctionAllMemory(m_functionMemorySizeMap, functions);
    // 排序输出
    QMultiMap<__int64, TreeItem*> size2Function;
    for (auto it = m_functionMemorySizeMap.begin(); it!=m_functionMemorySizeMap.end(); it++)
    {
        size2Function.insert(it.value()->m_memoryRealLocalSize, it.value());
    }

    // 显示并保存到文件
    QFile writeFile(statisticsFile);
    writeFile.open(QIODevice::WriteOnly | QIODevice::Text);
    writeFile.write(QString::fromLocal8Bit("函数名\t总内存申请量（K）\t预估实际内存量（K）\t申请次数\t每次申请量（B）\t所属模块\n").toLocal8Bit().data());
    //m_tableWidget->setRowCount(size2Function.size());
    int curRow = 0;
    for (auto it = m_functionMemorySizeMap.end() - 1; it!=m_functionMemorySizeMap.begin() - 1; it--)
    {
        QStringList names = it.value()->m_functionName.split("!");
        QString strLine, modelName, functionName;
        QString memorySize = QString::number(it.value()->m_memoryLocalSize/1024);
        QString realMemorySize = QString::number(it.value()->m_memoryRealLocalSize/1024);
        QString memoryCount = QString::number(it.value()->m_memoryCount);
        QString msTime = QString::number(it.value()->m_msTime);
        QString averageValue;
        if (it.value()->m_memoryCount != 0)
        {
            averageValue = QString::number(it.value()->m_memoryLocalSize/it.value()->m_memoryCount);
        }
        if (abs(it.value()->m_memoryLocalSize) < 1024)
        {
            memorySize = QString("%1 B").arg(it.value()->m_memoryLocalSize);
            realMemorySize = QString("%1 B").arg(it.value()->m_memoryRealLocalSize);
        }
        if (names.size() != 2)
        {
            modelName = "";
            functionName = it.value()->m_functionName;
            //strLine = QString("%1\t\t%2\n").arg(memorySize).arg(functionName);
        }
        else
        {
            modelName = names[0];
            functionName = names[1];
            //strLine = QString("%1\t%2\t%3\n").arg(memorySize).arg(names[0]).arg(names[1]);
        }

        strLine = QString("%1\t%2\t%3\t%4\t%5\t%6\n").arg(functionName).arg(memorySize).arg(realMemorySize).arg(memoryCount).arg(averageValue).arg(modelName);
        writeFile.write(strLine.toLocal8Bit().data());

        //m_tableWidget->setItem(curRow, 0, new QTableWidgetItem(functionName));
        //m_tableWidget->setItem(curRow, 1, new QTableWidgetItem(memorySize));
        //m_tableWidget->setItem(curRow, 2, new QTableWidgetItem(realMemorySize));
        //m_tableWidget->setItem(curRow, 3, new QTableWidgetItem(memoryCount));
        //m_tableWidget->setItem(curRow, 4, new QTableWidgetItem(averageValue));
        //m_tableWidget->setItem(curRow, 5, new QTableWidgetItem(msTime));
        //m_tableWidget->setItem(curRow, 6, new QTableWidgetItem(modelName));
        curRow++;
    }

    writeFile.close();
}

void VisualMemory::slotAbout()
{
    QMessageBox::information(this, "VisualMemory 2018", QString::fromLocal8Bit("一个快速、准确、清晰的内存分析工具。\r\n作者：秦国海、周毅、肖玉连、曹波。"));
}

// 常规方案
void VisualMemory::slotSaveResult()
{
    this->writeLog("VisualMemory::slotSaveResult start");

    QString fileName = QFileDialog::getSaveFileName(this, "SaveMemoryFile", m_pConfigFile->value("Default/LastOpenDir").toString());
    if (fileName.isEmpty())
    {
        return;
    }
    m_pConfigFile->setValue("Default/LastOpenDir", fileName);

    saveToFile(fileName);
}

void VisualMemory::saveToFile(const QString &fileName)
{
    this->writeLog("VisualMemory::saveToFile start");

    QFile writeFile(fileName);
    if (m_logFile.isOpen() && m_logFile.fileName().compare(fileName, Qt::CaseInsensitive) == 0)
    {
        m_logFile.close();
        writeFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    }
    else
    {
        writeFile.open(QIODevice::WriteOnly | QIODevice::Text);
        writeFile.write("VisualMemory 1.0\n");
    }

    writeFile.write("\n");

    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
    rootItem_Top2Bottom->m_bIsAddress2FunctionName = true;
    QList<TreeItem*> bottomItems;
    rootItem_Top2Bottom->getBottomItems(bottomItems);
    int validBranchCount = bottomItems.size();
    for (int i=0; i<bottomItems.size(); i++)
    {
        TreeItem *curItem = bottomItems[i];
        if (curItem->m_memoryLocalSize <= 0)
        {
            validBranchCount--;
            continue;
        }
        writeFile.write(QString::number(curItem->m_memoryLocalSize).toLocal8Bit().data());
        writeFile.write(",");
        writeFile.write(QString::number(curItem->m_memoryCount).toLocal8Bit().data());
        writeFile.write("\n");
        while (curItem)
        {
            QString funcName = curItem->getFunctionName();
            if (funcName == "All" || funcName == "")
            {
                curItem = curItem->parent();
                continue;
            }
            writeFile.write(curItem->getFunctionName().toLocal8Bit().data());
            writeFile.write("\n");
            curItem = curItem->parent();
        }
        writeFile.write("\n");
    }

    writeFile.write("\n");
    writeFile.write("All,");
    writeFile.write(QString::number(rootItem_Top2Bottom->m_memoryLocalSize).toLocal8Bit().data());
    writeFile.write(",");
    writeFile.write(QString::number(rootItem_Top2Bottom->m_memoryCount).toLocal8Bit().data());
    writeFile.write(",");
    writeFile.write(QString::number(validBranchCount).toLocal8Bit().data());

    writeFile.close();
}

// 优化方案
//void VisualMemory::slotSaveResult()
//{
//    QString fileName = qApp->applicationDirPath() + "/MemoryData.vm";
//    QFile writeFile(fileName);
//    writeFile.open(QIODevice::WriteOnly | QIODevice::Text);
//    writeFile.write("VisualMemory 1.0\n");
//
//    auto it = m_functionMemorySizeMap.begin();
//    for (; it != m_functionMemorySizeMap.end(); it++)
//    {
//        QString funcName = it.value()->getFunctionName();
//        writeFile.write(QString::number(Utils::getAddressFromFuncName(funcName)).toLocal8Bit().data());
//        writeFile.write(",");
//        writeFile.write(funcName.toLocal8Bit().data());
//        writeFile.write("\n");
//    }
//
//    writeFile.write("\n");
//
//    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
//    QList<TreeItem*> bottomItems;
//    rootItem_Top2Bottom->getBottomItems(bottomItems);
//    for (int i=0; i<bottomItems.size(); i++)
//    {
//        TreeItem *curItem = bottomItems[i];
//        writeFile.write(QString::number(curItem->m_memoryLocalSize).toLocal8Bit().data());
//        writeFile.write(",");
//        writeFile.write(QString::number(curItem->m_memoryCount).toLocal8Bit().data());
//        writeFile.write("\n");
//        while (curItem)
//        {
//            writeFile.write(QString::number(Utils::getAddressFromFuncName(curItem->getFunctionName())).toLocal8Bit().data());
//            writeFile.write("\n");
//            curItem = curItem->parent();
//        }
//        writeFile.write("\n");
//    }
//
//    writeFile.close();
//}

void VisualMemory::slotOpenResult()
{
    QString fileName = QFileDialog::getOpenFileName(this, "SelectMemoryFile", m_pConfigFile->value("Default/LastOpenDir").toString());
    if (fileName.isEmpty())
    {
        return;
    }
    m_pConfigFile->setValue("Default/LastOpenDir", fileName);

    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
    rootItem_Top2Bottom->clear();
    TreeItem *rootItem_Bottom2Top = m_model_Bottom2Top->rootShowItem();
    rootItem_Bottom2Top->clear();

    m_validFileTotal = 0;
    this->openOneFile(fileName);

    QString writeFileName = qApp->applicationDirPath() + "/VLD_AllFiles_statistics.txt";
    this->showStatisticsResult("CallStack_Result1_statistics.txt");

    if (m_validFileTotal == 0)
    {
        QMessageBox::information(this, QString::fromLocal8Bit(""),  QString::fromLocal8Bit("非法文件。"));
    }
    else
    {
        QMessageBox::information(this, QString::fromLocal8Bit(""),  QString::fromLocal8Bit("统计完毕！"));
    }
}

void VisualMemory::slotBatchOpenResult()
{
    QString readFolder;
    QString writeFileName = qApp->applicationDirPath() + "/VLD_AllFiles_statistics.txt";
    readFolder = QFileDialog::getExistingDirectory(this, "select folder",  m_pConfigFile->value("Default/LastOpenDir").toString());
    if (readFolder.isEmpty())
    {
        return;
    }
    m_pConfigFile->setValue("Default/LastOpenDir", readFolder);

    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
    rootItem_Top2Bottom->clear();
    TreeItem *rootItem_Bottom2Top = m_model_Bottom2Top->rootShowItem();
    rootItem_Bottom2Top->clear();

    QStringList fileList = Utils::getFileList(readFolder);
    QProgressDialog dlg(QString::fromLocal8Bit("正在统计内存结果文件"), QString::fromLocal8Bit("统计到此结束"),
        0, fileList.size(), this);
    connect(&dlg, SIGNAL(canceled()), &dlg, SLOT(hide()));
    dlg.show();
    m_hasReadItemCount = 0;
    m_validFileTotal = 0;
    QTime lastTime = QTime::currentTime();
    QTime curTime = QTime::currentTime();
    for (int i=0; i<fileList.size(); i++)
    {
        if (dlg.isHidden())
        {
            break;
        }
        QString fileName = fileList[i];
        curTime = QTime::currentTime();
        if (lastTime.msecsTo(curTime) > 200)
        {
            this->refreshUI();
            lastTime = curTime;
        }
        this->openOneFile(fileList[i]);
        if (m_hasReadItemCount > 1000000 || fileList.size() < 100 || i % (fileList.size() / 100) == 0)
        {
            m_hasReadItemCount = 0;

            dlg.setValue(i);
            dlg.setLabelText(QString::fromLocal8Bit("正在统计内存结果文件 %1/%2").arg(i).arg(fileList.size()));
            this->refreshUI();

            int nDepth = 0;
            rootItem_Top2Bottom->mergeSameSmallChild(nDepth);
            nDepth = 0;
            rootItem_Bottom2Top->mergeSameSmallChild(nDepth);
        }
    }
    showStatisticsResult(writeFileName);

    dlg.close();
    QMessageBox::information(this, QString::fromLocal8Bit(""),  QString::fromLocal8Bit("统计完毕！有效文件数%1/%2。").arg(m_validFileTotal).arg(fileList.size()));
}

void VisualMemory::openOneFile( const QString &fileName )
{
    //QString fileName = qApp->applicationDirPath() + "/Dump_Result1.txt";
    QFile readFile(fileName);
    readFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream oReadTxt(&readFile);
    QString strLine = oReadTxt.readLine();
    if (!strLine.startsWith("VisualMemory 1.0"))
    {
        return;
    }
    else
    {
        ++m_validFileTotal;
    }

    TreeItem *rootItem_Top2Bottom = m_model_Top2Bottom->rootShowItem();
    rootItem_Top2Bottom->m_bIsAddress2FunctionName = true;
    TreeItem *rootItem_Bottom2Top = m_model_Bottom2Top->rootShowItem();
    rootItem_Bottom2Top->m_bIsAddress2FunctionName = true;

    // 跳过空行
    oReadTxt.readLine();

    strLine = oReadTxt.readLine();
    while (!oReadTxt.atEnd())
    {
        if (strLine.startsWith("\\\\"))
        {
            strLine = oReadTxt.readLine();
            continue;
        }

        if (strLine.startsWith("All", Qt::CaseInsensitive))
        {
            strLine = oReadTxt.readLine();
            break;
        }

        if (strLine.isEmpty() || strLine == "0")
        {
            strLine = oReadTxt.readLine();
            continue;
        }

        QStringList values = strLine.split(",");
        if (values.size() < 2)
        {
            strLine = oReadTxt.readLine();
            continue;
        }
        int memorySize = values[0].toInt();
        int memoryTimes = values[1].toInt();

        TreeItem *pLastItem_Top2Bottom = nullptr;
        TreeItem *pLastItem_Bottom2Top = nullptr;
        TreeItem *pTopItem_Top2Bottom = nullptr;
        TreeItem *pTopItem_Bottom2Top = nullptr;

        strLine = oReadTxt.readLine();
        while (!strLine.isEmpty())
        {
            bool ok = false;
            strLine.toInt(&ok, 16);
            if (ok == true)
            {
                strLine = oReadTxt.readLine();
                continue;
            }

            ++m_hasReadItemCount;

            // 构建树形结构（自上而下）
            {
                TreeItem *pCurItem = new TreeItem(nullptr);
                pCurItem->m_functionName = strLine;
                pCurItem->m_bIsAddress2FunctionName = true;
                pCurItem->m_memoryLocalSize = memorySize;
                pCurItem->m_memoryRealLocalSize = memorySize;
                pCurItem->m_memoryCount = memoryTimes;
                pCurItem->m_branchNumber = 1;
                if (pLastItem_Top2Bottom)
                {
                    pCurItem->m_memoryLocalSize = 0;
                    pCurItem->m_memoryRealLocalSize = 0;
                    pCurItem->m_memoryCount = 0;
                    pCurItem->m_branchNumber = 0;
                    pCurItem->appendChild(pLastItem_Top2Bottom);
                }
                pLastItem_Top2Bottom = pCurItem;
            }
            // 构建树形结构（自下而上）
            {
                TreeItem *pCurItem = new TreeItem(nullptr);
                pCurItem->m_functionName = strLine;
                pCurItem->m_bIsAddress2FunctionName = true;
                pCurItem->m_memoryLocalSize = memorySize;
                pCurItem->m_memoryRealLocalSize = memorySize;
                pCurItem->m_memoryCount = memoryTimes;
                pCurItem->m_branchNumber = 1;
                if (pLastItem_Bottom2Top)
                {
                    pLastItem_Bottom2Top->m_memoryLocalSize = 0;
                    pLastItem_Bottom2Top->m_memoryRealLocalSize = 0;
                    pLastItem_Bottom2Top->m_memoryCount = 0;
                    pLastItem_Bottom2Top->m_branchNumber = 0;
                    pLastItem_Bottom2Top->appendChild(pCurItem);
                }
                else
                {
                    //rootItem_Bottom2Top->appendChild(pCurItem);
                    pTopItem_Bottom2Top = pCurItem;
                }
                pLastItem_Bottom2Top = pCurItem;
            }

            strLine = oReadTxt.readLine();
            if (strLine.isEmpty() || strLine == "0")
            {
                if (pLastItem_Top2Bottom)
                {
                    rootItem_Top2Bottom->appendChild(pLastItem_Top2Bottom);
                }

                if (pTopItem_Bottom2Top)
                {
                    rootItem_Bottom2Top->appendChild(pTopItem_Bottom2Top);
                }

                strLine = oReadTxt.readLine();
                break;
            }

            if (strLine.startsWith("All", Qt::CaseInsensitive))
            {
                break;
            }
        }
    }

    readFile.close();
}

void VisualMemory::slotStaModeChanged(int index)
{
    switch (index)
    {
        case 0: // 精准内存泄漏模式
            GlobalData::getInstance()->memoryMode = 0;
            GlobalData::getInstance()->isPreciseMemoryLeakMode = true;
            break;
        case 1: // 常规内存泄漏模式
            GlobalData::getInstance()->memoryMode = 0;
            GlobalData::getInstance()->isPreciseMemoryLeakMode = false;
            break;
        case 2: // 增量内存统计模式
        default:
            GlobalData::getInstance()->memoryMode = 1;
            GlobalData::getInstance()->isPreciseMemoryLeakMode = false;
            break;
    }
}

void VisualMemory::attachAndCheckMemoryLeak(const QStringList &arguments)
{
    GlobalData::getInstance()->memoryMode = 0;
    GlobalData::getInstance()->isPreciseMemoryLeakMode = true;
    GlobalData::getInstance()->isAutoMemoryLeakMode = true;
    m_strAppFilePath = QDir::toNativeSeparators(arguments[1]);
    m_strAppName = m_strAppFilePath.mid(m_strAppFilePath.lastIndexOf(QDir::separator()) + 1);
    Utils::s_openAppName = m_strAppName;
    QString cmdLine = QString("\"%1\"").arg(m_strAppFilePath);
    for (int i=2; i<arguments.size(); i++)
    {
        cmdLine = QString("%1 %2").arg(cmdLine).arg(arguments[i]);
    }
    if (arguments.size() == 2)
    {
        cmdLine += " -e";
    }
    QDir dir(m_strAppFilePath);
    dir.cdUp();
    QDir::setCurrent(dir.path());
    g_DetoursTest.injectDllDetours(cmdLine);
    Utils::AddProcessDirToPath(m_strAppName);

    m_pConfigFile->setValue("Default/AppFilePath", m_strAppFilePath);

    CallStackDataController::instance()->m_rootItem = m_model_Top2Bottom->rootShowItem();
    CallStackDataController::instance()->m_rootItemBottom2Top = m_model_Bottom2Top->rootShowItem();

    HANDLE hStartEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_START_EVENT);
    if (WAIT_OBJECT_0 == WaitForSingleObject(hStartEvent, MAX_WAIT_TIME))
    {
        ::ResetEvent(hStartEvent);
        slotStartStatistics();
    }

    HANDLE hStopEvent = CreateEventA(NULL, TRUE, FALSE, GLOBAL_STATICS_STOP_EVENT);
    if (WAIT_OBJECT_0 == WaitForSingleObject(hStopEvent, MAX_WAIT_TIME))
    {
        ::ResetEvent(hStopEvent);
        slotStopStatistics();
    }
    
}

void VisualMemory::writeLog( QString logInfo )
{
    if (m_logFile.isOpen())
    {
        m_logFile.write("\\\\");
        m_logFile.write(logInfo.toLocal8Bit().data());
        m_logFile.write("\n");
        m_logFile.flush();
    }
}

void VisualMemory::refreshUI()
{
    qApp->processEvents();
}
