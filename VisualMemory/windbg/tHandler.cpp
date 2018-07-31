#include <iostream>
#include "DebugSession.h"
#include "CmdHandlers.h"



//结束被调试进程命令的处理函数。
//命令格式为 t
void OnStopDebug(const Command& cmd) {

	CHECK_DEBUGGEE;

	StopDebugSeesion();
}