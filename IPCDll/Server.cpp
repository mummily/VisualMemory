#include "Server.h"

#define GLOBAL_MEMORY_NAME  ("Local\\GLOBAL_SHARE_MEM_NAME")

Server::Server(void)
{
}


Server::~Server(void)
{
}

int Server::init(int nSize)
{
    m_pShMem = new ShareMemory();
    return m_pShMem->createShareMemory(nSize, GLOBAL_MEMORY_NAME);
}

int Server::readData(char* pData, int& nSize)
{
    return m_pShMem->readData(pData, nSize);
}

int Server::unInit()
{
    m_pShMem->closeShareMemory();
    delete m_pShMem;
    return 0;
}