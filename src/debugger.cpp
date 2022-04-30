#include "debugger.h"
#include "util.h"

#include <sstream>
#include <list>

//all we need it for is to load and save image in save states, and updatescreen
#include "graphics.h"

using std::string;
using std::list;
using std::cout;
using std::vector;
using std::exception;
using std::to_string;
using std::stoi;
using std::cin;
using std::endl;
using std::getline;


//todo: load rom, better breakpoint checking, fare cose per gli altri due tipi di ram, help specifico

// when we press enter without a command this will be repeated
string lastcommand = "s";

//bounds of the memory
#define ADDRESS_MIN 0x0000
#define ADDRESS_MAX 0xFFFF

//declaring it in the header causes several problems, so we have to do this instead
bool dodebug = true;
void Debugger::SetActive(bool value) { dodebug = value; }
bool Debugger::IsActive() { return dodebug; }

//these are the flags set when you do something like "finish" or "next"
bool doBreakDepth = false;	//this tells you whether you should break
int breakAtDepth = 0;		//this at what value should you break if the above is true

int callDepth = 0;			//this is the current depth of the call stack
void Debugger::IncCallDepth() { callDepth++; }
void Debugger::DecCallDepth() { callDepth--; }


//breakpoints
typedef struct BreakPoint_T {
	word address;
	breakpointMode mode;
} BreakPoint;

list<BreakPoint> breakpoints;


const string ssFolder = "saveStates/";
int LoadState(CPU_T* _cpu, int num);
int SaveState(CPU_T* _cpu, int num);


void Debugger::DebugInit(CPU_T* CPU) {
	//save state to be used in the reset command
	SaveState(CPU, -1);
}

//check if the given address has a breakpoint in the given mode
bool Debugger::IsBreakpoint(word address, breakpointMode mode) {

	//call depth checking stuff
	if (mode == execute) {
		if (doBreakDepth == true) {
			if (callDepth == breakAtDepth) {
				doBreakDepth = false;	//reset as we've done what we had to and don't want to break again
				return true;
			}
		}
	}

	//check the actual breakpoints
	foreach (bp, breakpoints) {
		if (bp.address == address && bp.mode == mode) return true;
	}

	return false;
}
string BPModeToString(breakpointMode mode) {
	//there is no way of converting enums to string so we do this
	switch(mode) {
		case (execute):
			return "execute";
			break;
		case (write):
			return "write";
			break;
		case (read):
			return "read";
			break;
	}
}


//prints the option menu. it's defined at the bottom
void OptionsMenu(vector<string> command);


//this way we can ignore the buffer
void WriteMemory(CPU_T* CPU, word address, byte value, int destination) {
	switch (destination) {
		case 0:
			CPU->base[address & 0xFFFF] = value;
			break;
		case 1:
			CPU->vramBase[address & 0xFFFF] = value;
			break;
		case 2:
			CPU->cramBase[address & 0xFF] = value;
			break;
	}
}


//the 'codeToMnemonic' function has a '&' symbol where a 16-bit value is needed, and a '%' where an 8-bit one is.
//this functions substitutes them for the correct values
string AddImmediatesToDisassembly(string original, CPU_T* _cpu) {

	//first check for 16-bit
	int pos = original.find('&');
	while (pos != string::npos) {			//repeat until there are no '&' left
		
		//split the string in two so that the '&' is left out
		string part1 = original.substr(0, pos);
		string part2 = original.substr(pos + 1);

		//it's a 16bit value so read next two bytes
		byte b1 = _cpu->ReadMemory(_cpu->PC + 1),		// low
			 b2 = _cpu->ReadMemory(_cpu->PC + 2);		// high
		//the computer is little endian so we have to do b2 before b1
		string addressStr = "0x" + ByteToHex(b2) + ByteToHex(b1);

		//now the string has the address where it should be
		original = part1 + addressStr + part2;

		pos = original.find('&');
	}

	//then check for 8-bit
	pos = original.find('%');
	while (pos != string::npos) {			//repeat until there are no '%' left
		
		//split the string in two so that the '%' is left out
		string part1 = original.substr(0, pos);
		string part2 = original.substr(pos + 1);

		//it's an 8bit value so read next byte
		byte b1 = _cpu->ReadMemory(_cpu->PC + 1);
		
		string addressStr = "0x" + ByteToHex(b1);

		//now the string has the address where it should be
		original = part1 + addressStr + part2;

		pos = original.find('%');
	}

	return original;
}

void Debugger::printdebug(CPU_T* _cpu) {

	//do it before anything as we don't want to repeat it in the debugger

	updateDebugInfo:
	cls();

	word pc = _cpu->PC;

	auto opcode = _cpu->ReadMemory(pc);

	auto instDisassembly = codeToMnemonic(opcode);
	instDisassembly = AddImmediatesToDisassembly(instDisassembly, _cpu);

	cout << "PC =	" << WordToHex(pc) << " => " << ByteToHex(opcode) << " " << ByteToHex(_cpu->ReadMemory(pc + 1)) << " " << ByteToHex(_cpu->ReadMemory(pc + 2)) << " ... (" << instDisassembly << ")" << "\n";
	cout << "registers:\nA		" << ByteToHex(_cpu->A) << "\nB		" << ByteToHex(_cpu->B) << "\nC		" << ByteToHex(_cpu->C) << "\nD		" << ByteToHex(_cpu->D);
	cout << "\nHL		" << WordToHex(_cpu->HL);
	cout << "\nSP		" << ByteToHex(_cpu->SP);
	cout << "\nFlags:\n" <<
	"z	-	" << (int)_cpu->GetFlag(0) << "\n" <<
	"c	-	" << (int)_cpu->GetFlag(1) << "\n" <<
	"s	-	" << (int)_cpu->GetFlag(2) << "\n" <<
	"p	-	" << (int)_cpu->GetFlag(3) << "\n" <<
	"\n";
	
	cout.flush();

	askcommand:
	string command;
	cin.clear();
	getline(cin, command);

	//continue to next instruction
	if (command == "") command = lastcommand;
	
	auto tokens = splitString(command);

	try
	{
		//command list
		if (tokens.at(0) == "help") {
			//todo: specific help

			cout << "\n\n"
					<< "pressing enter will repeat the last step-like instruction" << "\n"
					<< "press esc while running to start debugging" << "\n\n"
					
					<< "commands:" << "\n"

					<< "help                - list all commands" << "\n"
					<< "help command        - get a descritpion of the syntax of the given command" << "\n"
//					<< "options             - list all options" << "\n"

 					<< "step / s            - execute the next instruction" << "\n"
 					<< "next / n            - execute the next instruction skipping over function calls" << "\n"
 					<< "finish / f          - run until the end of the current function" << "\n"

					<< "cc                  - print the value of the counter" << "\n"
					<< "cc reset            - print the value of the counter, then reset it" << "\n"

//					<< "set name val        - set the given option to the given value" << "\n"


					<< "m [v/c] addr        - print the memory value at \"addr\"" << "\n"
					<< "m [v/c] add1-add2   - print the values from \"add1\" to \"add2\"" << "\n"

					<< "w [v/c] addr val    - write \"val\" to \"addr\"" << "\n"

					<< "reg name val        - load \"val\" in the given register" << "\n"

					<< "b [v/c] [mode] addr - set a breakpoint at \"addr\" (the mode is -e by default)" << "\n"
						<< "                modes:" << "\n"
						<< "                	'-e'  => on execution (default)" << "\n"
						<< "                	'-r'  => on read" << "\n"
						<< "                	'-w'  => on write" << "\n"
						<< "                	'-l'  => list all breakpoints" << "\n"
						<< "                	'-d'  => delete breakpoint" << "\n"
						<< "                	'-da' => delete all breakpoints" << "\n"

					<< "jp addr             - jump to the given address" << "\n"

					<< "dis addr            - disassemble the opcode at the given address" << "\n"
					
					<< "ss num              - saves the state with the given number (0-9)" << "\n"
					<< "ls num              - loads the state with the given number (0-9)" << "\n"

					<< "update              - force a screen update" << "\n"
					<< "cls                 - clear the console" << "\n"

					<< "run                 - continue running until next breakpoint" << "\n"
					<< "reset               - reset the CPU" << "\n"
					;
		}
		//options list
		else if (tokens.at(0) == "options") {

			cout << "\n"
					<< "placeholder         - text"
					;
		}

		//settings
		else if (tokens.at(0) == "set") {
			OptionsMenu(tokens);
		}

		//finish
		else if (command == "finish" || command == "f") {
			doBreakDepth = true;
			breakAtDepth = callDepth - 1;	//we want to break when we get out of the function, so one level higher
		
			//exit from debug screen
			dodebug = false;
			cls();
			return;
		}

		//finish
		else if (command == "step" || command == "s") {
			lastcommand = "s";
			return;
		}
		
		//next
		else if (command == "next" || command == "n") {
			lastcommand = "n";

			doBreakDepth = true;
			breakAtDepth = callDepth;	//we want to break at the next instruction, so same level. this way we can also skip funcs
		
			//exit from debug screen
			dodebug = false;
			cls();
			return;
		}

		//clock counter
		else if (tokens.at(0) == "cc") {
			std::cout << _cpu->cyclecounter << "\n";

			if (tokens.size() == 2 && tokens.at(1) == "reset")
				_cpu->cyclecounter = 0;
		}

		//memory viewer
		else if (tokens.at(0) == "m") {

			if (tokens.size() < 2) {
				cout << "invalid parameter number for 'm'";
				goto end;
			}

			int destination = -1;	//0 is normal ram, 1 is vram, 2 is cram

			if (tokens.size() == 2) {	//no specified destination, so second element is address
				destination = 0;
				command = tokens.at(1);
			} else if (tokens.size() == 3) {	//address is third element bc destination is second
				if (tokens.at(1) == "v") destination = 1;	//vram
				if (tokens.at(1) == "c") destination = 2;	//cram
				command = tokens.at(2);
			}
			
			auto pos = command.find('-');		//find the first '-' symbol
			
			if (pos == string::npos) {		// if there is no '-' in the command it's a single address
				int addr = GetAddrFromStr(command);

				if (addr == -1) {	// an error occured in the function
					cout << "address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
					goto end;
				}

				byte result;
				switch(destination) {
					case 0:
						result = _cpu->ReadMemory(addr);
						break;
					case 1:
						result = _cpu->ReadVRAM(addr);
						break;
					case 2:
						result = _cpu->ReadCRAM(addr);
						break;
					default:
						throw std::exception();
				}
				cout << ByteToHex(result);
			} else {	//we have a '-' which means two addresses
				int addr1, addr2;
				string addr1str, addr2str;
				addr1str = command.substr(0, (command.length() - pos - 1));		//keep only the first  value
				addr2str = command.substr(pos + 1);	//keep only the second value

				//get address
				addr1 = GetAddrFromStr(addr1str);
				addr2 = GetAddrFromStr(addr2str);
				if (addr1 == -1) {
					cout << "first address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
					goto end;
				}
				if (addr2 == -1) {
					cout << "second address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
					goto end;
				}

				//output all values
				//this way we can write forward and backwards. useful bc the stack grows downwards
				byte current;
				if (addr1 <= addr2) {
					for (int addr = addr1; addr <= addr2; addr++) {
						switch(destination) {	//haha yes O P T I M I S A T I O N
							case 0:
								current = _cpu->ReadMemory(addr);
								break;
							case 1:
								current = _cpu->ReadVRAM(addr);
								break;
							case 2:
								current = _cpu->ReadCRAM(addr);
								break;
							default:
								throw std::exception();
						}
						cout << ByteToHex(current) << ' ';
					}
				} else {
					for (int addr = addr1; addr >= addr2; addr--) {
						switch(destination) {
							case 0:
								current = _cpu->ReadMemory(addr);
								break;
							case 1:
								current = _cpu->ReadVRAM(addr);
								break;
							case 2:
								current = _cpu->ReadCRAM(addr);
								break;
							default:
								throw std::exception();
						}
						cout << ByteToHex(current) << ' ';
					}
				}
			}
			
		}

		//write to memory
		else if (tokens.at(0) == "w") {

			int destination = -69;
			string addrstr;
			string  valstr;
			if (tokens.size() == 3) {
				destination = 0;
				addrstr = tokens.at(1);
				valstr  = tokens.at(2);
			} else if (tokens.size() == 4) {
				addrstr = tokens.at(2);
				valstr  = tokens.at(3);
				if (tokens.at(1) == "v") destination = 1;	//vram
				if (tokens.at(1) == "c") destination = 2;	//cram
			} else {
				throw;
			}

			//here we calculate the address the user asked
			int addr = GetAddrFromStr(addrstr);

			//here we find the value they want to write
			auto val = GetByteFromStr(valstr);

			//if we reached here we're good to go
			WriteMemory(_cpu, addr, val, destination);	//this function ignores the write buffer
			
			goto updateDebugInfo;
		}

		//register loader
		else if (tokens.at(0) == "reg") {
			
			auto name = tokens.at(1);				//get name of register

			//oversimplified toupper function
			for(int i = 0; i < name.length(); i++) {
				if (name[i] >= 97) name[i] -= 32;
			}

			if (name == "HL") {
				//it's the only 16bit register
				int val = GetAddrFromStr(tokens.at(2));
				if (val == -1) {
					cout << "invalid: expected a number";
					goto end;
				}

				//assign to HL
				_cpu->HL = val;
			}
			else {

				//this is for 8bit registers
				int val = GetByteFromStr(tokens.at(2));

				//assign the value to the register
					if (name == "A") _cpu->A = val;
				else if (name == "B") _cpu->B = val;
				else if (name == "C") _cpu->C = val;
				else if (name == "D") _cpu->D = val;
				else if (name == "F") _cpu->F = val;
				else if (name == "SP") _cpu->SP = val;
				else {
					//if this happens we don't want to update bc it clears the screen
					cout << "invalid register '" << name << "'";
					goto end;
				}
			}

			//if no errors, update info
			goto updateDebugInfo;
		}

		//breakpoint
		else if (tokens.at(0) == "b") {

			breakpointMode mode = execute;

			bool _delete = false;		//are we trying to delete a breakpoint?

			string modetoken;		//the token containing the mode (if any)
			string addrtoken;		//the token containing the address
			//this way the order does not matter
			if (tokens.at(1).at(0) == '-') {			//we have a mode option as the first parameter
				modetoken = tokens.at(1);
				if (tokens.size() > 2)
					addrtoken = tokens.at(2);
			}
			else if (tokens.size() > 2) {
				if (tokens.at(2).at(0) == '-') {		//we have a mode option as the second parameter
					modetoken = tokens.at(2);
					addrtoken = tokens.at(1);
				}
			} else {
				modetoken = "-e";			//use the default
				addrtoken = tokens.at(1);
			}

			switch (modetoken.at(1)) {
				case 'e':		//execute. it's the default anyways
					break;
				case 'w':
					mode = write;
					break;
				case 'r':
					mode = read;
					break;
				case 'd':
					if (modetoken.length() > 2) {	//we don't want an indexoutofrange
						if (modetoken.at(2) == 'a') {	//check if it says "-da"
							breakpoints.clear();
							cout << "cleared all breakpoints";
							goto end;
						}
					}
					_delete = true;
					break;
				case 'l':
					foreach(bp, breakpoints) {
						cout << WordToHex(bp.address) << " (" << BPModeToString(bp.mode) << ")\n";
					}
					goto end;
				default:
					cout << "invalid option for break point mode. type 'help' for a list\n";
					goto end;
			}

			int addr = GetAddrFromStr(addrtoken);
			if (addr == -1) {
				cout << "address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
				goto end;
			}

			if (_delete == true) {
				//find if there is an element with that address

				//lists in c++ are weird
				list<BreakPoint>::iterator i = breakpoints.begin();
				
				foreach(bp, breakpoints) {
					//we only check for the address bc we delete based on address only
					//we found our element
					if (bp.address == addr) {
						breakpoints.erase(i);	//remove element
						cout << "deleted one breakpoint at 0x" << WordToHex(addr);
						goto end;	//jump to avoid problems, as we'll print an error at the end of the loop if the element is not found
					}

					advance(i, 1);		//increment i by one
				}
				//if we reach here we didn't find the element
				cout << "no breakpoint found with address 0x" << WordToHex(addr);
			}
			else	//we want to add a breakpoint
			{
				//first we check if it exists already
				//we compare both address and mode bc we can add multiple breakpoints at the same address but w different modes
				foreach(bp, breakpoints) {
					if (bp.address == addr && bp.mode == mode) {
						cout << "there already is a breakpoint at 0x" << WordToHex(addr) << " with mode " << BPModeToString(mode);
						goto end;	//jump to end
					}
				}

				//set properties
				BreakPoint bp;
				bp.mode = mode;
				bp.address = addr;

				breakpoints.push_back(bp);	//add to the list

				//write a confirmation message
				cout << "added breakpoint at 0x" << WordToHex(addr) << " in " << BPModeToString(mode) << " mode";
			}
		}

		//disassembler
		else if (tokens.at(0) == "dis") {
			if (tokens.size() != 2) throw std::out_of_range("");
			int addr;

			addr = GetAddrFromStr(tokens.at(1));
			if (addr == -1) {
				std::cout << "addr out of range\n";
				goto end;
			}

			string disassembly = codeToMnemonic(_cpu->ReadMemory(addr));

			//we can't use the regular "addImmediateToDisassembly" so here's a modified one
			{
				int pos = disassembly.find('&');
				while (pos != string::npos) {			//repeat until there are no '&' left
					
					//split the string in two so that the '&' is left out
					string part1 = disassembly.substr(0, pos);
					string part2 = disassembly.substr(pos + 1);

					//it's a 16bit value so read next two bytes
					byte b1 = _cpu->ReadMemory(addr + 1),		// low
				 		 b2 = _cpu->ReadMemory(addr + 2);		// high
					//the computer is little endian so we have to do b2 before b1
					string addressStr = "0x" + ByteToHex(b2) + ByteToHex(b1);

					//now the string has the address where it should be
					disassembly = part1 + addressStr + part2;

					pos = disassembly.find('&');
				}
				//then check for 8-bit
				pos = disassembly.find('%');
				while (pos != string::npos) {			//repeat until there are no '%' left
					
					//split the string in two so that the '%' is left out
					string part1 = disassembly.substr(0, pos);
					string part2 = disassembly.substr(pos + 1);

					//it's an 8bit value so read next byte
					byte b1 = _cpu->ReadMemory(addr + 1);
					
					string addressStr = "0x" + ByteToHex(b1);

					//now the string has the address where it should be
					disassembly = part1 + addressStr + part2;

					pos = disassembly.find('%');
				}
			}

			std::cout << ByteToHex(_cpu->ReadMemory(addr)) << " => " << disassembly << "\n";
		}

		//load state
		else if (tokens.at(0) == "ls") {
			//get first digit from char
			int num;
			//it's in a try because stoi can cause exceptions if the value received is bad
			try
			{
				switch (stoi(tokens.at(1))) {
										case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						num = stoi(tokens.at(1));
						break;
					//we only accept values 0-9
					default:
						throw exception();
				}
			}
			catch(exception) {
				cout << "invalid number for state. expected a 0-9 value";
				goto end;
			}

			if (LoadState(_cpu, num) == 0) {
				cout << "loaded state";
				dodebug = true;
				goto updateDebugInfo;
			} else {
				cout << "could not load state";
			}
		}
		//save state
		else if (tokens.at(0) == "ss") {

			int num;
			//it's in a try because stoi can cause exceptions if the value received is bad
			try
			{
				switch (stoi(tokens.at(1))) {
										case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						num = stoi(tokens.at(1));
						break;
					//we only accept values 0-9
					default:
						throw exception();
				}
			}
			catch(exception) {
				cout << "invalid number for state. expected a 0-9 value";
				goto end;
			}

			if (SaveState(_cpu, num) == 0) {
				cout << "saved state";
			} else {
				cout << "could not save state";
			}
		}

		//jump
		else if (tokens.at(0) == "jp") {
			
			int addr = GetAddrFromStr(tokens.at(1));
			if (addr == -1) {
				cout << "invalid address";
				goto end;
			}

			_cpu->PC = addr;
			dodebug = true;
			goto updateDebugInfo;
		}

		//update screen
		else if (command == "update") {
			updatescreen(_cpu);
			std::cout << "updated screen!\n";
		}

		//clear console
		else if (tokens.at(0) == "cls") {

			goto updateDebugInfo;	//calling this will first clear the screen, then print debug info again so the console is nice and clear
		}

		//run
		else if (tokens.at(0) == "run") { dodebug = false; cls(); return; }

		//reset
		else if (tokens.at(0) == "reset") {
			//reset is just a fancy load state

			LoadState(_cpu, -1);		//the state -1 contains the machine at the beginning

			dodebug = true;

			goto updateDebugInfo;		//returning would also step once
		}

		else if (command == "69" || command == "420") cout << "nice";
		else if (command == "69420") cout << "nice++";

		//something we can't execute
		else {
		unknown_command:
			cout << "unknown command. write 'help' for a list of commands";
		}
	}
	catch(...) {
		cout << "invalid command parameters";
	}

	end:
	cout << endl;   		//flush the cout and print a new line
	goto askcommand;		//could just use a while(true) but this looks cooler
}


// -1 is a special save state used by the reset command
int LoadState(CPU_T* _cpu, int num) {
	try
	{
		if (num < -1 || num > 9) throw exception(); //error out of range

		string filename;
		if (num != -1) {
			filename = ssFolder + "state.ss";
			filename += to_string(num);
		} else filename = ssFolder + "reset.ss";
		
		FILE* f;
		//try to open file, raise exception if it does not exist
		if (!(f = fopen(filename.c_str(), "rb"))) {
			throw exception();
		}
		fseek(f, 0, SEEK_SET);					//place at beginning

		//get the ram
		fread(_cpu->base, 1, 0x10000, f);

		//get vram
		fread(_cpu->vramBase, 1, 0x10000, f);
		
		//get cram
		fread(_cpu->cramBase, 1, 0x100, f);
		
		//get registers
		fread(&_cpu->A,  1, sizeof(byte), f);
		fread(&_cpu->B,  1, sizeof(byte), f);
		fread(&_cpu->C,  1, sizeof(byte), f);
		fread(&_cpu->D,  1, sizeof(byte), f);
		fread(&_cpu->F,  1, sizeof(byte), f);
		fread(&_cpu->HL, 1, sizeof(word), f);
		fread(&_cpu->SP, 1, sizeof(byte), f);
		fread(&_cpu->PC, 1, sizeof(word), f);

		//get debugger stuff
		fread(&_cpu->cyclecounter, 1, sizeof(_cpu->cyclecounter), f);

		//screen
		LoadScreen(filename);

		fclose(f);
		return 0;
	}
	catch (exception) { return -1; }
}
// -1 is a special save state used by the reset command
int SaveState(CPU_T* _cpu, int num) {
	try {
		if (num < -1 || num > 9) throw exception();

		string filename;
		if (num != -1) {
			filename = ssFolder + "state.ss";
			filename += to_string(num);
		} else filename = ssFolder + "reset.ss";

		FILE* f = fopen(filename.c_str(), "wb");		//get file
		fseek(f, 0, SEEK_SET);					//place at beginning

		//get the ram
		fwrite(_cpu->base, 1, 0x10000, f);

		//get vram
		fwrite(_cpu->vramBase, 1, 0x10000, f);
		
		//get cram
		fwrite(_cpu->cramBase, 1, 0x100, f);

		//get registers
		fwrite(&_cpu->A,  1, sizeof(byte), f);
		fwrite(&_cpu->B,  1, sizeof(byte), f);
		fwrite(&_cpu->C,  1, sizeof(byte), f);
		fwrite(&_cpu->D,  1, sizeof(byte), f);
		fwrite(&_cpu->F,  1, sizeof(byte), f);
		fwrite(&_cpu->HL, 1, sizeof(word), f);
		fwrite(&_cpu->SP, 1, sizeof(byte), f);
		fwrite(&_cpu->PC, 1, sizeof(word), f);

		//debugger stuff
		fwrite(&_cpu->cyclecounter, 1, sizeof(_cpu->cyclecounter), f);

		//screen
		SaveScreen(filename);

		fclose(f);
		return 0;
	}
	catch (exception) { return -1; }
}


void OptionsMenu(vector<string> command) {

	if (command.size() < 3) {
		cout << "invalid: expected parameters for 'set' command";
		return;
	}

	auto optionname = command.at(1);
	auto optionval  = command.at(2);

	//here we set the options
	if (optionname == "placeholder") {
		if (optionval == "placeholder") {
			
		}
		else goto unknownValue;
	}
	else {
		cout << "unknown option. type 'options' for a list of all options";
		return;
	}

	return;        //if we dont return it prints the bad value message

	unknownValue:	//reached when we have an invalid value request for a KNOWN option
	cout << "invalid value '" << optionval << "' for option '" << optionname << "'";
}