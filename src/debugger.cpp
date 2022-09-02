#include "debugger.h"
#include "util.h"

//it's called "Irresponsible Memory Management" for a reason
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>		//to check/create directories
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


//todo: better breakpoint checking, fare cose per gli altri due tipi di ram, call stack backtrace


void PrintDebug(string msg) {
	cout << "DEBUG: " << msg << endl;
}


//possible return values of the command
enum command_return_t { crt_none, crt_return, crt_continue_execution };
typedef command_return_t (*command_function_t)(CPU_T* cpu, vector<string> params);

#define debugger_func(name) command_return_t name(CPU_T* cpu, vector<string> params)
#define cmd_alias(name) {name, (debugger_command_t*)1, NULL, "", "", "" }

struct debugger_command_t {

	//command to write
	string name;

	// //we will first check if this is not null. if it isn't, we will ignore the other two as this is an alternative for another command, we will execute the parent command
	// debugger_command_t* parent_command = NULL;
	
	//function to execute
	command_function_t command_function;

	//for the help menu. short is for general, long is for specific
	string description_short, description_long;
	string parameter_description;

	bool operator==(debugger_command_t sc) {
		return name == sc.name;
	}

};

//command functions declaration
debugger_func(cmd_help);
debugger_func(cmd_quit);
debugger_func(cmd_cc);
debugger_func(cmd_step);
debugger_func(cmd_next);
debugger_func(cmd_finish);
debugger_func(cmd_memview);
debugger_func(cmd_memwrite);
debugger_func(cmd_reg_write);
debugger_func(cmd_jump);
debugger_func(cmd_breakpoint);
debugger_func(cmd_breakpoint_delete);
debugger_func(cmd_breakpoint_list);
debugger_func(cmd_disassembly);
debugger_func(cmd_loadstate);
debugger_func(cmd_savestate);
debugger_func(cmd_reset);
debugger_func(cmd_load);
debugger_func(cmd_update);
debugger_func(cmd_cls);
debugger_func(cmd_run);
debugger_func(cmd_shell);

//prints a space between the previous command and the next
#define help_msg_separator {"\n", nullptr, "", "", ""},
//all commands
debugger_command_t commands[] = {

	{"help", cmd_help, "list all commands, or describe a specific one", "", "[command]"},
	{"quit", cmd_quit, "exit the emulator", "", "/ exit"},
	{"exit", cmd_quit, "", "", ""},
	{"shell", cmd_shell, "execute a command in the host shell", "", "[command]"},
	help_msg_separator

	{"cc", cmd_cc, "print the value of the clock counter or reset it", "print the value of the clock counter. adding \"reset\" will also reset it to zero", "[reset]"},
	help_msg_separator

	{"step", cmd_step, "step to the next instruction, going into function calls", "", "/ s"},
	{"s", cmd_step, "", ""},
	{"next", cmd_next, "step to the next instruction, skipping over function calls", "", "/ n"},
	{"n", cmd_next, "", ""},
	{"finish", cmd_finish, "run until the end of the function", "", "/ f"},
	{"f", cmd_finish, "", ""},
	help_msg_separator

	{"m", cmd_memview, "print the memory value at ADDR", "print the value of the memory at the given ADDRess. specifying 'r', 'v' or 'c' as parameters will select which memory to look at (RAM, VRAM or CRAM)", "[r/v/c] ADDR"},
	{"m", cmd_memview, "print the memory values from ADDR1 to ADDR2", "print all the values in the given memory interval. specifying 'r', 'v' or 'c' as parameters will select which memory to look at (RAM, VRAM or CRAM)", "[r/v/c] ADDR"},
	help_msg_separator

	{"dis", cmd_disassembly, "disassembles the byte at the given ADDRess" },
	help_msg_separator

	{"w", cmd_memwrite, "write VAL to ADDR", "write the given VALue to the specified ADDRess. you can specify 'r', 'v' or 'c' to write to RAM, VRAM or Cram", "[r/v/c] ADDR VAL"},
	help_msg_separator

	{"reg", cmd_reg_write, "load VAL in the given register", "load the given VALue in any of the REGisters. the registers are:\ngeneral purpose, 8bit:\n\tA, B, C, D, H, L\ngeneral purpose, 16bit:\n\tHL\nSP (stack pointer, 8bit)\nF  (flags, 8bit)", "NAME VAL"},
	{"jp", cmd_jump, "set the Program Counter to the given ADDRess", "", "ADDR"},
	help_msg_separator

	{"break" , cmd_breakpoint, "add a breakpoint at the given ADDRess", "add a breakpoint at the given ADDRess.\nyou can specify a breakpoint mode:\n\t-e: the default mode, will trigger on execution\n\t-w: will trigger on write\n\t-r: will trigger on read\n\nyou can also specify 'r', 'v' or 'c' to add the breakpoint in RAM, VRAM or CRAM", "[r/v/c] [mode] ADDR"},
	{"remove", cmd_breakpoint_delete, "remove the breakpoint at the specified ADDRess", "remove all breakpoints at the specified ADDRess, or specify a mode to remove only those.\nyou can specify 'r' if it's in RAM (default), 'v' if the breakpoint is in VRAM or 'c' if it's in CRAM.\nwriting 'all' in place of the address will remove all breakpoints, at all addresses", "[r/v/c] [mode] ADDR"},
	{"list",   cmd_breakpoint_list, "list all existing breakpoints", "shows a list of all existing breakpoints", ""},
	help_msg_separator

	{"ss", cmd_savestate, "save a state", "save the state with the given NUMber. they are numbered with the digits from 0 to 9", "NUM"},
	{"ss", cmd_loadstate, "load a state", "load the state with the given NUMber. they are numbered with the digits from 0 to 9", "NUM"},
	{"reset", cmd_reset, "reset the CPU", "reset the CPU. this will return it to the initial state of the last time you've loaded a ROM, or if you haven't to when you first opened the emulator", ""},
	help_msg_separator

	{"load", cmd_load, "load a binary file to RAM"},
	help_msg_separator

	{"cls", cmd_cls, "clear the console", "", ""},
	{"update", cmd_update, "force a screen update", "manually update the display. this will not influence the normal screen update cycle", ""},
	help_msg_separator

	{"run", cmd_run, "exit the debugger and run until the next breakpoint", "", ""},
};
debugger_command_t GetDebuggerCommand(string name) {

	foreach (command, commands) {
		//TODO: make aliases work
		// while (command.parent_command != NULL) {
		// 	command = *command.parent_command;
		// }

		if (command.name == name) {
			return command;
		}
	}

	return {""};//null command. we'll check if the name is not zero
}
int MAX_CMD_LEN = -1;


// when we press enter without a command this will be repeated
string lastcommand = "step";

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
struct BreakPoint {
	word address;
	breakpointMode mode;
};

list<BreakPoint> breakpoints;
list<BreakPoint> breakpoints_cram;
list<BreakPoint> breakpoints_vram;


const string ssFolder = "saveStates/";
int LoadState(CPU_T* _cpu, int num);
int SaveState(CPU_T* _cpu, int num);


void Debugger::DebugInit(CPU_T* CPU) {
    
	//set window title
	std::string command;
	#ifdef _WIN32
	command = "title " + WINDOW_TITLE;
	#elif __linux__
	command = "echo -ne \"\\033]0;" + WINDOW_TITLE + "\\007\"";
	#endif
	system(command.c_str());
	cls();
	
	//used when writing help message
	foreach (command, commands) {
		int currentlen = (command.name + command.parameter_description).length();
		if (currentlen > MAX_CMD_LEN)
			MAX_CMD_LEN = currentlen;
	}

	//if the folder doesn't exist, create it
	namespace fs = std::filesystem;
	if (!fs::is_directory("saveStates") || !fs::exists("saveStates")) {
		fs::create_directory("saveStates");
	}

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
bool Debugger::IsBreakpointVRAM(word address, breakpointMode mode) {

	//check the actual breakpoints
	foreach (bp, breakpoints_vram) {
		if (bp.address == address && bp.mode == mode) return true;
	}

	return false;
}
bool Debugger::IsBreakpointCRAM(word address, breakpointMode mode) {

	//check the actual breakpoints
	foreach (bp, breakpoints_cram) {
		if (bp.address == address && bp.mode == mode) return true;
	}

	return false;
}

string BPModeToString(breakpointMode mode) {
	//there is no way of converting enums to string so we do this
	switch(mode) {
		case (execute):
			return "execute";
		case (write):
			return "write";
		case (read):
			return "read";
	}
}


//prints the option menu. it's defined at the bottom
void OptionsMenu(vector<string> command);


//this way we can ignore the buffer
void WriteMemory_ignorebuffer(CPU_T* CPU, word address, byte value, int destination) {
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


//the 'CodeToMnemonic' function has a '&' symbol where a 16-bit value is needed, and a '%' where an 8-bit one is.
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

void UpdateDebugInfo(CPU_T* _cpu) {

	cls();

	word pc = _cpu->PC;

	auto opcode = _cpu->ReadMemory(pc);

	auto instDisassembly = CodeToMnemonic(opcode);
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
}

void Debugger::printdebug(CPU_T* _cpu) {

	//do it before anything as we don't want to repeat it in the debugger

	UpdateDebugInfo(_cpu);

	askcommand:
	string command;
	cin.clear();
	getline(cin, command);

	//continue to next instruction
	if (command == "") command = lastcommand;

	// std::cout << ":" << command << ":" << std::endl;
	
	auto tokens = splitString(command);

	// PrintDebug("going into tryexcept");

	try
	{
		// PrintDebug("finding command...");
		//find the command to execute
		command_function_t function = GetDebuggerCommand(tokens.at(0)).command_function;

		// PrintDebug("about to execute...");
		command_return_t command_retval;
		//execute it, or a null command
		if (function != NULL) {

			// PrintDebug("function is not null...");
			command_retval = function(_cpu, tokens);

		} else {

			// PrintDebug("function is null...");
			command_retval = crt_none;
			cout << "unknown command. write 'help' for a list of commands";
		}

		// PrintDebug("managing retval...");
		switch (command_retval) {
			//nothing
			case crt_none:
				break;
			//exit and continue execution
			case crt_continue_execution:
				dodebug = false;
				cls();
				return;
			//return
			case crt_return:
				return;
		}

	}
	catch(...) {
		cout << "invalid command parameters";
	}

	end:
	cout << endl;   		//flush the cout and print a new line
	goto askcommand;		//could just use a while(true) but this looks cooler (and is an excuse to remove a layer of indentation)
}


// -1 is a special save state used by the reset command
int LoadState(CPU_T* _cpu, int num) {
	try
	{
		if (num < -1 || num > 9) throw; //error out of range

		string filename;
		if (num != -1) {
			filename = ssFolder + "state.ss";
			filename += to_string(num);
		} else filename = ssFolder + "reset.ss";
		
		FILE* f;
		//try to open file, raise exception if it does not exist
		if (!(f = fopen(filename.c_str(), "rb"))) {
			throw;
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
		if (num < -1 || num > 9) throw;

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



debugger_func(cmd_help) {

	//general help
	if (params.size() == 1) {

		cout << "\n\n"

			// TODO: "step-like" is not really intuitive as a description
			<< "pressing enter will repeat the last step-like instruction" << "\n"
			<< "press esc while running to start debugging" << "\n\n"

			<< "commands:" << "\n";

		foreach (command, commands) {
			if (command.description_short != "") {
				
				cout << command.name << " " << command.parameter_description << ": ";

				//pad spaces
				for (int i = (command.name + command.parameter_description).length(); i < MAX_CMD_LEN; i++) {
					cout << " ";
				}

				cout << command.description_short << "\n";
			} else if (command.command_function == nullptr) {
				cout << "\n";
			}
		}
	
	//specific help
	} else {

		auto command = GetDebuggerCommand(params.at(1));
		if (command.name == "") {

			cout << "unknown command \"" << params.at(1) << "\". type \"help\" for a list of commands";

		} else {

			//we already get the top parent command, not the aliases, so we don't worry about those

			string descToPrint;
			if (command.description_long == "")
				descToPrint = command.description_short;
			else
				descToPrint = command.description_long;

			cout << command.name << " " << command.parameter_description << ": " << descToPrint;
		}

	}

	cout << "\n\n\n";

	return crt_none;
}
debugger_func(cmd_quit) {
	exit(EXIT_SUCCESS);
}

debugger_func(cmd_cc) {

	std::cout << cpu->cyclecounter << "\n";

	if (params.size() == 2 && params.at(1) == "reset")
		cpu->cyclecounter = 0;

	return crt_none;
}

debugger_func(cmd_step) {

	lastcommand = "step";
	return crt_return;
}
debugger_func(cmd_next) {

	lastcommand = "next";

	doBreakDepth = true;
	breakAtDepth = callDepth;	//we want to break at the next instruction, so same level. this way we can also skip funcs

	return crt_continue_execution;
}
debugger_func(cmd_finish) {

	lastcommand = "finish";

	doBreakDepth = true;
	breakAtDepth = callDepth - 1;	//we want to break when we get out of the function, so one level higher
	
	return crt_continue_execution;
}

debugger_func(cmd_memview) {

	if (params.size() < 2) {
		cout << "invalid parameter number for 'm'";
		return crt_none;
	}

	int destination = -1;	//0 is normal ram, 1 is vram, 2 is cram

	string address;

	if (params.size() == 2) {	//no specified destination, so second element is address
		destination = 0;
		address = params.at(1);
	} else if (params.size() == 3) {	//address is third element bc destination is second
		if (params.at(1) == "v") destination = 1;	//vram
		if (params.at(1) == "c") destination = 2;	//cram
		address = params.at(2);
	}
	
	auto pos = address.find('-');		//find the first '-' symbol
	
	if (pos == string::npos) {		// if there is no '-' in the command it's a single address
		int addr = GetAddrFromStr(address);

		if (addr == -1) {	// an error occured in the function
			cout << "address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
			return crt_none;
		}

		byte result;
		switch(destination) {
			case 0:
				result = cpu->ReadMemory(addr);
				break;
			case 1:
				result = cpu->ReadVRAM(addr);
				break;
			case 2:
				result = cpu->ReadCRAM(addr);
				break;
			default:
				throw;
		}
		cout << ByteToHex(result);
	} else {	//we have a '-' which means two addresses
		int addr1, addr2;
		string addr1str, addr2str;
		addr1str = address.substr(0, (address.length() - pos - 1));		//keep only the first  value
		addr2str = address.substr(pos + 1);	//keep only the second value

		//get address
		addr1 = GetAddrFromStr(addr1str);
		addr2 = GetAddrFromStr(addr2str);
		if (addr1 == -1) {
			cout << "first address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
			return crt_none;
		}
		if (addr2 == -1) {
			cout << "second address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
			return crt_none;
		}

		//output all values
		//this way we can write forward and backwards. useful bc the stack grows downwards
		byte current;
		if (addr1 <= addr2) {
			for (int addr = addr1; addr <= addr2; addr++) {
				switch(destination) {	//haha yes O P T I M I S A T I O N
					case 0:
						current = cpu->ReadMemory(addr);
						break;
					case 1:
						current = cpu->ReadVRAM(addr);
						break;
					case 2:
						current = cpu->ReadCRAM(addr);
						break;
					default:
						throw;
				}
				cout << ByteToHex(current) << ' ';
			}
		} else {
			for (int addr = addr1; addr >= addr2; addr--) {
				switch(destination) {
					case 0:
						current = cpu->ReadMemory(addr);
						break;
					case 1:
						current = cpu->ReadVRAM(addr);
						break;
					case 2:
						current = cpu->ReadCRAM(addr);
						break;
					default:
						throw;
				}
				cout << ByteToHex(current) << ' ';
			}
		}
	}

	return crt_none;
}
debugger_func(cmd_memwrite) {

	int destination = -69;
	string addrstr;
	string  valstr;
	if (params.size() == 3) {

		destination = 0;
		addrstr = params.at(1);
		valstr  = params.at(2);

	} else if (params.size() == 4) {

		addrstr = params.at(2);
		valstr  = params.at(3);
		if (params.at(1) == "v") destination = 1;	//vram
		if (params.at(1) == "c") destination = 2;	//cram

	} else {
		throw;
	}

	//here we calculate the address the user asked
	int addr = GetAddrFromStr(addrstr);

	//here we find the value they want to write
	auto val = GetByteFromStr(valstr);

	//if we reached here we're good to go
	WriteMemory_ignorebuffer(cpu, addr, val, destination);
	
	UpdateDebugInfo(cpu);

	return crt_none;
}

debugger_func(cmd_reg_write) {

	auto name = params.at(1);				//get name of register

	//oversimplified toupper function
	for(int i = 0; i < name.length(); i++) {
		if (name[i] >= 97) name[i] -= 32;
	}

	if (name == "HL") {
		//it's the only 16bit register
		int val = GetAddrFromStr(params.at(2));
		if (val == -1) {
			cout << "invalid: expected a number";
			return crt_none;
		}

		//assign to HL
		cpu->HL = val;
	}
	else {

		//this is for 8bit registers
		int val = GetByteFromStr(params.at(2));

		//assign the value to the register
			 if (name == "A") cpu->A = val;
		else if (name == "B") cpu->B = val;
		else if (name == "C") cpu->C = val;
		else if (name == "D") cpu->D = val;
		else if (name == "F") cpu->F = val;
		else if (name == "SP") cpu->SP = val;
		else if (name == "H") {
			//only set the high byte of HL
			cpu->HL &= 0x00FF;
			cpu->HL |= val << 8;
		}
		else if (name == "L") {
			//only set the low byte of HL
			cpu->HL &= 0xFF00;
			cpu->HL |= val;
		}
		else {
			//if this happens we don't want to update bc it clears the screen
			cout << "invalid register '" << name << "'";
			return crt_none;
		}
	}

	//if no errors, update info
	UpdateDebugInfo(cpu);

	return crt_none;
}
debugger_func(cmd_jump) {

	int addr = GetAddrFromStr(params.at(1));
	if (addr == -1) {
		cout << "invalid address";
		return crt_none;
	}

	cpu->PC = addr;
	dodebug = true;
	UpdateDebugInfo(cpu);

	return crt_none;
}

debugger_func(cmd_breakpoint) {

	breakpointMode mode = execute;

	string modetoken;		//the token containing the mode (if any)
	string addrtoken;		//the token containing the address
	//this way the order does not matter
	if (params.at(1).at(0) == '-') {			//we have a mode option as the first parameter
		modetoken = params.at(1);
		addrtoken = params.at(2);
	}
	else if (params.size() > 2) {
		if (params.at(2).at(0) == '-') {		//we have a mode option as the second parameter
			modetoken = params.at(2);
			addrtoken = params.at(1);
		}
	} else {
		modetoken = "-e";			//use the default
		addrtoken = params.at(1);
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
		default:
			cout << "invalid option for break point mode. type 'help break' for a list\n";
			return crt_none;
	}

	int addr = GetAddrFromStr(addrtoken);
	if (addr == -1) {
		cout << "address out of range (min: " << ADDRESS_MIN << ", max: " << ADDRESS_MAX << ")";
		return crt_none;
	}

	//first we check if it exists already
	//we compare both address and mode bc we can add multiple breakpoints at the same address but w different modes
	list<BreakPoint>* ls = nullptr;
	//TODO: add the breakpoint checking the destination
	
	foreach(bp, breakpoints) {
		if (bp.address == addr && bp.mode == mode) {
			cout << "there already is a breakpoint at 0x" << WordToHex(addr) << " with mode " << BPModeToString(mode);
			return crt_none;
		}
	}

	//set properties
	BreakPoint bp;
	bp.mode = mode;
	bp.address = addr;

	breakpoints.push_back(bp);	//add to the list

	//write a confirmation message
	cout << "added breakpoint at 0x" << WordToHex(addr) << " in " << BPModeToString(mode) << " mode";

	return crt_none;
}
debugger_func(cmd_breakpoint_delete) {

	//what we'll loop over
	list<BreakPoint> bps;
	breakpointMode mode = execute;
	int address;

	string areatoken = "";		//which memory to target, if any
	string modetoken = "";		//the token containing the mode (if any)
	string addrtoken = "";		//the token containing the address

	switch (params.size() - 1) {

		//no parameters is all of them
		case 0:
			breakpoints.clear();
			breakpoints_vram.clear();
			breakpoints_cram.clear();
			cout << "cleared all breakpoints";
			return crt_none;
		//do this way so we fall through
		default:
		case 3:
			areatoken = params.at(3);
		case 2:
			modetoken = params.at(2);
		case 1:
			addrtoken = params.at(1);
			break;
	}

	switch (areatoken.at(0)) {
		case 'r':
			bps = breakpoints;
			break;
		case 'v':
			bps = breakpoints_vram;
			break;
		case 'c':
			bps = breakpoints_cram;
			break;
		default:
			//there is no specification so all of them
			bps.insert(bps.end(), breakpoints.begin(), breakpoints.end());
			bps.insert(bps.end(), breakpoints_vram.begin(), breakpoints_vram.end());
			bps.insert(bps.end(), breakpoints_cram.begin(), breakpoints_cram.end());
			break;
	}

	//means all of them, we'll say that -1 means all
	if (modetoken == "") mode = (breakpointMode)(-1);
	else {
		switch (modetoken.at(1)) {
			case 'e':
				mode = execute;
				break;
			case 'w':
				mode = write;
				break;
			case 'r':
				mode = read;
				break;
			default:
				cout << "invalid breakpoint mode: \"" << modetoken << "\"";
				break;
		}
	}

	address = GetAddrFromStr(addrtoken);

	//how many breakpoints we've removed
	int removedCount = 0;
	list<BreakPoint>::iterator i = breakpoints.begin();
				
	foreach(bp, bps) {
		//we only check for the address bc we delete based on address only
		//we found our element
		if (bp.address == address) {
			if ((mode == (breakpointMode)(-1)) || (mode == bp.mode)) {
				removedCount++;
				breakpoints.erase(i);	//remove element
			}
		}

		advance(i, 1);		//increment i by one
	}

	if (removedCount == 0) {
		cout << "no breakpoint was found";
	} else {
		cout << removedCount << " breakpoints removed";
	}

	return crt_none;
}
debugger_func(cmd_breakpoint_list) {

	//i don't need to do it this way but regular compares are boring, this makes me feel like i know what i'm doing
	if (!(breakpoints.size() || breakpoints_vram.size() || breakpoints_cram.size())) {
		cout << "the list of breakpoints is empty";
	}

	//all the ones in normal RAM
	if (breakpoints.size() > 0) {
		cout << "RAM:\n";
		foreach (breakpoint, breakpoints) {
			cout << "\t" << WordToHex(breakpoint.address) << " (" << BPModeToString(breakpoint.mode) << ")\n";
		}
	}

	cout << "\n";

	//all the ones in VRAM
	if (breakpoints_vram.size() > 0) {
		cout << "VRAM:\n";
		foreach (breakpoint, breakpoints_vram) {
			cout << "\t" << WordToHex(breakpoint.address) << " (" << BPModeToString(breakpoint.mode) << ")\n";
		}
	}

	cout << "\n";

	//all the ones in CRAM
	if (breakpoints_cram.size() > 0) {
		cout << "CRAM:\n";
		foreach (breakpoint, breakpoints_cram) {
			cout << "\t" << WordToHex(breakpoint.address) << " (" << BPModeToString(breakpoint.mode) << ")\n";
		}
	}

	return crt_none;
}

debugger_func(cmd_disassembly) {

	int addr;

	addr = GetAddrFromStr(params.at(1));
	if (addr == -1) {
		std::cout << "addr out of range\n";
		return crt_none;
	}

	string disassembly = CodeToMnemonic(cpu->ReadMemory(addr));

	//TODO: fix this because it's bad
	//we can't use the regular "addImmediateToDisassembly" so here's a modified one
	{
		int pos = disassembly.find('&');
		while (pos != string::npos) {			//repeat until there are no '&' left
	
			//split the string in two so that the '&' is left out
			string part1 = disassembly.substr(0, pos);
			string part2 = disassembly.substr(pos + 1);

			//it's a 16bit value so read next two bytes
			byte b1 = cpu->ReadMemory(addr + 1),		// low
					b2 = cpu->ReadMemory(addr + 2);		// high
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
			byte b1 = cpu->ReadMemory(addr + 1);
	
			string addressStr = "0x" + ByteToHex(b1);

			//now the string has the address where it should be
			disassembly = part1 + addressStr + part2;

			pos = disassembly.find('%');
		}
	}

	std::cout << ByteToHex(cpu->ReadMemory(addr)) << " => " << disassembly << "\n";

	return crt_none;
}

debugger_func(cmd_savestate) {

	int num;
	try
	{
		num = stoi(params.at(1));
		if (num > 9 || num < 0) throw;
	}
	catch(...) {
		cout << "invalid number for state. expected a 0-9 value";
		return crt_none;
	}

	if (SaveState(cpu, num) == 0) {
		cout << "saved state!";
	} else {
		cout << "could not save state";
	}

	return crt_none;
}
debugger_func(cmd_loadstate) {
	
	int num;
	//it's in a try because stoi can cause exceptions if the value received is bad
	try
	{
		num = stoi(params.at(1));
		if (num > 9 || num < 0) throw exception();
	}
	catch(exception) {
		cout << "invalid number for state. expected a 0-9 value";
		return crt_none;
	}

	if (LoadState(cpu, num) == 0) {
		dodebug = true;
		UpdateDebugInfo(cpu);
		cout << "succesfully loaded state!";
	} else {
		cout << "could not load state";
	}

	return crt_none;
}
debugger_func(cmd_reset) {

	//reset is just a fancy load state
	LoadState(cpu, -1);		//the state -1 contains the machine at the beginning
	dodebug = true;
	UpdateDebugInfo(cpu);		//returning would also step once

	cout << "correctly reset the CPU!";

	return crt_none;
}

debugger_func(cmd_load) {

	if (LoadProgramToMemory(params.at(1), cpu->base) == true) {
		cpu->reset();
		SaveState(cpu, -1);	//for the reset command
		UpdateDebugInfo(cpu);
		cout << "success!";
	} else {
		LoadFileErrorMsg();
	}

	return crt_none;
}

debugger_func(cmd_update) {

	updatescreen(cpu);
	std::cout << "updated screen!\n";
	return crt_none;
}
debugger_func(cmd_cls) {

	UpdateDebugInfo(cpu);

	return crt_none;
}

//it's all managed in the return thing
debugger_func(cmd_run) { return crt_continue_execution; }

debugger_func(cmd_shell) {

	std::string cmd;
	bool i = false;	//the first param is just the command, so skip that
	foreach (param, params) {
		if (i) {
			cmd += param + " ";
		} else {
			i = true;
		}
	}

	int res = system(cmd.c_str());

	if (res != 0) cout << "could not complete the operation";

	return crt_none;
}
