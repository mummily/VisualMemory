#include "Client.h"

#define GLOBAL_MEMORY_NAME  ("Local\\GLOBAL_SHARE_MEM_NAME")
Client::Client(void)
{
}


Client::~Client(void)
{
}

int Client::init(int nSize)
{
    m_pShMem = new ShareMemory();
    return m_pShMem->openShareMemory(nSize, GLOBAL_MEMORY_NAME);
}

int Client::writeData(char* pData, int nSize)
{
    return m_pShMem->writeData(pData, nSize);
}

int Client::unInit()
{
    m_pShMem->closeShareMemory();
    delete m_pShMem;
    return 0;
}
