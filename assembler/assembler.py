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

def abort_assembly(msg):
	print("error:", msg)
	print("burp")
	print(Fore.RED, "assembly aborted", Fore.WHITE)
	exit(-1)
def errormsg(msg, showline = True):
	global linecounter, valid
	print(Fore.RED + "error" + (f" on line {str(linecounter)}" if showline is True else "") + Fore.WHITE + ":", msg, "\n")
	valid = False
def warning (msg):
	global linecounter
	print(Fore.YELLOW + "warning on line " + str(linecounter) + Fore.WHITE + ":", msg, "\n")
ccinit(warning) #per usare i warning anche nell'altro file
del(ccinit)

if len(args) == 2:
	if args[1] == "-h":
		print;
		exit(0)

CURRENT_ADDRESS = 0

PATH = input("path to the file: ") if len(args) <= 1 else args[1]
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
		OUT_MDOE = "c-array"
	if args[i] == "-np":
		NOPAUSE = True
	if args[i] == "-q":
		QUIET = True
	if args[i] == "-org":
		CURRENT_ADDRESS = int(args[i+1], 0)
	#TODO: un coso pepr fare uscire i simboli di debug per essere portati nello script principale, un coso per definire costanti DALLO script principale a quelli secondari
if OUT_FILENAME == -1:
	OUT_FILENAME = Path(PATH).stem + (".bin" if OUT_MODE == "bin" else ".txt")
#TODO: maybe
# if os.path.isfile(OUT_FILENAME):
# 	abort_assembly("output file already exists")

#TODO: eval, include


#ignores the split if the symbol is between quotes
def CustomSplit(string, char):
	tokens = []

	endloop = False
	isInDoubleQuotes = isInSingleQuotes = False

	while char in string and endloop is False:
		for x in range(len(string)):
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
def Preprocessing():	#todo: do comment and other stuff in here
	global linecounter

	macros = {}

	charsToRemove = ["\t", " "]

	try:
		FILE = open(PATH)
		lines = [ line for line in FILE ]
		FILE.close()

		linecounter = 0
		while linecounter < len(lines):
			#all operations are done on this "line" variable and then reinserted in the array
			linecounter += 1
			line: str = lines[linecounter - 1].lower()
			
			if line[-1] == "\n": line = line[:-1]	#remove endline

			line = CustomSplit(line, ";")[0]  #remove comment

			line = line.lstrip()		#remove leading spaces
			if line == "":
				lines[linecounter - 1] = ""
				continue

			#remove tabs and spaces at the end of the line
			while line[-1] in charsToRemove:
				line = line[:-1]

			if line[0] == "#":
				if line[1:4] == "def":
					#create a macro
					key = line[5:(line.index("as") - 1)].strip()
					value = line[(line.index("as") + 3):].strip()
					macros[key] = value

					lines[linecounter - 1] = ""
					continue
				elif line[1:8] == "include":
					line = line[1:]
				else:
					errormsg("unknown macro \"" + line[1:] + "\"")
					exit()
			else:
				#replace in the line
				for macro in macros:
					while macro in line:
						index = line.index(macro)
						line = line[:index] + macros[macro] + line[(index + len(macro)):]
			lines[linecounter - 1] = line
		#END WHILE
		with open(PATH + ".temp", "w") as OUTFILE:
			for line in lines:
				OUTFILE.write(line + "\n")
	except:
		import traceback
		tb_str = traceback.format_exc()
		errormsg("\n" + tb_str)


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
				labels[line.lower()] = len(allbytes)

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
		if OUT_MODE == "bin":
			out = open(OUT_FILENAME, "wb")
			out.write(bytes(allbytes))
		elif OUT_MODE == "c-array":
			out = open(OUT_FILENAME, "w")
			out.write("{\n")
			for byte in allbytes:
				out.write( "\t" + str(byte) + ",\n")
			out.write("};")
			out.write("\n\n" + str(len(allbytes)))
		out.close()
		if QUIET is False:
			print("success!")

except Exception:
	import traceback
	tb_str = traceback.format_exc()
	errormsg("\n" + tb_str, False)

#remove temp file
if os.path.isfile(PATH + ".temp") and KEEP_TEMP_FILES is False:
	os.remove(PATH + ".temp")
os.system("pause") if NOPAUSE is False else None

exit(0 if valid is True else 1)
