#include <iostream>
#include "DebugSession.h"
#include "CmdHandlers.h"



//启动被调试进程命令的处理函数。
//命令格式为 s path
//如果路径中有空格，则应该用双引号将其括住。
void OnStartDebug(const Command& cmd) {

	//if (cmd.size() < 2) {

	//	std::wcout << TEXT("Lack path.") << std::endl;
	//	return;
	//}

	//StartDebugSession(cmd[1].c_str());
}