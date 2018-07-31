#ifndef VISUALMEMORYAPP_H
#define VISUALMEMORYAPP_H

#include <QtWidgets/QMainWindow>
#include "ui_VisualMemoryApp.h"

class VisualMemoryApp : public QMainWindow
{
    Q_OBJECT

public:
    VisualMemoryApp(QWidget *parent = 0);
    ~VisualMemoryApp();

private:
    Ui::VisualMemoryAppClass ui;
};

#endif // VISUALMEMORYAPP_H
