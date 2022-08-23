import os

from preprocessing import Preprocess
from common import CustomSplit, getcurrentaddr, print_if_allowed, abort_assembly, errormsg, warning, isvalid, setfilename, inc_line, inc_addr, setcurrentaddr, INTERNAL_COMMAND_PREFIX, include_file, end_include, RemoveTempFiles
from codecheckers import INVALID_LABEL_NAMES, checkertable, InstrError, ParamError, IsLabelNameValid
from argscheck import PATH, OUT_MODE, OUT_FILENAME, NOPAUSE, KEEP_TEMP_FILES


"""
This contains the main "body" of the assembler
The actual syntax of the instructions is in "codecheckers.py"
"""

#TODO:
#	eval
# 	riordina codice:
# 	rimuovi cose non necessarie (come tutti i toupper / tolower / strip visto che sono fatti già nel preprocessor)

#stuff like define, removing comments, etc
Preprocess(PATH)

#key is name of label, value is address it represents
labels = { }

allbytes = []   #all the assembled bytes
try:
	with open(PATH + ".temp") as FILE:
		for line in FILE:
			inc_line()

			if line[-1] == "\n": line = line[:-1]	#remove endline

			#check for internal commands
			if line.startswith(INTERNAL_COMMAND_PREFIX):
				#set the file name to the included one and line number to zero
				if line[2:6] == "FILE":
					include_file(line[7:])
					continue
				#we reached the end of the included file, so reset to the previous one
				elif line[2:] == "ENDFILE":
					end_include()
					continue

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
				labels[line.lower()] = getcurrentaddr()

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
					params += [getcurrentaddr()]

				instBytes = instchecker(params)
			except (ParamError, InstrError) as pe:
				errormsg(pe.args[0])
				continue

			inc_addr(len(instBytes))

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

	if isvalid() is True:
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
if KEEP_TEMP_FILES is False:
	RemoveTempFiles()
os.system("pause") if NOPAUSE is False else None

exit(0 if isvalid() is True else 1)
