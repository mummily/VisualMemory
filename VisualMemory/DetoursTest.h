#pragma once
#include <QString>
#include <QObject>

class DetoursTest
{
public:
    DetoursTest(void);
    ~DetoursTest(void);

    void injectDllDetours(QString appFullPath);
    void Hook();
    void UnHook();

    static DetoursTest *instance();

    bool m_bIsHook;
    static int m_iShowIntevalTimes;

    static bool m_bIsStating;
};
