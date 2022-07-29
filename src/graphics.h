#define GRAPHICS

#ifndef GRAPHICS
#include "types.h"

void updatescreen(CPU_T* cpu);

void graphicsinit();

void SaveScreen(std::string state);
void LoadScreen(std::string state);

bool CheckForDebugKey();
#endif