#ifndef MEMORYTEST_H
#define MEMORYTEST_H

#include <QtWidgets/QMainWindow>
#include "ui_memorytest.h"

class MemoryTest : public QMainWindow
{
    Q_OBJECT

public:
    MemoryTest(QWidget *parent = 0);
    ~MemoryTest();

private:
    Ui::MemoryTestClass ui;
};

#endif // MEMORYTEST_H
