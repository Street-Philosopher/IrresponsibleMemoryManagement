#include "debugger.h"
#include "util.h"
#include "cramConstants.h"
//#include "types.h"

#include <list>

using std::list;
using Debugger::IsBreakpoint;
using Debugger::IsBreakpointVRAM;
using Debugger::IsBreakpointCRAM;


//void RandomiseMemory(byte* pointer, int size) {
//	srand(time(NULL));
//	for (int i = 0; i < size; i++) {
//		pointer[i] = rand() % 256;
//	}
//}


struct memorybuffer {
	word address;
	byte value;
};
list<memorybuffer> mb, vmb, cmb;

//writes to memory all buffered changes
void CPU_T::ApplyWriteBuffer() {

	//write to the address of the buffer the given value
	foreach(i, mb) {
		base[i.address] = i.value;
	}
	foreach(i, vmb) {
		vramBase[i.address] = i.value;
	}
	foreach(i, cmb) {
		cramBase[i.address & 0xFF] = i.value;
	}

	//clear as we're done
	ClearWriteBuffer();
}
void CPU_T::ClearWriteBuffer() {
	mb.clear();
	vmb.clear();
	cmb.clear();
}


bool CPU_T::GetFlag(int num) {
	return F & 1 << num;   //check if the corresponding flag is set, ignoring the others
}
void CPU_T::SetFlag(int num, bool value) {
	if (value == true)		//set
		F |=  (1 << num);
	else					//reset
		F &= ~(1 << num);
}

//constructor
CPU_T::CPU_T() {

/*
	base = (pt)malloc(0x10000);
	vramBase = (pt)malloc(0x10000);
	cramBase = (pt)malloc(0x100);
*/
	
	init();
}
CPU_T::~CPU_T() {
/*
	//no, this doesn't cause a memory leak
	free(base);
	free(vramBase);
	free(cramBase);
*/
}
void CPU_T::init() {

	SP = 0xFF;
	PC = 0x0000;

	//ADD_CRAM: set initial cram values
	// cycle counter
	cramBase[CLOCK_COUNTER_LOW]  = 0;
	cramBase[CLOCK_COUNTER_HIGH] = 0;

	//exception handling mode
	cramBase[EXCEPTION_MODE_REGISTER] = EXCEPTION_MODE_HALT;

/*
	//not necessary, but having all set to zero is boring
	RandomiseMemory(base, 0x10000);
	RandomiseMemory(vramBase, 0x10000);
	RandomiseMemory(cramBase, 0x100);
*/

	A  = rand() % 256;
	B  = rand() % 256;
	C  = rand() % 256;
	D  = rand() % 256;
	HL = rand() & 0xFFFF;
}
void CPU_T::reset() {
	init();
}


void CPU_T::WriteMemory(word address, byte value) {

	//if we hit a breakpoint and breakpoints were disabled we throw this exception
	//it's handled in the exec loop. there it will go back one instruction and activate debug mode
	if (!Debugger::IsActive() && IsBreakpoint(address, write)) {
		throw BreakpointException();
	}
	
	//this will buffer the read. all reads will be set at the end of the CPU cycle
	//this is to make it work with breakpoints
	memorybuffer temp;
	temp.address = address;
	temp.value = value;
	mb.push_back(temp);
}
byte CPU_T::ReadMemory (word address) {

	if (!Debugger::IsActive() && IsBreakpoint(address, read)) {
		throw BreakpointException();
	}

	return base[address];
}
byte CPU_T::ReadNext() {	//read and increase program counter
	return ReadMemory(PC++);
}

void CPU_T::WriteVRAM(word address, byte value) {

	//TODO: breakpoints

	//same as for normal read, but in a separate buffer
	memorybuffer temp;
	temp.address = address;
	temp.value = value;
	vmb.push_back(temp);
}
byte CPU_T::ReadVRAM(word address) {
	
	//TODO: breakpoints

	return vramBase[address];
}
void CPU_T::WriteCRAM(byte address, byte value) {

	// if (!Debugger::IsActive() && IsBreakpointCRAM(address, write)) {
	// 	throw BreakpointException();
	// }

	//same as for normal read, but in a separate buffer
	memorybuffer temp;
	temp.address = address;
	temp.value = value;
	cmb.push_back(temp);
}
byte CPU_T::ReadCRAM(byte address) {

	// if (!Debugger::IsActive() && IsBreakpointCRAM(address, read)) {
	// 	throw BreakpointException();
	// }
	
	return cramBase[address];
}