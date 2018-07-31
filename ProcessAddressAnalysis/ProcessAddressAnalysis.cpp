// ProcessAddressAnalysis.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>
#include <qstring>
#include "GTJCommon.h"
#include <DbgHelp.h>
#include <iostream>
#include <QDebug>
using namespace std;
using namespace ggp;
const UINT max_name_length = 256;	// Max length of symbols' name.

BOOL CALLBACK SymEnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    QString str;
    if (pSymInfo != 0)
    {                        
        //if (strcmp("KeAttachProcess", pSymInfo->Name) == 0)
        {        
            str = QString("名称: %s 地址: %08x:").arg(pSymInfo->Name).arg((DWORD)pSymInfo->Address);
            cout<<str.toLocal8Bit().data();
        }                
    }
    return TRUE;
}

std::vector<QString> strDllNameArr;
std::vector<HANDLE> strModleInfoList;
void getProccDllName(HANDLE process)
{
    //// 获取该进程的快照
    //MODULEENTRY32 me32;
    //DWORD nPid = GetProcessId(process);
    //HANDLE hProcessDll = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, nPid);
    //
    //me32.dwSize = sizeof(MODULEENTRY32);
    //if (hProcessDll == NULL)
    //{
    //    return ;
    //}
    //do
    //{
    //    strDllNameArr.push_back(QString::fromWCharArray(me32.szExePath));
    //    LPMODULEINFO modelInfo = new MODULEINFO;
    //    modelInfo->lpBaseOfDll = me32.modBaseAddr;
    //    modelInfo->SizeOfImage = me32.modBaseSize;
    //    modelInfo->EntryPoint = me32.;
    //    strModleInfoList.push_back()
    //}
    //while( Module32Next(hProcessDll, &me32));
}

//获取dll函数列表
int getModleFunList()
{
    DWORD dwFileSize;//文件大小
    QString str;
    DWORD error;//错误
    LPCWSTR FilePathName = L"G:\\myProject\\Blackhouse\\Win32\\Release\\VisualMemory.dll";//文件全路径
    QString pSymPath = "G:\\myProject\\Blackhouse\\Win32\\Release";
    HANDLE pHandle = GetModuleHandle(NULL);
    pHandle = ExeOpt::getProcessHandle(L"ProcessAddressAnalysis.exe");
    SymCleanup(pHandle);
    if (!SymInitialize(pHandle, NULL, true))
    {
        error = GetLastError();
        str = QString("SymInitialize returned error  %x").arg(error);
        qDebug()<<(str);
        return 0;
    }
    //得到文件大小
    HANDLE hFile = CreateFile(FilePathName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        error = GetLastError();
        str = QString("SCreateFile error  %x").arg(error);
        qDebug()<<(str);
        return false;
    }

    if (INVALID_FILE_SIZE == (dwFileSize = GetFileSize(hFile, NULL)))
    {
        error = GetLastError();
        str = QString("GetFileSize error  %x").arg(error);
        qDebug()<<(str);
    }
    CloseHandle(hFile);

    HMODULE pModleHandle = LoadLibrary(FilePathName);
    //调用SymLoadModule函数载入对应符号库
    DWORD64 dw64ModAddress = SymLoadModule64(pHandle, NULL, "VisualMemory.dll", NULL, (DWORD64)pModleHandle, dwFileSize);

    if (dw64ModAddress == 0)
    {
        return 0;
    }

    //使用SymEnumSymbols函数枚举模块中的符号
    if (!SymEnumSymbols(pHandle,
        dw64ModAddress,
        NULL, // Null point out that list all symbols
        SymEnumSymbolsProc,  //回调过程
        NULL))
    {
        return 0;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE pHandle = ExeOpt::getProcessHandle(L"ProcessAddressAnalysis.exe");
    getProccDllName(pHandle);
    system("pause");
	return 0;
}

