#pragma once

#include <vector>

//makes it easier to iterate over all elements in a list/array
#define foreach(x, y) for(auto x: y)

void cls();

std::string ByteToHex(int byte);
std::string WordToHex(int word);
std::string ByteToBin(int byte);
std::string WordToBin(int word);
int GetByteFromStr(std::string str);
int GetAddrFromStr(std::string str);

bool LoadProgramToMemory(std::string name, std::uint8_t* memory);

std::string codeToMnemonic(std::uint8_t opcode);

int InstructionLength(std::uint8_t opcode);

std::vector<std::string> splitString(const std::string& s);
