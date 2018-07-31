#include "VisualMemory.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 获取命令行参数
    QStringList arguments = QCoreApplication::arguments();

    VisualMemory w;
    if (arguments.size() >= 2)
    {
        QString argument2 = arguments[1];
        if (argument2.compare("cmd", Qt::CaseInsensitive) == 0)
        {
            if (arguments[2].compare("start", Qt::CaseInsensitive) == 0)
            {
                VMEventSender sender;
                sender.sendStartEvent();
                ::Sleep(1000);
            }
            else if (arguments[2].compare("stop", Qt::CaseInsensitive) == 0)
            {
                VMEventSender sender;
                sender.sendStopEvent();
                ::Sleep(1000); 
            }
        }
        else
        {
            w.showMinimized();
            w.attachAndCheckMemoryLeak(arguments);
            ::Sleep(5000);
            return 0;
        }

        return 0;
    }
    else
    {
        w.show();
        return a.exec();
    }

    return a.exec();
}
