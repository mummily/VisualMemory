#include <iostream>
#include <sstream>
#include <iomanip>
#include "HelperFunctions.h"
#include "DebugSession.h"
#include "CmdHandlers.h"

#include <QString>
#include <QDebug>


//void DumpHex(unsigned int address, unsigned int length);
char ByteToChar(BYTE byte);



//显示内存内容的命令处理函数。
//命令格式为 d [address] [length]
//address为16进制，length为10进制。
//如果省略了第三个参数，则length为128。
//如果省略了第二个参数，则address为EIP指向的地址。
void OnDump(const Command& cmd) {

	CHECK_DEBUGGEE;

	unsigned int address = 0;
	unsigned int length = 128;

	if (cmd.size() < 2) {

		CONTEXT context;
		GetDebuggeeContext(&context);

		address = context.Eip;
	}
	else {

		address = wcstoul(cmd[1].c_str(), NULL, 16);
	}

	if (cmd.size() > 2) {

		length = _wtoi(cmd[2].c_str());
	}

	//DumpHex(address, length);	
}



//以十六进制的格式显示指定地址出的内存。每一行固定显示16个字节，输出的地址要对齐到16的倍数。
//显示的内容先输出到一个wostringstream对象中，
//最后再输出这个对象中的字符串。
void DumpHex(unsigned int address, unsigned int length) {

	std::wostringstream dumpStr;
	dumpStr.imbue(std::locale("chs", std::locale::ctype));
	dumpStr << std::hex << std::uppercase;

	//存储每一行字节对应的ASCII字符。
	char content[17] = { 0 };

	//将起始输出的地址对齐到16的倍数。
	unsigned int blankLen = address % 16;
	unsigned int startAddress = address - blankLen;

	unsigned int lineLen = 0;
	unsigned int contentLen = 0;

	//输出对齐后第一行的空白。
	if (blankLen != 0) {

		dumpStr << std::setw(8) << TEXT('0') << startAddress << TEXT("  "); // qingh temp
		startAddress += 16;

		for (unsigned int len = 0; len < blankLen; ++len) {

			dumpStr << TEXT("   ");
			content[contentLen] = TEXT(' ');
			++contentLen;
			++lineLen;
		}
	}

	//逐个字节读取被调试进程的内存，并输出。
	BYTE byte;
    QString strShow;
	for (unsigned int memLen = 0; memLen < length; ++memLen) {

		unsigned int curAddress = address + memLen;

		//如果是每行的第一个字节，则先输出其地址。
		if (lineLen % 16 == 0) {

			dumpStr << std::setw(8) << TEXT('0') << startAddress << TEXT("  "); // qingh temp
			startAddress += 16;

			lineLen = 0;
		}

		//读取内存，如果成功的话，则原样输出，并获取其对应的ASCII字符。
		if (ReadDebuggeeMemory(curAddress, 1, &byte) == TRUE) {
			
			dumpStr << std::setw(2) << TEXT('0') << byte << TEXT(' '); // qingh temp
			content[contentLen] = ByteToChar(byte);
            strShow = strShow + QString(" ") + QString::number((int)byte, 16);
		}
		//如果读取失败，则以 ?? 代替。
		else {

			dumpStr << TEXT("?? ");
			content[contentLen] = TEXT('.');
		}

		//如果一行满了16个字节，则输出换行符。由于从0开始计数，所以这里与15比较。
		if (contentLen == 15) {
			dumpStr << TEXT(' ') << content << std::endl;
		}

		++contentLen;
		contentLen %= 16;

		++lineLen;
	}

	//输出最后一行的空白，为了使最后一行的ASCII字符对齐上一行。
	for (unsigned int len = 0; len < 16 - lineLen; ++len) {

		dumpStr << TEXT("   ");
	}

	//输出最后一行的ASCII字符。
	dumpStr << TEXT(' ');
	content[contentLen] = 0;
	dumpStr << content;

	std::wcout << dumpStr.str() << std::endl;
    qDebug() << "qingh-a, strShow = " << address << length << strShow;
}



//将字节转换成字符。不能显示的字符分别以 . 和 ？ 表示。
char ByteToChar(BYTE byte) {

	if (byte >= 0x00 && byte <= 0x1F) {
		return '.';
	}

	if (byte >= 0x81 && byte <= 0xFF) {
		return '?';
	}

	return (char)byte;
}