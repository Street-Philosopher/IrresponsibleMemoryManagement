from sys import argv as args
from colorama import Fore
from codecheckers import INVALID_LABEL_NAMES, checkertable, InstrError, ParamError, IsLabelNameValid
from codecheckers import init as ccinit
from pathlib import Path
import os


"""
This contains the main "body" of the assembler
The actual syntax of the instructions is in "codecheckers.py"
"""

def print_if_allowed(*values):
	"""prints only if the ```QUIET``` flag is false"""
	if QUIET is False:
		for i in values:
			print(i, end=" ")
		print()
def abort_assembly(msg):
	print("error:", msg)
	print("burp")
	print(Fore.RED, "assembly aborted", Fore.WHITE)
	exit(-1)
def errormsg(msg, showline = True):
	global linecounter, valid
	print(Fore.RED + "error" + Fore.WHITE + (f" on line {str(linecounter)} of file {CURRENT_FILE}" if showline is True else "") + ":", msg, "\n")
	valid = False
def warning (msg):
	global linecounter
	print(Fore.YELLOW + "warning" + Fore.WHITE + f" on line {str(linecounter)} of file {CURRENT_FILE}:", msg, "\n")
def setcurrentaddr(addr):
	global CURRENT_ADDRESS
	CURRENT_ADDRESS = addr
ccinit(warning, setcurrentaddr, (lambda: CURRENT_ADDRESS)) #per usare i warning anche nell'altro file
del(ccinit)

if len(args) == 2:
	if args[1] == "-h":
		print;	#TODO
		exit(0)

PATH = input("path to the file: ") if len(args) <= 1 else args[1]

CURRENT_ADDRESS = 0
CURRENT_FILE = PATH
INCLUDE_STACK: list[tuple[str, int]] = []

INTERNAL_COMMAND_PREFIX = ".."

# TODO:
# 	riordina codice:
# 	sposta il preprocessor in un file separato,
# 	sistema i command line arguments,
# 	rimuovi cose non necessarie (come tutti i toupper / tolower / strip visto che sono fatti già nel preprocessor)

OUT_FILENAME = -1
KEEP_TEMP_FILES = False
OUT_MODE = "bin"
NOPAUSE = False
QUIET = False
#TODO: make this not bad
for i in range(2, len(args)):
	if args[i] == "-o":
		try:
			OUT_FILENAME = args[i + 1]
		except:
			abort_assembly("no filename passed with the '-o' command")
	if args[i] == "-kt":
		KEEP_TEMP_FILES = True
	if args[i] == "-c":
		# OUT_MDOE = "c-array"		#of fucking course it's a misspelling error
		OUT_MODE = "c-array"
	if args[i] == "-np":
		NOPAUSE = True
	if args[i] == "-q":
		QUIET = True
	if args[i] == "-org":
		CURRENT_ADDRESS = int(args[i+1], 0)
if OUT_FILENAME == -1:
	OUT_FILENAME = Path(PATH).stem + (".bin" if OUT_MODE == "bin" else ".c")
print_if_allowed("assembling into", OUT_FILENAME)

#TODO: eval, include


#ignores the split if the symbol is between quotes
def CustomSplit(string, char) -> str:
	tokens = []

	endloop = False
	isInDoubleQuotes = isInSingleQuotes = False

	while char in string and endloop is False:
		for x in range(len(string)):#TODO: cnotrolla
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

#gotta love indentation
def Preprocessing():		#TODO: check for label definition here
	# global linecounter

	macros = {}

	# charsToRemove = ["\t", " "]

	try:
		FILE = open(PATH)
		lines = [ line for line in FILE ]
		newlines = []
		FILE.close()

		linecounter = 0
		for line in lines:		#TODO: preprocessing nel file incluso
			#all operations are done on this "line" variable and then reinserted in the array
			linecounter += 1
			line = line.lower()
			
			if line[-1] == "\n": line = line[:-1]	#remove endline

			line = CustomSplit(line, ";")[0]  #remove comment

			line = line.strip()		#remove leading spaces

			#append empty line, skip everything else as we're now indexing so it would cause errors
			if line == "":
				newlines.append("")
				continue

			# #remove tabs and spaces at the end of the line
			# while line[-1] in charsToRemove:
			# 	line = line[:-1]


			#TODO: migliora i comandi qui, troppo strano modificare un comando
			if line[0] == "#":
				if line[1:7] == "define":
					#create a macro
					key = line[8:(line.index("as") - 1)].strip()
					value = line[(line.index("as") + 3):].strip()
					macros[key] = value

					newlines.append("")
					continue
				elif line[1:8] == "include":
					# TODO:
					# 	do include by writing the contents here and writing metadata to say that we are in a new file;
					# 	push the current line and filename to some sort of stack when you reach one of those sections when assembling,
					# 	then reset the linecounter and set the filename to the current one, and once you're done reset the old ones
					# 	this way we can use errors and warning but simplify the process
					try:
						#find the path, absolute or relative. if only a filename is given it's assumed to be in the directory of the main file
						included_file_name = line[10:-1]
						if not os.path.isabs(included_file_name):
							included_file_name = os.path.dirname(os.path.abspath(args[1])) + "\\" + included_file_name
						
						#append the contents together with a command stating the beginning and end of an included file has occurred
						with open(included_file_name) as included_file:
							newlines.append(f"{INTERNAL_COMMAND_PREFIX}FILE {included_file_name}")
							for included_line in included_file:
								newlines.append(included_line)
							newlines.append(f"{INTERNAL_COMMAND_PREFIX}ENDFILE")
					except Exception as e:
						abort_assembly(f"could not include file '{included_file_name}' because an error occured:", e)
				else:
					errormsg("unknown macro \"" + line[1:] + "\"")
					exit()
			#check for defines only if there are no directives
			else:
				for macro in macros:
					while macro in line:
						index = line.index(macro)
						line = line[:index] + macros[macro] + line[(index + len(macro)):]

			newlines.append(line)
		#END WHILE
		with open(PATH + ".temp", "w") as OUTFILE:
			for line in newlines:
				OUTFILE.write(line + "\n")
	except:
		import traceback
		tb_str = traceback.format_exc()
		errormsg("\n" + tb_str, False)


valid = True #is the file valid?
global linecounter
linecounter = 0

#stuff like define, removing comments, etc
Preprocessing()

#key is name of label, value is address it represents
labels = { }

allbytes = []   #all the assembled bytes
try:
	with open(PATH + ".temp") as FILE:
		for line in FILE:
			linecounter += 1

			if line[-1] == "\n": line = line[:-1]	#remove endline

			#initialisation
			instBytes = []
			instchecker = None
			params = []
			instr = ""

			line = line.lstrip()		#remove leading spaces

			if line == "":	#means the line is only a comment or empty
				continue

			instr = line.split(" ")[0]  #get instruction
			#it's fine to use normal split here bc the first thing should always be the mnemonic, never something in a string

			#labels
			#if an instruction references an undefined symbol it will be considered a label and checked at the end
			if line[-1] == ":":
					#something		#check that all characters are valid
				if line != instr or IsLabelNameValid(line[:-1]) is not True:
					errormsg("invalid syntax for label definition")
					continue
				#take out this so we can check
				line = line[:-1]
				#check if the label would replace stuff which is bad
				if line.upper() in INVALID_LABEL_NAMES:
					errormsg("redefinition of a symbol in the creation of a label")
					continue
			
				if line.lower() in labels:
					warning("redefinition of label '" + line.lower() + "'")

				#create a new entry in label list (without colon)
				labels[line.lower()] = CURRENT_ADDRESS

				continue
			#END LABEL DEFINITION

			try:
				params = CustomSplit(line, " ")		#everything after params will be ignored
				params.pop(0)

				paramstring = ""
				for param in params:		#this way we can split
					paramstring += param
				
				params = CustomSplit(paramstring, ",")	#no spaces, always separated by comma
			except:
				params = []  #if there are no parameters an exception happens because yes

			while "" in params:
				params.remove("")
			
			try:
				instchecker = checkertable[instr]
			except:
				errormsg("invalid opcode: " + instr)
				continue
			#bytes for this instruction
			try:
				#org needs the current program length
				if instr in ["org", "include"]:
					params += [CURRENT_ADDRESS]

				instBytes = instchecker(params)
			except (ParamError, InstrError) as pe:
				errormsg(pe.args[0])
				continue

			CURRENT_ADDRESS += len(instBytes)

			for byte in instBytes:  #add all things
				allbytes.append(byte)

		#END MAIN FOR
	#END WITH FILE

	#check labels
	for i in range(len(allbytes)):
		if isinstance(allbytes[i], int):	#valid byte, no need to do anything
			continue
		
		if allbytes[i] is None:	#it woiuld be bc we foudn an undefined symbol. we don't want to do anything
			continue

		#if not a number, check if it is a valid label
		if allbytes[i] not in labels:
			errormsg(f"undefined symbol: '{allbytes[i]}'", False)
			continue

		#if there is a label for that
		addr = labels[allbytes[i]]
		allbytes[i]     = addr  % 0x100	#convert label to address and change bytes
		allbytes[i + 1] = addr // 0x100
	#END LABEL FOR

	if valid is True:
		#plain binary file
		if OUT_MODE == "bin":
			out = open(OUT_FILENAME, "wb")
			out.write(bytes(allbytes))
		#writes an array of bytes, adding the length of the array as well
		elif OUT_MODE == "c-array":
			out = open(OUT_FILENAME, "w")
			#define a uint8_t type
			out.write("typedef unsigned char uint8_t;\n\n")
			#write the length to a variable, and add a comment with the length in hex
			out.write(f"int program_length = {len(allbytes)};")
			out.write(f"\t\t// 0x{hex(len(allbytes)).upper()[2:]}\n")
			#write the array
			out.write("uint8_t program[] = {\n")
			for byte in allbytes:	#add padding				#this is to make it look better
				out.write( "\t0x" + ("0" if byte < 16 else "") + hex(byte).upper()[2:] + ",\n")
			out.write("};\n")
		out.close()
		print_if_allowed("success!")

except Exception:
	import traceback
	tb_str = traceback.format_exc()
	errormsg("\n" + tb_str, False)

#remove temp file
if os.path.isfile(PATH + ".temp") and KEEP_TEMP_FILES is False:
	os.remove(PATH + ".temp")
os.system("pause") if NOPAUSE is False else None

exit(0 if valid is True else 1)
