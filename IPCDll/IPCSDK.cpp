#include "IPCSdk.h"
#include "Server.h"
#include "Client.h"

#define SHARE_MEMORY_LEN (12*1024*1024)

Server* g_pServer = nullptr;
Client* g_pClient = nullptr;

IPC_SDK_API int serverInit()
{
    return 0;
}

IPC_SDK_API int serverSend(char* pData, int nSize)
{
    return 0;
}

IPC_SDK_API int serverUninit()
{
    return 0;
}

IPC_SDK_API int clientInit()
{
    return 0;
}

IPC_SDK_API int clientRead(char* pData, int nSize)
{;
    return 0;
}

IPC_SDK_API int clientUninit()
{
    return 0;
}