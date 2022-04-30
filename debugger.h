#pragma once

#include "types.h"

namespace Debugger {

void DebugInit(CPU_T* CPU);

void printdebug(CPU_T* _cpu);
void SetActive(bool value);
bool IsActive();

bool IsBreakpoint(word address, breakpointMode mode);

void IncCallDepth();
void DecCallDepth();

}