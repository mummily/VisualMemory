#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IPCDLL_EXPORTS
#define IPC_SDK_API __declspec(dllexport)
#else
#define IPC_SDK_API __declspec(dllimport)
#endif

    IPC_SDK_API int serverInit();
    IPC_SDK_API int serverSend(char* pData, int nSize);
    IPC_SDK_API int serverUninit();

    IPC_SDK_API int clientInit();
    IPC_SDK_API int clientRead(char* pData, int nSize);
    IPC_SDK_API int clientUninit();

#ifdef __cplusplus
}
#endif