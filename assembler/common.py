from colorama import Fore as _Fore
import os as _os

INTERNAL_COMMAND_PREFIX = ".."

def print_if_allowed(*values):
	"""prints only if the ```QUIET``` flag is false"""
	# if QUIET is False:
	if True:
		for i in values:
			print(i, end=" ")
		print()

def abort_assembly(msg):
	print("error:", msg)
	print("burp")
	print(_Fore.RED, "assembly aborted", _Fore.WHITE)
	exit(-1)
def errormsg(msg, showline = True):
	fileinfo = getcurrentfileinfo()
	print(_Fore.RED + "error" + _Fore.WHITE + (f" on line {fileinfo[1]} of file {fileinfo[0]}" if showline is True else "") + ":", msg, "\n")
	markasinvalid()
def warning (msg):
	fileinfo = getcurrentfileinfo()
	print(_Fore.YELLOW + "warning" + _Fore.WHITE + f" on line {fileinfo[1]} of file {fileinfo[0]}:", msg, "\n")

#managed in here bc it's easier
VALID = True
def markasinvalid():
	global VALID
	VALID = False
def isvalid() -> bool:
	return VALID

current_line = 0
current_address = 0
current_file = ""
include_stack: list[tuple[str, int]] = []

def setfilename(name : str):
	global current_file
	current_file = name
def setcurrentline(value : int):
	global current_line
	current_line = value
def setcurrentaddr(value : int):
	global current_address
	current_address = value
def inc_line():
	global current_line
	current_line += 1
def inc_addr(value : int):
	global current_address
	current_address += value
def getcurrentfileinfo() -> tuple[str, int]:
	"returns a tuple of the form (current file name, current file line)"
	return (current_file, current_line)
def getcurrentaddr() -> int:
	return current_address
def getcurrentline() -> int:
	return current_line

def include_file(new_file_name : str):
	"""a file has been included. push the previous to the call stack and set to the new values"""
	global current_file, current_line
	include_stack.append( (current_file, current_line) )
	current_file, current_line = new_file_name, 0
def end_include ():
	"""pop from the stack into the current file and line"""
	global current_file, current_line
	current_file, current_line = include_stack.pop()


temp_files = []
def AddTempFile(name : str):
	temp_files.append(name)
def RemoveTempFiles():
	for file in temp_files:
		_os.remove(file)


#ignores the split if the symbol is between quotes
def CustomSplit(string, char) -> list[str]:
	tokens = []

	endloop = False
	isInDoubleQuotes = isInSingleQuotes = False

	while char in string and endloop is False:
		for x in range(len(string)):#TODO: cnotrolla che funzioni giusto
			if string[x] == '"':
				isInDoubleQuotes = not isInDoubleQuotes
			if string[x] == "'":
				isInSingleQuotes = not isInSingleQuotes

			if string[x] == char and not (isInSingleQuotes or isInDoubleQuotes):
				tokens += [string[:x]]
				string = string[x + 1:]
				break

			if x == len(string)-1:
				endloop = True
				break
	
	tokens += [string]		#what's left
	return tokens
