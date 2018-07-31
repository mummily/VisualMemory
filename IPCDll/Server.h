#pragma once
#include "ShareMemory.h"

class Server
{
public:
    Server(void);
    virtual ~Server(void);

public:
    int init(int nSize);
    int readData(char* pData, int& nSize);
    int unInit();

protected:
    ShareMemory* m_pShMem;
};

