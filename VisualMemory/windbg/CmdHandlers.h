#pragma once

#include "Command.h"



void OnStartDebug(const Command& cmd);
void OnShowRegisters(const Command& cmd);
void OnStopDebug(const Command& cmd);
void OnGo(const Command& cmd);
void OnDump(const Command& cmd);
void DumpHex(unsigned int address, unsigned int length);
