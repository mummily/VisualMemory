#include <iostream>
#include <string>
#include "CmdHandlers.h"
#include "DebugSession.h"



//继续被调试进程执行命令的处理函数。
//命令格式为 g [c]
//如果省略了c参数，则意味着调试器未处理异常。
//否则意味着调试器处理了异常。
void OnGo(const Command& cmd) {

	//CHECK_DEBUGGEE;

	//if (cmd.size() < 2) {

	//	HandledException(FALSE);
	//	ContinueDebugSession();
	//	return;
	//}

	//if (cmd[1] == TEXT("c")) {

	//	HandledException(TRUE);
	//	ContinueDebugSession();
	//}
}