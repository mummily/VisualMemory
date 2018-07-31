#pragma once

#include <windows.h>


#define STATUS_NONE 1
#define STATUS_SUSPENDED 2
#define STATUS_INTERRUPTED 3



void StartDebugSession(LPCTSTR path);
void ContinueDebugSession();
void StopDebugSeesion();
void HandledException(BOOL handled);
BOOL GetDebuggeeContext(CONTEXT* pContext);
int GetDebuggeeStatus();
BOOL ReadDebuggeeMemory(unsigned int address, unsigned int length, void* pData);
HANDLE GetDebuggeeHandle();



#define CHECK_DEBUGGEE \
	do { \
		if (GetDebuggeeStatus() == STATUS_NONE) {	\
			std::wcout << L"Debuggee has not started yet." << std::endl; \
			return; \
		} \
	} \
	while (0)