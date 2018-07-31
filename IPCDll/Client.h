#pragma once

#include "ShareMemory.h"


class Client
{
public:
    Client(void);
    virtual ~Client(void);

public:
    int init(int nSize);
    int writeData(char* pData, int nSize);
    int unInit();

protected:
    ShareMemory* m_pShMem;
};

