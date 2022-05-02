# IrresponsibleMemoryManagement
An emulator I made in my free time, for a _very_ simple CPU I designed. If you want to use it don't expect anything too crazy.

## Specifications
The full specification is in [dd.ò](dd.ò).

A few important things:
* it has 64 kB of RAM, 64 kB of VRAM (used to determine what pixels to render) and 256 bytes of control RAM for the computer's settings (currently unused for the most part);
* has four 8bit and one 16bit general purpose registers;
* each opcode is a byte long, excluding the parameters it may have;
* the display is 256x256, and can only show black and white. It doesn't have a set refresh rate, but refreshes every 65536 instructions are executed;
* it has a built in debugger, based on console commands. To use it just press esc while playing, and you can use it to do many cool things like reading/writing RAM, breakpoints, savestates etc.

## Assembler
The assembler is written in Python. Again, the full specification for the Assembly language is in the [dd.ò](dd.ò) file.
