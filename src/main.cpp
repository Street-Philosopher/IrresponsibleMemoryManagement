#include <iostream>

#include "util.h"
#include "debugger.h"

#include "graphics.h"

#include "types.h"

#include "cramConstants.h"



using Debugger::IsBreakpoint;
using Debugger::DebugInit;
using Debugger::printdebug;


//these are so we can just access them and don't have to reference the cpu every time
#define A  CPU.A
#define B  CPU.B
#define C  CPU.C
#define D  CPU.D
#define HL CPU.HL
#define F  CPU.F
#define SP CPU.SP
#define PC CPU.PC

CPU_T CPU = CPU_T();

void mainloop();
int main(int argc, char* argv[]) {

	cls();

	if (argc >= 2) {
		if (LoadProgramToMemory(argv[1], CPU.base) == false) {
			LoadFileErrorMsg();
			getchar();
		}
	}

	graphicsinit();
	updatescreen(&CPU);	//if we don't do this the screen will be empty, not even black, which looks bad

	//initialises debug screen
	DebugInit(&CPU);
	Debugger::SetActive(false);

	mainloop();

}
//END MAIN PROGRAM

//#include <chrono>
//typedef std::chrono::high_resolution_clock Clock;
void mainloop() {
	
// t1 = Clock::now();
// t2 = Clock::now();
// std::cout << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1000000000 << std::endl;
	
	byte opcode;

	while(true) {
		
		if (CheckForDebugKey()) Debugger::SetActive(true);

		//debug is done before, so if we hit a breakpoint we show the instruction before it's executed
		//we first evaluate Debugger::IsActive so we don't have to call isbreakpoint if Debugger::IsActive is true
		if (Debugger::IsActive() || IsBreakpoint(PC, execute)) {
			Debugger::SetActive(true);
			printdebug(&CPU);
		}

		opcode = CPU.ReadNext();
		CPU.exec(opcode);

		//we dont do regular read here because 1) it's slow af, 2) it would be a nightmare for people actually interested in seeing if something is done there
		if (!(CPU.cramBase[CLOCK_COUNTER_HIGH] | CPU.cramBase[CLOCK_COUNTER_LOW]))
		{
			updatescreen(&CPU);
		}
	}
}



byte  GetRegVal(int n) {
	switch (n) {
		case 0: return A;
		case 1: return B;
		case 2: return C;
		case 3: return D;
		default: return -1;
	}
}
byte* GetRegPtr(int n) {
	switch (n) {
		case 0: return &A;
		case 1: return &B;
		case 2: return &C;
		case 3: return &D;
		default: return 0;
	}
}
bool CheckConditionalFromOpcode(byte opcode) {
	switch (opcode & 0b11) {
		case 0b00:		//unconditional
			return true;
		case 0b01:		//not zero
			return !CPU.GetFlag(0);
		case 0b10:		//carry
			return CPU.GetFlag(1);
			break;
		case 0b11:		//zeri
			return CPU.GetFlag(0);
		default: return false;	//the compiler complains otherwise
	}
}

bool GetBit(int value, int bitNumber) {
	return value & (1 << bitNumber);
}


//REGION opcodes
//syscodes
void nop () {  }

void push(uint16_t reg) {
	//push the two 8b components
	CPU.WriteMemory(0xFF00 + SP, (reg & 0x00FF));
	SP--;
	CPU.WriteMemory(0xFF00 + SP, (reg & 0xFF00) >> 8);
	SP--;

}
void pop(uint16_t* reg) {
	(*reg) = 0;

	//pop two 8b values in the temp variable
	SP++;
	(*reg) += CPU.ReadMemory(0xFF00 + SP) << 8;
	SP++;
	(*reg) += CPU.ReadMemory(0xFF00 + SP);

	CPU.SetFlag(0, (*reg) == 0);
}
// void pop(byte* reg) {
// 	SP++;
// 	(*reg) = CPU.ReadMemory(0xFF00 + SP);

// 	CPU.SetFlag(0, (*reg) == 0);
// }

void jp(word addr) {
	PC = addr;
}
void jr(byte relativeAddr) {
	PC += (sbyte)relativeAddr;
}
void call(word addr) {
	push(PC);
	PC = addr;

	//we call a function so we're one level deeper
	Debugger::IncCallDepth();
}
void ret() {

	pop(&PC);

	//we return from a function, so we're one level less deep
	Debugger::DecCallDepth();
}


//alu codes
void add(byte* Areg, byte* reg) {
	word result = (int)*Areg + *reg;//to catch carry
	*Areg += *reg;

	CPU.SetFlag(0, (*Areg == 0));	//set z flag
	CPU.SetFlag(1, result > 0xFF);	//set c flag
	CPU.SetFlag(2, GetBit(result, 7));	//sign flag
	CPU.SetFlag(3, GetBit(result, 0));	//parity flag
}
void addhl(word value) {
	int ans = (int)HL + value;
	HL = ans & 0xFFFF;

	CPU.SetFlag(0, HL == 0);
	CPU.SetFlag(1, ans > 0xFFFF);
	CPU.SetFlag(2, GetBit(ans, 15));
	CPU.SetFlag(3, GetBit(ans, 0));
}
void sub(byte* Areg, byte* reg) {
	int res = (int)*Areg - *reg;
	*Areg -= *reg;

	CPU.SetFlag(0, (*Areg == 0));	//set z flag
	CPU.SetFlag(1, (res < 0));		//set c flag
	CPU.SetFlag(2, GetBit(res, 7));//sign flag
	CPU.SetFlag(3, GetBit(res, 0));	//parity flag
}
void subhl(word value) {
	int res = (int)HL - value;
	HL = res & 0xFFFF;

	CPU.SetFlag(0, (HL == 0));	//set z flag
	CPU.SetFlag(1, (res < 0));		//set c flag
	CPU.SetFlag(2, GetBit(res, 15));//sign flag
	CPU.SetFlag(3, GetBit(res, 0));	//parity flag
}
void Xor(byte* Areg, byte* reg) {
	*Areg ^= *reg;

	CPU.SetFlag(0, (*Areg == 0));		//set z flag
	CPU.SetFlag(1, 0);					//set c flag
	CPU.SetFlag(2, GetBit(*Areg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*Areg, 0));	//parity flag
}
void  Or(byte* Areg, byte* reg) {
	*Areg |= *reg;

	CPU.SetFlag(0, (*Areg == 0));		//set z flag
	CPU.SetFlag(1, 0);					//set c flag
	CPU.SetFlag(2, GetBit(*Areg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*Areg, 0));	//parity flag
}
void And(byte* Areg, byte* reg) {
	*Areg &= *reg;

	CPU.SetFlag(0, (*Areg == 0));		//set z flag
	CPU.SetFlag(1, 0);					//set c flag
	CPU.SetFlag(2, GetBit(*Areg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*Areg, 0));	//parity flag
}
void inc(byte* reg) {
	(*reg)++;

	//if this is the case we went from 0xFF to 0x00 so both
	CPU.SetFlag(0, *reg == 0);
	CPU.SetFlag(1, *reg == 0);

	CPU.SetFlag(2, GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
	
}
void inc(word* reg) {
	(*reg)++;

	CPU.SetFlag(0, *reg == 0);
	CPU.SetFlag(1, *reg == 0);

	CPU.SetFlag(2, GetBit(*reg, 15));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
	
}
void dec(byte* reg) {
	(*reg)--;

	CPU.SetFlag(0, *reg == 0);
	//if we went from 0 to oxFF
	CPU.SetFlag(1, (*reg == 0xFF));

	CPU.SetFlag(2, GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
	
}
void dec(word* reg) {
	(*reg)--;

	CPU.SetFlag(0, *reg == 0);
	//if we went from 0 to oxFF
	CPU.SetFlag(1, (*reg == 0xFFFF));

	CPU.SetFlag(2, GetBit(*reg, 15));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag

}
void ror(byte* reg) {
	byte hadCarry  = CPU.GetFlag(1);
	bool willCarry = (*reg) & 1;

	(*reg) >>= 1;

	(*reg) |= hadCarry << 7;		//set bit 7 if we had the carry
	
	CPU.SetFlag(0, (*reg) == 0);
	CPU.SetFlag(1, willCarry);

	CPU.SetFlag(2, GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
}
void rol(byte* reg) {
	bool willCarry = (*reg) >= 0b10000000;

	(*reg) <<= 1;
	(*reg) |= (byte)CPU.GetFlag(1);

	CPU.SetFlag(0, (*reg) == 0);
	CPU.SetFlag(1, willCarry);

	CPU.SetFlag(2, GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
}
void Not(byte* reg) {
	(*reg) = ~(*reg);

	CPU.SetFlag(0, (*reg) == 0);

	CPU.SetFlag(2, GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*reg, 0));	//parity flag
}
//set z if equal, c if A is smaller. set parity flag if evenness is different, set sign flag if signs are different
void cmp(byte* Areg, byte* reg) {
	CPU.SetFlag(0, (*Areg) == (*reg));		//	z flag
	CPU.SetFlag(1, (*Areg) < (*reg));		//	c flag

	CPU.SetFlag(2, GetBit(*Areg, 7) != GetBit(*reg, 7));	//sign flag
	CPU.SetFlag(3, GetBit(*Areg, 0) != GetBit(*reg, 0));	//parity flag
}


//bitwise
void bit(int bit, byte* reg = &A) {
	bool set = *reg & (1 << bit);		//check if the given bit is set
	CPU.SetFlag(0, !set);				//set z flag accordingly
}
void res(int bit) {
	A &= ~(1 << bit);

	CPU.SetFlag(0, A==0);
	if (bit == 0) CPU.SetFlag(3, false);	//parity
	if (bit == 7) CPU.SetFlag(2, false);	//sign
}
void set(int bit) {
	A |= 1 << bit;

	CPU.SetFlag(0, false);	//always false bc we're setting a bit
	if (bit == 0) CPU.SetFlag(3, true);	//parity
	if (bit == 7) CPU.SetFlag(2, true);	//sign
}


//loads
void ld(byte* to, byte* from) {
	(*to) = (*from);
}
void ldhl(word* hl, word val) {
	(*hl) = val;
}


//video loads
void stv(word addr, byte val) {
	CPU.WriteVRAM(addr, val);
}
void ldv(byte* reg, word addr) {
	(*reg) = CPU.ReadVRAM(addr);
}

//ENDREGION opcodes

//undefine these bc this is a CPU function, so we don't need to do "CPU."
#undef A
#undef B
#undef C
#undef D
#undef F
#undef SP
#undef PC


void CPU_T::ExceptionHandler() {

	switch (cramBase[EXCEPTION_MODE_REGISTER]) {
		case EXCEPTION_MODE_IGNORE:
			break;
		case EXCEPTION_MODE_HANDLE:
			//deactivate the exception handler. to avoid recursive exceptions
			cramBase[EXCEPTION_MODE_REGISTER] = EXCEPTION_MODE_STOP;

			//push some information about the CPU
			push(PC);

			//we jump to the exception handler
			int addr;
			addr = (cramBase[EXC_HANDLE_HIGH] << 8) + cramBase[EXC_HANDLE_LOW];
			jp(addr);
			break;
		default:
		case EXCEPTION_MODE_STOP:
			throw BreakpointException();
	}
}


byte b1, b2;
word w1;		//variables used in some operations. here so compiler won't complain
void CPU_T::exec(byte opcode) {

	//cool but probably could cause problems. used to increase clock counter
	//word* pointer = (word*)&cramBase[CLOCK_COUNTER_LOW];
	//(*pointer)++;

	//debug cycle counter
	cyclecounter++;

	//increase actual cycle counter
	//we don't do normal r/w because same reason as above
	cramBase[CLOCK_COUNTER_LOW]++;
	if (cramBase[CLOCK_COUNTER_LOW] == 0) cramBase[CLOCK_COUNTER_HIGH]++;

	try {
	
		//we clear the buffer to make sure there are no leftovers from previous instruction
		ClearWriteBuffer();

		//ADD_OPCODE
		switch(opcode) {//system instructions
			case 0b00000000:					//nop
				nop();
				break;
			case 0b00000001:					//stop
				throw BreakpointException();	//will open the debugger
				break;
			case 0b00000010:					//halt
				//does nothing until you update the screen
				while (CPU.cramBase[CLOCK_COUNTER_LOW] | CPU.cramBase[CLOCK_COUNTER_HIGH]) {
					exec(0);
				}
				break;
			//jp &imm16
			case 0b00000100 ... 0b00000111:
				b1 = ReadNext();
				b2 = ReadNext();
				w1 = b1 + (0x100 * b2);
				//read the two following bytes for address

				if (CheckConditionalFromOpcode(opcode) == true)
					jp(w1);
				break;
			//jp (HL)
			case 0b01110000 ... 0b01110011:
				if (CheckConditionalFromOpcode(opcode) == true)
					jp(HL);
				break;
			//jr %imm8
			case 0b11000100 ... 0b11000111:
				b1 = ReadNext();
				if (CheckConditionalFromOpcode(opcode) == true)
					jr(b1);
				break;
			//call &imm16
			case 0b00001000 ... 0b00001011:
				b1 = ReadNext();
				b2 = ReadNext();    //read two bytes of params
				w1 = b1 + (0x100 * b2);

				if (CheckConditionalFromOpcode(opcode) == true)
					call(w1);
				break;
			//call (HL)
			case 0b00011100 ... 0b00011111:
				if (CheckConditionalFromOpcode(opcode) == true)
					call(HL);
				break;
			//ret
			case 0b00001100 ... 0b00001111:
				if (CheckConditionalFromOpcode(opcode) == true)
					ret();
				break;
			case 0b00011000:					//push HL
				push(HL);
				break;
			case 0b00011001:					//push AB
				w1 = B + (0x100 * A);
				push(w1);
				break;
			case 0b00011010:					//pop HL
				pop(&HL);
				break;
			case 0b00011011:					//pop AB
				pop(&w1);
				A = (0xFF00 & w1) >> 8;
				B = (0x00FF & w1);
				break;


			//alu instructions
			//add A,reg
			case 0b00100000 ...0b00100011:
						//this gets the register to use
				add(&A, GetRegPtr(opcode & 0b11));
				break;
			//sub A,reg
			case 0b00100100 ... 0b00100111:
				sub(&A, GetRegPtr(opcode & 0b11));
				break;
			case 0b00010011:					//add A,%imm
				b1 = ReadNext();
				add(&A, &b1);
				break;
			case 0b00010101:					//sub A,%imm
				b1 = ReadNext();
				sub(&A, &b1);
				break;
			case 0b11000000:					//add HL,&imm
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				addhl(w1);
				break;
			case 0b11000001:					//sub HL,&imm
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				subhl(w1);
				break;
			//and A,%imm
			case 0b00101000:
				b1 = ReadNext();
				And(&A, &b1);
				break;
			//and A,reg
			case 0b00101001 ... 0b00101011:
				And(&A, GetRegPtr(opcode & 0b11));
				break;
			//xor A,reg
			case 0b00101100 ... 0b00101111:
				Xor(&A, GetRegPtr(opcode & 0b11));
				break;
			//or  A,%imm
			case 0b00110000:
				b1 = ReadNext();
				Or(&A, &b1);
				break;
			//or  A,reg
			case 0b00110001 ... 0b00110011:
				Or (&A, GetRegPtr(opcode & 0b11));
				break;
			//inc reg
			case 0b00110100 ... 0b00110111:
				inc(GetRegPtr(opcode & 0b11));
				break;
			//dec reg
			case 0b00111000 ... 0b00111011:
				dec(GetRegPtr(opcode & 0b11));
				break;
			case 0b00111100:					//inc HL
				inc(&HL);
				break;
			case 0b00111101:					//inc SP
				inc(&SP);
				break;
			case 0b00111110:					//dec HL
				dec(&HL);
				break;
			case 0b00111111:					//dec SP
				dec(&SP);
				break;
			case 0b10000000:					//cmp A,%imm
				b1 = ReadNext();
				cmp(&A, &b1);
				break;
			//cmp A,reg
			case 0b10000001 ... 0b10000011:
				cmp(&A, GetRegPtr(opcode & 0b11));
				break;
			//not reg
			case 0b01110100 ... 0b01110111:
				Not(GetRegPtr(opcode & 0b11));
				break;
			//ror reg
			case 0b01111000 ... 0b01111011:
				ror(GetRegPtr(opcode & 0b11));
				break;
			//rol reg
			case 0b01111100 ... 0b01111111:
				rol(GetRegPtr(opcode & 0b11));
				break;
			case 0b00010100:					//add HL,AB
				w1 = (A * 0x100) + B;
				addhl(w1);
				break;
			//adc A,reg
			case 0b10101000 ... 0b10101011:
				b1 = GetFlag(1);	//if carry set bit to 1
				add(&A, GetRegPtr(opcode & 0b11));
				add(&A, &b1);		//add to carry flag
				break;


			//bit operations
			//bit x,A
			case 0b10010000 ... 0b10010111:
				bit(opcode & 0b111);
				break;
			//res x,A
			case 0b10011000 ... 0b10011111:
				res(opcode & 0b111);
				break;
			//set x,A
			case 0b10100000 ... 0b10100111:
				set(opcode & 0b111);
				break;
			
			//flag x
			// case 0b10111000:
			// case 0b10111001:
			case 0b10111010 ... 0b10111111:
				bit(opcode & 0b111, &F);	//it's really just a fancy "bit x". only difference it's on the F register
				break;


			//load instructions
			//ld reg1,reg2
			case 0b01000001:
			case 0b01000010:
			case 0b01000011:
			case 0b01000100:
			case 0b01000110:
			case 0b01000111:
			case 0b01001000:
			case 0b01001001:
			case 0b01001011:
			case 0b01001100:
			case 0b01001101:
			case 0b01001110:
			// case 0b01000000:
			// case 0b01000101:
			// case 0b01001010:
			// case 0b01001111:
				ld(
					GetRegPtr((opcode % 0b10000) / 0b100),		//gets bits 2 and 3
					GetRegPtr(opcode & 0b11)						//gets bits 0 and 1
				);
				break;
			//ld reg,%imm
			case 0b01010000 ... 0b01010011:
				b1 = ReadNext();			//parameter
				ld(
					GetRegPtr(opcode & 0b11),						//get register
					&b1
				);
				break;
			case 0b00010001:					//ld A,H
				b1 = HL / 0x100;
				ld (&A, &b1);
				break;
			case 0b00010010:					//ld A,L
				b1 = HL % 0x100;
				ld (&A, &b1);
				break;
			case 0b01011110:					//ld H,A
				w1 = HL % 0x100;	//it means we keep low
				w1 += A * 0x100;

				ldhl(&HL, w1);
				break;
			case 0b01011111:					//ld L,A
				w1 = HL / 0x100;	//it means we keep high
				w1 *= 0x100;
				//add A, to replace L
				w1 += A;

				ldhl(&HL, w1);
				break;
			//ld reg,(HL)
			case 0b01010100 ... 0b01010111:
				b1 = ReadMemory(HL);					//read what's in HL
				ld(GetRegPtr(opcode & 0b11), &b1);			//load it in the register
				break;
			//ld (HL),reg
			case 0b01011000 ... 0b01011011:
				WriteMemory(HL, GetRegVal(opcode & 0b11));
				break;
			case 0b01011100:					//ld HL,%imm16
				w1 = ReadNext();			//low
				w1 += ReadNext() * 0x100;	//high
				ldhl(&HL, w1);
				break;
			case 0b01011101:					//ld HL,AB
				w1 =  B;			//low
				w1 += A * 0x100;	//high
				ldhl(&HL, w1);
				break;
			case 0b11000010:
				A = (HL & 0xFF00) >> 8;
				B = HL & 0xFF;
				break;
			//ld (&imm),reg
			case 0b01100000 ... 0b01100011:
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				WriteMemory(w1, GetRegVal(opcode & 0b11));
				break;
			//ld reg,(&imm)
			case 0b01100100 ... 0b01100111:
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				b1 = ReadMemory(w1);
				ld(GetRegPtr(opcode & 0b11), &b1);
				break;
			//ldi reg,(HL)
			case 0b01101000 ... 0b01101011:
				b1 = ReadMemory(HL);					//read what's in HL
				inc(&HL);		//this way we set flags
				ld(GetRegPtr(opcode & 0b11), &b1);			//load it in the register
				break;
			//ldi (HL),reg
			case 0b01101100 ... 0b01101111:
				WriteMemory(HL, GetRegVal(opcode & 0b11));
				inc(&HL);
				break;
			//ldd reg,(HL)
			case 0b10000100 ... 0b10000111:
				b1 = ReadMemory(HL);					//read what's in HL
				dec(&HL);
				ld(GetRegPtr(opcode & 0b11), &b1);			//load it in the register
				break;
			//ldd (HL),reg
			case 0b10001000 ... 0b10001011:
				WriteMemory(HL, GetRegVal(opcode & 0b11));
				dec(&HL);
				break;
			
			
			//video
			//stv (&imm),reg
			case 0b10101100 ... 0b10101111:
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				stv(w1, GetRegVal(opcode & 0b11));
				break;
			//ldv reg,(&imm)
			case 0b10110000 ... 0b10110011:
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				ldv(GetRegPtr(opcode & 0b11), w1);
				break;
			case 0b10110100:					//stv (HL),A
				stv(HL, A);		//store at the address pointed by HL
				break;
			case 0b10110101:					//stv (HL+),A
				stv(HL, A);	//store at the address pointed by HL
				inc(&HL);
				break;
			case 0b10110110:					//stv (HL-),A
				stv(HL, A);	//store at the address pointed by HL
				dec(&HL);
				break;
			case 0b10110111:					//ldv A,(HL)
				ldv(&A, HL);	//load from the address pointed by HL
				break;
			case 0b10001100:					//ccv (HL),(&addr)
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				b2 = 8;
				while (b2--) {	//repeat 8 times

					//load from address in video ram and store to HL
					b1 = ReadVRAM(w1);
					WriteVRAM(HL, b1);

					w1++;
					HL++;
				}

				break;
			case 0b10001101:					//ccr (HL),(&addr)
				w1 = ReadNext();
				w1 += 0x100 * ReadNext();
				b2 = 8;
				while (b2--) {	//repeat 8 times

					//load from address in main ram and store to HL
					b1 = ReadMemory(w1);
					WriteVRAM(HL, b1);

					w1++;
					HL++;
				}

				break;
			case 0b10001110:					//ccv (HL),(AB)
				w1 = (0x100 * A) + B;
				b2 = 8;
				while (b2--) {	//repeat 8 times

					//load from address in video ram and store to HL
					b1 = ReadVRAM(w1);
					WriteVRAM(HL, b1);

					w1++;
					HL++;
				}

				//increase AB register since we didnt use it directly
				B += 8;
				if (B < 8) //we had an overflow in B, so inc A
					A++;
			
				break;
			case 0b10001111:					//ccr (HL),(AB)
				w1 = (0x100 * A) + B;
				b2 = 8;
				while (b2--) {	//repeat 8 times

					//load from address in video ram and store to HL
					b1 = ReadMemory(w1);
					WriteVRAM(HL, b1);

					w1++;
					HL++;
				}

				//increase AB register since we didn't use it directly
				B += 8;
				if (B < 8) //we had an overflow
					A++;
				break;
			

			//ldh
			case 0b00010110:					//ldh (%addr),A
				b1 = ReadNext();
				WriteCRAM(b1, A);
				break;
			case 0b00010111:					//ldh A,(%addr)
				b1 = ReadNext();
				A = ReadCRAM(b1);
				break;

			
			//bad
			default:
				ExceptionHandler();
				break;
		}

		ApplyWriteBuffer();

	} //end try
	//this is triggered if a breakpoint is hit when breakpoints are not enabled
	catch (BreakpointException) {
		
		//enable debug
		Debugger::SetActive(true);
		//go back to previous instruction
		PC -= InstructionLength(opcode);
		//this way we can actually see what triggered it
	}
}
