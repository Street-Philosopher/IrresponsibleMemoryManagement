from sys import argv as args
from colorama import Fore
from codecheckers import INVALID_LABEL_NAMES, checkertable, InstrError, ParamError, IsLabelNameValid
from codecheckers import init as ccinit


"""
This contains the main "body" of the assembler
The actual syntax of the instructions is in "codecheckers.py"
"""


PATH = input("path to the file: ") if len(args) <= 1 else args[1]

#TODO: eval, include


def errormsg(msg, showline = True):
	global linecounter
	print(Fore.RED + "error" + f" on line {str(linecounter)}: " if showline is True else "", msg, "\n\n" + Fore.WHITE)
def warning (msg):
	global linecounter
	print(Fore.YELLOW + "warning on line " + str(linecounter) + ": ", msg, "\n\n" + Fore.WHITE)
ccinit(warning) #per usare i warning anche nell'altro file
del(ccinit)

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
			line = lines[linecounter - 1].lower()
			
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
					key = line[5:(line.index("as") - 1)]
					value = line[(line.index("as") + 3):]
					macros[key] = value

					lines[linecounter - 1] = ""
					continue
				else:
					errormsg("unknown macro \"" + line[1:] + "\"")
					exit()
				"""
				elif line[1:8] == "include":
					path = line[10:-1]
					c = 0
					#lines.pop(linecounter - 1)			#this line replaces the linecounter
					with open(path) as INCLUDEFILE:
						for include_line in INCLUDEFILE:
							lines.insert((linecounter + c), include_line)
							c += 1
					lines[linecounter - 1] = ""
					continue"""
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
		print("an error occured:\n", tb_str)

def Assemble(PATH, caller=""):
	global allbytes



#stuff like define, removing comments, etc
Preprocessing()

valid = True #is the file valid?
global linecounter
linecounter = 0

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
					valid = False
					errormsg("invalid syntax for label definition")
					continue
				#take out this so we can check
				line = line[:-1]
				#check if the label would replace stuff which is bad
				if line.upper() in INVALID_LABEL_NAMES:
					valid = False
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
				valid = False
				errormsg("invalid opcode: " + instr)
				continue
			#bytes for this instruction
			try:
				#org needs the current program length
				if instr == "org":
					params += [len(allbytes)]

				instBytes = instchecker(params)
			except (ParamError, InstrError) as pe:
				valid = False
				errormsg(pe.args[0])
				continue

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
			valid = False
			continue

		#if there is a label for that
		addr = labels[allbytes[i]]
		allbytes[i]     = addr  % 0x100	#convert label to address and change bytes
		allbytes[i + 1] = addr // 0x100
	#END LABEL FOR

	if valid is True:
		from pathlib import Path
		filename = Path(PATH).stem + ".bin"
		out = open(filename, "wb")
		out.write(bytes(allbytes))
		out.close()
		print("success!")

#		print ("{")
#		for byte in allbytes:
#			print( "		" + str(byte) + ",")
#		print("	};")
#		print("\n\n" + str(len(allbytes)))
except Exception:
	import traceback
	tb_str = traceback.format_exc()
	print("an error occured:\n", tb_str)

#remove temp file
import os
if os.path.isfile(PATH + ".temp"):
	os.remove(PATH + ".temp")
input("press enter to continue...")
