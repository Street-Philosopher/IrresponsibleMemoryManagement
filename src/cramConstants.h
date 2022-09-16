//addresses of CRAM registers or values they can have

//internal clock counter

#define CLOCK_COUNTER_LOW			0xF0
#define CLOCK_COUNTER_HIGH			0xF1

//how should we handle exceptions?
// ignore:	the exception is just skipped
// halt:	stops the CPU (opens the debugger)
// handle:	jumps to the address pointed by the EXC_HANDLE register, pushing some information to the stack

#define EXCEPTION_MODE_REGISTER		0x10
#define EXCEPTION_MODE_IGNORE		0x00
#define EXCEPTION_MODE_STOP			0x01
#define EXCEPTION_MODE_HANDLE		0x02

#define EXC_HANDLE_LOW				0x11
#define EXC_HANDLE_HIGH				0x12

//each bit indicates whether a certain button is pressed or not
// bit 0:	Right Arrow
// bit 1:	Left Arrow
// bit 2:	Up Arrow
// bit 3:	Down Arrow
// bit 4:	Z
// bit 5:	X
// bit 6:	A
// bit 7:	S
//the register is updated every time the screen is, in the updatescreen() function

#define INPUT_REGISTER				0x80
