#include <iostream>
#include <stdio.h>
#include <list>
#include <sstream>
#include <vector>
#include <bitset>

#include "types.h"

using std::string;
using std::vector;
using std::to_string;
using std::list;
using std::hex;
using std::stringstream;
using std::cout;

char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

#define BYTE_MAX 0xFF
#define BYTE_MIN 0x00
#define ADDR_MAX 0xFFFF
#define ADDR_MIN 0x0000


void cls() {
	#ifdef __linux__
	system("clear");
	#elif _WIN32
	system("cls");
	#endif
	// cout << "\033[2J\033[0;0H";
	// cout.flush();
}


string ByteToHex(int byte) {
	if (byte < BYTE_MIN) return "00";
	if (byte > BYTE_MAX) return "FF";

	string res = "";
	res += digits[byte / 0x10];
	res += digits[byte % 0x10];

	return res;
}
string WordToHex(int word) {
	if (word < ADDR_MIN) return "00";

	string res = "";

	res += digits[(word % 0x10000) / 0x1000];
	res += digits[(word % 0x1000) / 0x100];
	res += digits[(word % 0x100) / 0x10];
	res += digits[(word % 0x10) / 0x1];
	
	return res;
}

string ByteToBin(int byte) {

	//create 8 bits from the given number
	return std::bitset<8>(byte).to_string();
}
string WordToBin(int word) {
	
	//create 16 bits from the number
	return std::bitset<16>(word).to_string();
}


int GetByteFromStr(string str) {
	int addr;
	if (str[0] == '0' && str[1] == 'x') {		//hex number
		str = str.substr(2, str.length() - 2);
		stringstream ss;
		ss << hex << str;			//hex => dec
		ss >> addr;
	}
	else addr = stoi(str);						//dec number

	//error
	if (addr > BYTE_MAX || addr < BYTE_MIN) return -1;

	else return addr;
}
int GetAddrFromStr(string str) {
	int addr;
	if (str[0] == '0' && str[1] == 'x') {		//hex number
		str = str.substr(2, str.length() - 2);
		stringstream ss;
		ss << hex << str;			//hex => dec
		ss >> addr;
	}
	else addr = stoi(str);						//dec number

	//error
	if (addr > ADDR_MAX || addr < ADDR_MIN) return -1;

	else return addr;
}


vector<string> splitString(const string& s)
{
    stringstream ss(s);
    vector<string> words;
    for (string w; ss>>w; ) words.push_back(w);
    return words;
}


//return good if read with no probs
bool LoadProgramToMemory(string name, byte* memory) {
	try {
		FILE* f = fopen(name.c_str(), "rb");		//get file
		if (f == NULL) throw int(12);

		int size;
		fseek(f, 0, SEEK_END);			//get size
		size = ftell(f);
		if (size > 0xFFFF) throw 2;		//bad size
		fseek(f, 0, SEEK_SET);			//res position

		fread(memory, 1, size, f);		//write the program to the memory at offset 0

		fclose(f);						//close

		return 1;
	}
	catch(...) { return 0; }
}

void LoadFileErrorMsg() {
	std::cout << "there was an error loading the program" << std::endl;
}


//ADD_OPCODE:
//returns the number of bytes an instruction requires
int InstructionLength(byte opcode) {

	switch(opcode) {
		//system instructions
		case 0b00000000:			//nop
			return 0;
		case 0b00000001:			//stop
			return 1;
		case 0b00000010:			//halt
			return 1;
		case 0b00000100:			//jp &addr
			return 3;
		case 0b00000101:
			return 3;
		case 0b00000110:
			return 3;
		case 0b00000111:
			return 3;
		case 0b01110000:			//jp (HL)
			return 1;
		case 0b01110001:
			return 1;
		case 0b01110010:
			return 1;
		case 0b01110011:
			return 1;
		case 0b00001000:			//call &addr
			return 3;
		case 0b00001001:
			return 3;
		case 0b00001010:
			return 3;
		case 0b00001011:
			return 3;
		case 0b00011100:			//call (HL)
			return 1;
		case 0b00011101:
			return 1;
		case 0b00011110:
			return 1;
		case 0b00011111:
			return 1;
		case 0b00001100:			//ret
			return 1;
		case 0b00001101:
			return 1;
		case 0b00001110:
			return 1;
		case 0b00001111:
			return 1;
		case 0b00011000:			//push HL
			return 1;
		case 0b00011001:			//push AB
			return 1;
		case 0b00011010:			//pop HL
			return 1;
		case 0b00011011:			//pop AB
			return 1;

		//alu instructions
		//add A,reg
		case 0b00100000:
		case 0b00100001:
		case 0b00100010:
		case 0b00100011:
			return 1;
		//sub A,reg
		case 0b00100100:
		case 0b00100101:
		case 0b00100110:
		case 0b00100111:
			return 1;
		case 0b00010011:					//add A,%imm
			return 2;
		case 0b00010101:					//sub A,%imm
			return 2;
		case 0b11000000:					//add HL,&imm
			return 3;
		case 0b11000001:					//sub HL,&imm
			return 3;
		//and A,%imm
		case 0b00101000:
			return 2;
		//and A,reg
		case 0b00101001:
		case 0b00101010:
		case 0b00101011:
			return 1;
		//xor A,reg
		case 0b00101100:
		case 0b00101101:
		case 0b00101110:
		case 0b00101111:
			return 1;
		//or  A,%imm
		case 0b00110000:
			return 2;
		//or  A,reg
		case 0b00110001:
		case 0b00110010:
		case 0b00110011:
			return 1;
		//inc reg
		case 0b00110100:
		case 0b00110101:
		case 0b00110110:
		case 0b00110111:
			return 1;
		//dec reg
		case 0b00111000:
		case 0b00111001:
		case 0b00111010:
		case 0b00111011:
			return 1;
		case 0b00111100:					//inc HL
			return 1;
		case 0b00111101:					//inc SP
			return 1;
		case 0b00111110:					//dec HL
			return 1;
		case 0b00111111:					//dec SP
			return 1;
		case 0b10000000:					//cmp A,%imm
			return 2;
		//cmp A,reg
		case 0b10000001:
		case 0b10000010:
		case 0b10000011:
			return 1;
		//not reg
		case 0b01110100:
		case 0b01110101:
		case 0b01110110:
		case 0b01110111:
			return 1;
		//ror reg
		case 0b01111000:
		case 0b01111001:
		case 0b01111010:
		case 0b01111011:
			return 1;
		//rol reg
		case 0b01111100:
		case 0b01111101:
		case 0b01111110:
		case 0b01111111:
			return 1;
		case 0b00010100:					//add HL,AB
			return 1;
		//adc A,reg
		case 0b10101000:
		case 0b10101001:
		case 0b10101010:
		case 0b10101011:
			return 1;

		//bit operations
		//bit x,A
		case 0b10010000:
		case 0b10010001:
		case 0b10010010:
		case 0b10010011:
		case 0b10010100:
		case 0b10010101:
		case 0b10010110:
		case 0b10010111:
			return 1;
		//res x,A
		case 0b10011000:
		case 0b10011001:
		case 0b10011010:
		case 0b10011011:
		case 0b10011100:
		case 0b10011101:
		case 0b10011110:
		case 0b10011111:
			return 1;
		//set x,A
		case 0b10100000:
		case 0b10100001:
		case 0b10100010:
		case 0b10100011:
		case 0b10100100:
		case 0b10100101:
		case 0b10100110:
		case 0b10100111:
			return 1;
		//flag x
		case 0b10111010:
		case 0b10111011:
		case 0b10111100:
		case 0b10111101:
		case 0b10111110:
		case 0b10111111:
		// case 0b10111000:
		// case 0b10111001:
			return 1;		

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
			return 1;
		//ld reg,%imm
		case 0b01010000:
		case 0b01010001:
		case 0b01010010:
		case 0b01010011:
			return 2;
		case 0b00010001:					//ld A,H
			return 1;
		case 0b00010010:					//ld A,L
			return 1;
		case 0b01011110:					//ld H,A
			return 1;
		case 0b01011111:					//ld L,A
			return 1;
		//ld reg,(HL)
		case 0b01010100:
		case 0b01010101:
		case 0b01010110:
		case 0b01010111:
			return 1;
		//ld (HL),reg
		case 0b01011000:
		case 0b01011001:
		case 0b01011010:
		case 0b01011011:
			return 1;
		case 0b01011100:					//ld HL,&imm
			return 3;
		case 0b01011101:					//ld HL,AB
			return 1;
		case 0b11000010:					//ld AB,HL
			return 1;
		//ld (&addr),reg
		case 0b01100000:
		case 0b01100001:
		case 0b01100010:
		case 0b01100011:
			return 3;
		//ld reg,(&addr)
		case 0b01100100:
		case 0b01100101:
		case 0b01100110:
		case 0b01100111:
			return 3;
		//ldi reg,(HL)
		case 0b01101000:
		case 0b01101001:
		case 0b01101010:
		case 0b01101011:
			return 1;
		//ldi (HL),reg
		case 0b01101100:
		case 0b01101101:
		case 0b01101110:
		case 0b01101111:
			return 1;
		//ldd reg,(HL)
		case 0b10000100:
		case 0b10000101:
		case 0b10000110:
		case 0b10000111:
			return 1;
		//ldd (HL),reg
		case 0b10001000:
		case 0b10001001:
		case 0b10001010:
		case 0b10001011:
			return 1;
		
		//video instructions
		//stv (&addr),reg
		case 0b10101100:
		case 0b10101101:
		case 0b10101110:
		case 0b10101111:
			return 3;
		//ldv reg,(&addr)
		case 0b10110000:
		case 0b10110001:
		case 0b10110010:
		case 0b10110011:
			return 3;
		case 0b10110100:					//stv (HL),A
			return 1;
		case 0b10110101:					//stv (HL+),A
			return 1;
		case 0b10110110:					//stv (HL-),A
			return 1;
		case 0b10110111:					//ldv A,(HL)
			return 1;
		//ccv/ccr (HL),(&addr)
		case 0b10001100:
		case 0b10001101:
			return 3;
		//ccv/ccr (HL),(AB)
		case 0b10001110:
		case 0b10001111:
			return 1;
		
		//ldh
		case 0b00010110:					//ldh (%addr),A
			return 2;
		case 0b00010111:					//ldh A,(%addr)
			return 2;

		//invalid code, no parameters
		default:
			return 1;
	}
}

//ADD_OPCODE:
string codeToMnemonic(byte opcode) {
	
	string reg1(1, (char)('A' + (opcode % 4)));
	//only used by "ld reg,reg" but ok
	string reg2(1, (char)('A' + (opcode % 16) / 4));

	//any '&' will be changed to a 16bit immediate, any '%' to an 8bit one

	switch(opcode) {
		//system instructions
		case 0b00000000:
			return "nop";
		case 0b00000001:
			return "stop";
		case 0b00000010:
			return "halt";
		case 0b00000100:
			return "jp &";
		case 0b00000101:
			return "jnz &";
		case 0b00000110:
			return "jc &";
		case 0b00000111:
			return "jz &";
		case 0b01110000:
			return "jp (HL)";
		case 0b01110001:
			return "jnz (HL)";
		case 0b01110010:
			return "jc (HL)";
		case 0b01110011:
			return "jz (HL)";
		case 0b00001000:
			return "call &";
		case 0b00001001:
			return "cnz &";
		case 0b00001010:
			return "callc &";
		case 0b00001011:
			return "callz &";
		case 0b00011100:
			return "call (HL)";
		case 0b00011101:
			return "cnz (HL)";
		case 0b00011110:
			return "callc (HL)";
		case 0b00011111:
			return "callz (HL)";
		case 0b00001100:
			return "ret";
		case 0b00001101:
			return "retnz";
		case 0b00001110:
			return "retc";
		case 0b00001111:
			return "retz";
		case 0b00011000:
			return "push HL";
		case 0b00011001:
			return "push AB";
		case 0b00011010:
			return "pop HL";
		case 0b00011011:
			return "pop AB";

		//alu instructions
		case 0b00100000:
		case 0b00100001:
		case 0b00100010:
		case 0b00100011:
			return "add A," + reg1;
		case 0b00100100:
		case 0b00100101:
		case 0b00100110:
		case 0b00100111:
			return "sub A," + reg1;
		case 0b00010011:
			return "add A,%";
		case 0b00010101:
			return "sub A,%";
		case 0b11000000:
			return "add HL,&";
		case 0b11000001:
			return "sub HL,&";
		case 0b00101000:
			return "and A,%";
		case 0b00101001:
		case 0b00101010:
		case 0b00101011:
			return "and A," + reg1;
		case 0b00101100:
		case 0b00101101:
		case 0b00101110:
		case 0b00101111:
			return "xor A," + reg1;
		case 0b00110000:
			return "or A,%";
		case 0b00110001:
		case 0b00110010:
		case 0b00110011:
			return  "or A," + reg1;
		case 0b00110100:
		case 0b00110101:
		case 0b00110110:
		case 0b00110111:
			return "inc " + reg1;
		case 0b00111000:
		case 0b00111001:
		case 0b00111010:
		case 0b00111011:
			return "dec " + reg1;
		case 0b00111100:
			return "inc HL";
		case 0b00111101:
			return "inc SP";
		case 0b00111110:
			return "dec HL";
		case 0b00111111:
			return "dec SP";
		case 0b10000000:
			return "cmp A,%";
		case 0b10000001:
		case 0b10000010:
		case 0b10000011:
			return "cmp A," + reg1;
		case 0b01110100:
		case 0b01110101:
		case 0b01110110:
		case 0b01110111:
			return "not " + reg1;
		case 0b01111000:
		case 0b01111001:
		case 0b01111010:
		case 0b01111011:
			return "ror " + reg1;
		case 0b01111100:
		case 0b01111101:
		case 0b01111110:
		case 0b01111111:
			return "rol " + reg1;
		case 0b00010100:
			return "add HL,AB";
		case 0b10101000:
		case 0b10101001:
		case 0b10101010:
		case 0b10101011:
			return "adc A," + reg1;

		//bit operations
		case 0b10010000:
		case 0b10010001:
		case 0b10010010:
		case 0b10010011:
		case 0b10010100:
		case 0b10010101:
		case 0b10010110:
		case 0b10010111:
			return "bit " + to_string(opcode % 8) + ",A";
		case 0b10011000:
		case 0b10011001:
		case 0b10011010:
		case 0b10011011:
		case 0b10011100:
		case 0b10011101:
		case 0b10011110:
		case 0b10011111:
			return "res " + to_string(opcode % 8) + ",A";
		case 0b10100000:
		case 0b10100001:
		case 0b10100010:
		case 0b10100011:
		case 0b10100100:
		case 0b10100101:
		case 0b10100110:
		case 0b10100111:
			return "set " + to_string(opcode % 8) + ",A";
		case 0b10111010:
		case 0b10111011:
		case 0b10111100:
		case 0b10111101:
		case 0b10111110:
		case 0b10111111:
		// case 0b10111000:
		// case 0b10111001:
			return "flag " + to_string(opcode % 8);

		//load instructions
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
			return "ld " + reg2 + "," + reg1;
		case 0b01010000:
		case 0b01010001:
		case 0b01010010:
		case 0b01010011:
			return "ld " + reg1 + ",%";
		case 0b00010001:
			return "ld A,H";
		case 0b00010010:
			return "ld A,L";
		case 0b01011110:
			return "ld H,A";
		case 0b01011111:
			return "ld L,A";
		case 0b01010100:
		case 0b01010101:
		case 0b01010110:
		case 0b01010111:
			return "ld " + reg1 + ",(HL)";
		case 0b01011000:
		case 0b01011001:
		case 0b01011010:
		case 0b01011011:
			return "ld (HL)," + reg1;
		case 0b01011100:
			return "ld HL,&";
		case 0b01011101:
			return "ld HL,AB";
		case 0b11000010:
			return "ld AB,HL";
		case 0b01100000:
		case 0b01100001:
		case 0b01100010:
		case 0b01100011:
			return "ld (&)," + reg1;
		case 0b01100100:
		case 0b01100101:
		case 0b01100110:
		case 0b01100111:
			return "ld " + reg1 + ",(&)";
		case 0b01101000:
		case 0b01101001:
		case 0b01101010:
		case 0b01101011:
			return "ldi " + reg1 + ",(HL)";
		case 0b01101100:
		case 0b01101101:
		case 0b01101110:
		case 0b01101111:
			return "ldi (HL)," + reg1;
		case 0b10000100:
		case 0b10000101:
		case 0b10000110:
		case 0b10000111:
			return "ldd " + reg1 + ",(HL)";
		case 0b10001000:
		case 0b10001001:
		case 0b10001010:
		case 0b10001011:
			return "ldd (HL)," + reg1;
		
		//video instructions
		case 0b10101100:
		case 0b10101101:
		case 0b10101110:
		case 0b10101111:
			return "stv (&)," + reg1;
		case 0b10110000:
		case 0b10110001:
		case 0b10110010:
		case 0b10110011:
			return "ldv " + reg1 + ",(&)";
		case 0b10110100:
			return "stv (HL),A";
		case 0b10110101:
			return "stv (HL+),A";
		case 0b10110110:
			return "stv (HL-),A";
		case 0b10110111:
			return "ldv A,(HL)";
		case 0b10001100:
			return "ccv (HL),(&)";
		case 0b10001101:
			return "ccr (HL),(&)";
		case 0b10001110:
			return "ccv (HL),(AB)";
		case 0b10001111:
			return "ccr (HL),(AB)";

		//ldh
		case 0b00010110:
			return "ldh (%),A";
		case 0b00010111:
			return "ldh A,(%)";

		default:
			return "invalid";
	}
}