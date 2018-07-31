#ifndef VisualMemory_H
#define VisualMemory_H

#include "VisualMemory_global.h"
#include <QtWidgets/QMainWindow>
#include <QMap>
#include <QSet>
#include <QThread>
#include <Windows.h>
#include <QFile>
#include "Utils.h"

class QSettings;
class QTableWidget;
class TreeModel;
class TreeItem;

namespace Ui
{
    class VisualMemoryClass;
}

class MainWindowStarter : public  QObject
{
    Q_OBJECT

public:
    MainWindowStarter(){}
    void showMainWindow();

public slots:
    void slotShowMainWindow();

signals:
    void sigShowMainWindow();
};

class VISUALMEMORY_EXPORT VisualMemory : public QMainWindow
{
    Q_OBJECT

public:
    VisualMemory(QWidget *parent = 0);
    ~VisualMemory();

public slots:
    void slotAttachApp();
    void slotStartStatistics();
    void slotStopStatistics();

private slots:
    void slotPauseApp();
    void slotAbout();
    void slotSaveResult();
    void slotOpenResult();
    void slotBatchOpenResult();
    void slotStaModeChanged(int index);
    void refreshUI();

public slots:


private:
    void showStatisticsResult(const QString &statisticsFile);
    void writeLog(QString logInfo);
    void openOneFile(const QString &fileName);
    void saveToFile(const QString &fileName);

    TreeModel *m_model_StartMemory;
    TreeModel *m_model_EndMemory;
    TreeModel *m_model_DiffMemory;

    Ui::VisualMemoryClass *ui;
    QTableWidget *m_tableWidget;
    TreeModel *m_model_Top2Bottom;
    TreeModel *m_model_Bottom2Top;

    
    // 待统计应用程序名称
    QString m_strAppName;
    // 启动的应用程序全路径
    QString m_strAppFilePath;

    QAction *m_pAttachAppAct;
    QAction *m_pStartStaAct;
    QAction *m_pStopStaAct;
    QAction *m_pPauseAppAct;

    QSettings *m_pConfigFile;

    QMap<QString, TreeItem*> m_functionMemorySizeMap;
    QFile m_logFile;
    int m_hasReadItemCount; // 已读取记录数
    int m_validFileTotal; // 有效内存文件数

public:
    void attachAndCheckMemoryLeak(const QStringList &arguments);
    static VisualMemory* m_pInstance;
};

class VMEventReceiver : public QThread, public EventController
{
    Q_OBJECT

public:
    VMEventReceiver(QObject *parent = nullptr) : QThread(parent)
    {
    }

    virtual void run() override
    {
        while (true)
        {
            if (WAIT_OBJECT_0 == WaitForSingleObject(hStartEvent, 100))
            {
                //::ResetEvent(hStartEvent);
                //emit sigStart();
            }

            if (WAIT_OBJECT_0 == WaitForSingleObject(hStopEvent, 100))
            {
                //::ResetEvent(hStopEvent);
                //emit sigStop();
            }

            if (WAIT_OBJECT_0 == WaitForSingleObject(hExitEvent, 100))
            {
                if (GlobalData::getInstance()->memoryMode != 0 || GlobalData::getInstance()->curStep != 3)
                {
                    ::ResetEvent(hExitEvent); //temp
                }
            }
        }
    }

signals:
    void sigStart();
    void sigStop();
};

class VISUALMEMORY_EXPORT VMEventSender : public EventController
{
public:
    VMEventSender(QObject *parent = nullptr)
    {
    }

    void sendStartEvent()
    {
        ::SetEvent(hStartEvent);
        while (WAIT_OBJECT_0 == WaitForSingleObject(hStartEvent, 100))
        {
            // 等待
            ::Sleep(100);
        }
    }

    void sendStopEvent()
    {
        ::SetEvent(hStopEvent);
        while (WAIT_OBJECT_0 == WaitForSingleObject(hStopEvent, 100))
        {
            // 等待
            ::Sleep(100);
        }
    }
};

#endif // VisualMemory_H
