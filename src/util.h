#pragma once

#include <vector>

//makes it easier to iterate over all elements in a list/array
#define foreach(x, y) for(auto x: y)
#define tforeach(type, x, y) for(type x: y)

#define WINDOW_TITLE (std::string)"IMM-8 Emulator"

void cls();

std::string ByteToHex(int byte);
std::string WordToHex(int word);
std::string ByteToBin(int byte);
std::string WordToBin(int word);
int GetByteFromStr(std::string str);
int GetAddrFromStr(std::string str);

bool LoadProgramToMemory(std::string name, std::uint8_t* memory);

std::string CodeToMnemonic(std::uint8_t opcode);

int InstructionLength(std::uint8_t opcode);

std::vector<std::string> splitString(const std::string& s);

void LoadFileErrorMsg();
