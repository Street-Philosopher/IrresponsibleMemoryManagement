# IrresponsibleMemoryManagement
An emulator I made in my free time, for a _very_ simple CPU I designed. If you want to use it don't expect anything too crazy.

## Specifications
The full specification is in [dd.ò](dd.ò).

A few important things:
* it has 64 kB of RAM, 64 kB of VRAM (used to determine what pixels to render) and 256 bytes of control RAM for the computer's settings (currently unused for the most part);
* has four 8bit and one 16bit general purpose registers;
* opcodes are one byte long, plus any parameters it may have;
* the display is 256x256, and can only show black and white. It doesn't have a set refresh rate, but refreshes every 65536 instructions executed;
* it has a built in [debugger](#debugger), based on console commands. To use it just press ESC while playing, and you can use it to do many cool things like reading/writing RAM, breakpoints, savestates etc.

## Assembler
The assembler is written in Python. Again, the full specification for the Assembly language is in the [dd.ò](dd.ò) file. It also supports comments (after a semicolon), defining constants (syntax: "#define NAME as VALUE"), including other files (' #include "FILENAME" '), the DB directive (values are comma separated, also supports strings and escape sequences), the ORG (writes bytes until the specified address. you can specify a byte or 'r' for 'random' as the second parameter, which is what bytes will be written) and PAD and PADTO (which changes the current addres by a value or until the specified value is reached respectively).
The first parameter it needs is the path to the main file to compile. Then the assembler will start to assemble it into a file, which will only be created if there are no syntax errors; if there are any, they'll be listed in the console.
There are also some other options you can specify after the file name:
* -np: stands for "No Pause". it means that any time the assembler would normally ask to press a key to continue (like at the end) will just be skipped.
* -kt: keeps all the temporary files created by the assembler that are normally deleted automatically (like intermediate files).
* -q: quiet mode. silences all logs and only prints errors.
* -org (ADDRESS): starts assembling from the given address (integer in the range 0x0000-0xFFFF). defaults to zero.
* -o (NAME): specifies the name of the output file. defaults to the name of the main file being assembled, with an appropriate file extension.
* -bin: specifies that the output mode is a .bin binary file. it's the default, so in practice it does nothing.
* -c: will output the program as an array in a .c file, as well as its length in a separate variable.

## Debugger
To access the debugger press ESC in the window. It will halt execution, and will open a terminal window where you can see information on the registers and memory (the memory shown is the one in front of the Program Counter, so the one that's about to be executed), and can execute commands.

You can single step by pressing enter without writing any command. Otherwise the commands are:
* help: lists all commands
* quit/exit: exits the application
* shell CMD: executes CMD in the host shell

* cc: displays a counter. this counter is increased every CPU cycle. can be reset with "cc reset"

* cls: clears the console screen
* update: forces a display update
* step: single steps to the next instruction (can also be written as simply "s"). After executing this, stepping by pressing enter will repeat this instruction.
* next: like step, but it will skip over function calls (can be written as "n"). After executing this, stepping by pressing enter will repeat this instruction.
* finish: will run until the program exits the current function (shortened as "f")

* m ADDR: view the memory at the given address. if the parameter "v" is put before the address it will be read from VRAM, if the parameter "c" it will be read from CRAM and if none it will be read from normal RAM. it also supports a range of addresses, if written in the form "ADDR1-ADDR2"
* dis ADDR: reads the memory value at addr, disassembling it into an instruction. only supports normal RAM
* w ADDR VAL: writes VAL to the given address ADDR. it also supports the "v" and "c" parameters, in the same position as before
* reg NAME VAL: loads the given VALue into the register specified
* jp ADDR: loads the given address into the program counter, simulating a jump
* b ADDR: this will add a breakpoint to the given address. it also supports the "v" and "c" parameters, put directly after the "b". it supports three modes: by default it's in execute mode (-e), but it can be read (-r) or write (-w). you can also list all breakpoints (b -l) or delete (specifying the -d mode will delete the breakpoint at that address, doing "b -da" will delete all). breakpoints will open the debugger when triggered
* ss NUM: saves the state with the given number (from 0 to 9)
* ls NUM: loads the state with the given number (from 0 to 9)
* load PATH: loads a file, whose binary contents will be loaded to RAM, and then will reset the CPU. this is basically just loading a ROM
* reset: resets the CPU. this keeps the current ROM, just resetting the state of the CPU
* run: exits the debugger, restarting execution as normal until it's brought up again

## Building
As long as you have SFML installed, just run [main.bash](main.bash). I haven't found a way to compile it on Windows, but it's just because I'm dumb. There is no reason that I know of for why this could not compile.
