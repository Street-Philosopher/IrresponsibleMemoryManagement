import os
from common import CustomSplit, INTERNAL_COMMAND_PREFIX, errormsg, print_if_allowed, warning, abort_assembly, AddTempFile
from sys import argv as args

macros = {}

#gotta love indentation
def Preprocess(file_path):
	global macros

	try:
		with open(file_path) as file:
			lines = [ line for line in file ]	#the lines read from the real file
			newlines = []						#the lines after preprocessing

		linecounter = 0
		for line in lines:
			#all operations are done on this "line" variable and then reinserted in the array
			linecounter += 1
			
			if line[-1] == "\n": line = line[:-1]	#remove endline

			line = CustomSplit(line, ";")[0]  #remove comment

			line = line.strip()		#remove leading spaces

			#append empty line, skip everything else as we're now indexing so it would cause errors
			if line == "":
				newlines.append("")
				continue


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
					try:

						#find the path, absolute or relative. if only a filename is given it's assumed to be in the directory of the main file
						included_file_name = line[10:-1]
						if not os.path.isabs(included_file_name):
							full_file_name = os.path.dirname(os.path.abspath(args[1])) + "/" + included_file_name
						else:
							full_file_name = included_file_name
							included_file_name = os.path.basename(included_file_name)

						print_if_allowed("including file '" + included_file_name + "'")
						
						#append the contents together with a command stating the beginning and end of an included file has occurred
						newlines.append(f"{INTERNAL_COMMAND_PREFIX}FILE {included_file_name}")

						Preprocess(full_file_name)
						with open(full_file_name + ".temp") as included_file:
							for included_line in included_file:
								newlines.append(included_line[:-1] if included_line[-1] == "\n" else included_line)

						newlines.append(f"{INTERNAL_COMMAND_PREFIX}ENDFILE")
						
						continue
					except Exception as e:
						abort_assembly(f"could not include file '{included_file_name}' because an error occured: " + str(e))
				else:
					errormsg("unknown macro \"" + line[1:] + "\"")
					exit()
			#check for defines only if there are no directives
			else:
				for macro in macros:
					while macro in line:
						index = line.index(macro)
						line = line[:index] + macros[macro] + line[(index + len(macro)):]
				line = line.lower()

			newlines.append(line)
		#END WHILE
		with open(file_path + ".temp", "w") as OUTFILE:
			for line in newlines:
				OUTFILE.write(line + "\n")
			AddTempFile(file_path + ".temp")
	except:
		import traceback
		tb_str = traceback.format_exc()
		errormsg("\n" + tb_str, False)
