#pragma once

#include <iostream>

#define  RAM_SIZE 0x10000
#define VRAM_SIZE 0x10000
#define CRAM_SIZE 0x100

typedef unsigned short word;
typedef std::uint8_t byte;
typedef char sbyte;
//typedef byte* pt;

/*
union LargeRegister {
	word full;
	struct {
		byte high, low;
	}
}
*/

enum breakpointMode { execute, read, write };

class CPU_T {

	private:

	//called at the end of a cycle. will write all buffered memory writes
	void ApplyWriteBuffer();
	//clears the buffered memory changes
	void ClearWriteBuffer();

	public:

	//this is only for the debugger
	//we techinally will never need THIS much space, but it's nice to have it
	unsigned long long cyclecounter = 0;


	const int CYCLES_PER_FRAME = 65536;

	//arrays bc with malloc the memory is reset to zero which is boring
	byte base[RAM_SIZE];
	byte vramBase[VRAM_SIZE];
	byte cramBase[CRAM_SIZE];

	void exec(byte opcode);		//executes instruction for given opcode

	//REGION registers
	word PC;				//program counter
	byte F,		 			//000000cz
		SP;					//stack pointer
	byte A, B, C, D;		//general purpose registers
	word HL;				//HL
	//ENDREGION

	 CPU_T();
	~CPU_T();

	void WriteMemory(word address, byte value);
	byte ReadMemory (word address);
	byte ReadNext();
	void WriteVRAM(word address, byte value);
	byte ReadVRAM (word address);
	void WriteCRAM(byte address, byte value);
	byte ReadCRAM (byte address);

	void ExceptionHandler();
	void SetActiveExceptionType(byte code);

	bool GetFlag(int num);
	void SetFlag(int num, bool value);

	void init();
	void reset();

};

class BreakpointException : public std::exception {

	public:
	const char * what () const throw ()
    {
    	return "breakpoint reached";
    }
};