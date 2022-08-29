from sys import argv as args
from typing import Callable
from common import setfilename, abort_assembly, setcurrentaddr, print_if_allowed
from pathlib import Path

class argument_command:
	#the amount of parameters that must follow it
	length:  int
	#the function that will manage it
	command: Callable
	#description to be used in help
	description: str
	#syntax hint
	syntax: str
	#name for it to be called
	name: str
	def __init__(self, name, command, length, syntax, description):
		self.name = name
		self.command = command
		self.length = length
		self.description = description
		self.syntax = syntax

def _kt():
	global KEEP_TEMP_FILES
	KEEP_TEMP_FILES = True
	return True
def _set_out_mode(mode):
	global OUT_MODE
	if mode == "-c":
		OUT_MODE = "c-array"
	elif mode == "-bin":
		OUT_MODE = "bin"
	return True
def _quiet():
	global QUIET
	QUIET = True
def _out_filename(name):
	global OUT_FILENAME
	OUT_FILENAME = name
	return True
def _nopause():
	global NOPAUSE
	NOPAUSE = True
	return True
def _setaddr(addr):
	try:
		setcurrentaddr(int(addr))
		return True
	except:
		return False

COMMANDS = [
	argument_command("-h",   lambda: False,                0, "",         "Show the help message and exit"),
	
	argument_command("-q",   _quiet,                       0, "",         "Minimises the amount of messages printed to the console"),
	argument_command("-kt",  _kt,                          0, "",         "Don't delete the temp files created by the assembly"),
	argument_command("-np",  _nopause,                     0, "",         "Don't prompt for input by the user at any point in the assembly"),
	
	argument_command("-c",   lambda:_set_out_mode("-c"),   0, "",         "Sets the output mode to a C header file, as an array"),
	argument_command("-bin", lambda:_set_out_mode("-bin"), 0, "",         "Sets the output mode to a plain binary file (default)"),

	argument_command("-o",   _out_filename,                1, "-o name",  "Sets the name of the output file"),

	argument_command("-org", _setaddr,                     1, "-org addr", "Start assembling at the given address. Defaults to zero"),
]
def _getcommand(name):
	for command in COMMANDS:
		if command.name == name:
			return command
	return None

if (len(args) == 2 and args[1] == "-h") or len(args) == 1:
	print("USAGE:", "assemble path/to/file [optional arguments]")
	print("\ncommands:")

	max_length = max(len(command.syntax if command.syntax != "" else command.name) for command in COMMANDS)
	
	for command in COMMANDS:
		length = len(command.syntax if command.syntax != "" else command.name)
		print("\t" + (command.syntax if command.syntax != "" else command.name), end="")
		print((" " * (max_length - length)), "--> ", end="")
		print(command.description)

	exit(0 if len(args) != 1 else 1)

PATH = args[1]
setfilename(PATH)

#default values
OUT_FILENAME = -1
KEEP_TEMP_FILES = False
OUT_MODE = "bin"
NOPAUSE = False
QUIET = False

current_arg = 2
while current_arg < len(args):
	cmd = _getcommand(args[current_arg])
	if cmd == None:
		abort_assembly(f"unknown option: '{args[current_arg]}'")

	#get the right number of parameters
	params = tuple( (args[current_arg+1:])[:cmd.length] )

	try:
		res = cmd.command(*params)
	except:
		res = False
	if res is False:
		abort_assembly(f"there was an error with the arguments of the command: '{args[current_arg]}'")

	current_arg += 1 + cmd.length

#set default filename if none was entered
if OUT_FILENAME == -1:
	OUT_FILENAME = Path(PATH).stem + (".bin" if OUT_MODE == "bin" else ".c")

if QUIET is False: print("assembling into '" + OUT_FILENAME + "'")
