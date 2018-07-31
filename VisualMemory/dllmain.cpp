// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include "VisualMemory.h"

//HINSTANCE g_hDLLInst = NULL;
//
//BOOL APIENTRY DllMain( HMODULE hModule,
//    DWORD  ul_reason_for_call,
//    LPVOID lpReserved
//    )
//{
//    switch (ul_reason_for_call)
//    {
//    case DLL_PROCESS_ATTACH:
//        {
//            g_hDLLInst = hModule;
//
//            //VisualMemory *mainwindow = new VisualMemory();
//            //mainwindow->show();
//
//            //DisableThreadLibraryCalls(g_hDLLInst);
//            //DetourTransactionBegin();
//            //DetourUpdateThread(GetCurrentThread());
//
//            //pNetUserAdd = (PNETUSERADD)DetourFindFunction("Netapi32.dll", "NetUserAdd");
//            //if (pNetUserAdd == NULL)
//            //{
//            //    pNetUserAdd = NetUserAdd;
//            //    OutputDebugString(L"pNetUserAdd not found!\r\n");
//            //}
//
//            //if(DetourAttachEx(&(PVOID&)pNetUserAdd, MyNetUserAdd))
//            //    SrTrace(L"NetUserAdd() detoured successfully\r\n");
//
//            //pStartDocW = (PSTARTDOCW)DetourFindFunction("gdi32.dll", "StartDocW");
//            //if (pStartDocW == NULL)
//            //{
//            //    pStartDocW = StartDocW;
//            //    OutputDebugString(L"StartDocW not found!\r\n");
//            //}
//
//            //if(DetourAttachEx(&(PVOID&)pStartDocW, MyStartDocW))
//            //    SrTrace(L"StartDocW() detoured successfully\r\n");
//
//            //SystemFunction = (HeapAlloc_Type)DetourFindFunction("msvcr100","malloc");
//            //DetourAttach(&(PVOID&)SystemFunction,MyHookMalloc);
//
//            //DetourTransactionCommit();
//        }
//        break;
//    case DLL_THREAD_ATTACH:
//        {
//            //if (VisualMemory::m_pInstance == nullptr && qApp != nullptr && qApp->activeWindow() != nullptr)
//            //{
//            //    VisualMemory *mainwindow = new VisualMemory();
//            //    mainwindow->moveToThread(g_pMainThread);
//            //    mainwindow->show();
//            //    //QMessageBox::information(nullptr, "msg", "test");
//            //}
//        }
//        break;
//    case DLL_THREAD_DETACH:
//        break;
//    case DLL_PROCESS_DETACH:
//        {
//
//            //DetourTransactionBegin();
//            //DetourUpdateThread(GetCurrentThread());
//
//            //DetourDetachEx(&(PVOID&)pNetUserAdd, MyNetUserAdd);
//
//            //DetourDetachEx(&(PVOID&)pStartDocW, MyStartDocW);
//
//            //DetourAttach(&(PVOID&)SystemFunction,MyHookMalloc);
//
//            //DetourTransactionCommit();
//
//        }
//        break;
//    }
//    return TRUE;
//}