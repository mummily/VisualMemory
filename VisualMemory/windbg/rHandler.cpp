#include <iostream>
#include <iomanip>
#include "DebugSession.h"
#include "CmdHandlers.h"
#include "HelperFunctions.h"



//显示寄存器值命令的处理函数。
//命令格式为 r
void OnShowRegisters(const Command& cmd) {

	CHECK_DEBUGGEE;

	CONTEXT context;

	if (GetDebuggeeContext(&context) == FALSE) {
		std::wcout << TEXT("Show registers failed.") << std::endl;
		return;
	}

	//输出EAX
	std::wcout << TEXT("EAX = ");
	PrintHex(context.Eax, FALSE);

	//输出EBX
	std::wcout << TEXT("    EBX = ");
	PrintHex(context.Ebx, FALSE);
	std::wcout << std::endl;

	//输出ECX
	std::wcout << TEXT("ECX = ");
	PrintHex(context.Ecx, FALSE);

	//输出EDX
	std::wcout << TEXT("    EDX = ");
	PrintHex(context.Edx, FALSE);
	std::wcout << std::endl;
	
	//输出ESI
	std::wcout << TEXT("ESI = ");
	PrintHex(context.Esi, FALSE);

	//输出EDI
	std::wcout << TEXT("    EDI = ");
	PrintHex(context.Edi, FALSE);
	std::wcout << std::endl;

	//输出EBP
	std::wcout << TEXT("EBP = ");
	PrintHex(context.Ebp, FALSE);

	//输出ESP
	std::wcout << TEXT("    ESP = "); 
	PrintHex(context.Esp, FALSE); 
	std::wcout << std::endl;
			
	//输出EIP
	std::wcout << TEXT("EIP = ");
	PrintHex(context.Eip, FALSE);
	std::wcout << TEXT("    ");

	//CF 在第0位
	if ((context.EFlags & 0x1) != 0) {

		std::wcout << L"CF  ";
	}

	//PF 在第2位
	if ((context.EFlags & 0x4) != 0) {

		std::wcout << L"PF  ";
	}

	//AF 在第4位
	if ((context.EFlags & 0x10) != 0) {
		
		std::wcout << L"AF  ";
	}

	//ZF 在第6位
	if ((context.EFlags & 0x40) != 0) {

		std::wcout << L"ZF  ";
	}

	//SF 在第7位
	if ((context.EFlags & 0x80) != 0) {

		std::wcout << L"SF  ";
	}

	//OF 在第11位
	if ((context.EFlags & 0x400) != 0) {

		std::wcout << L"OF  ";
	}

	//DF 在第10位
	if ((context.EFlags & 0x200) != 0) {

		std::wcout << L"DF  ";
	}

	std::wcout << std::endl;
}