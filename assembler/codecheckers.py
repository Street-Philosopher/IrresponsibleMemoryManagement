#from er import er as er

#TODO:
#   change checking functions very similar to each other to be a single one
#   change org so it doesnt pad bytes, and create a function to replace the current org

#this way we can have the warning function from main to here without circimp
from posixpath import isabs


funcs = []

labels = {}

#exceptions raised here and caught in the mainloop if an invalid thing happens
class ParamError(Exception):
	pass
class InstrError(Exception):
	pass

def init(warningfunc):
	global warning
	warning = warningfunc

#all regular 8-bit registers
regs = ["A", "B", "C", "D"]

					#all capital letters					#alll non capital letters				 #numbers
VALID_LABEL_CHARS = [ chr(num) for num in range(65, 91) ] + [ chr(num) for num in range(97, 123) ] + [ chr(num) for num in range(48, 58) ] + [ "_" ]
INVALID_LABEL_NAMES = ["DB", "ORG", "A", "B", "C", "D", "F", "HL", "AB", "PC", "SP", "H", "L", "Z", "NZ", "C"]


#self explanatory. returns true or false
def IsLabelNameValid(label):
	if label.lower() in opcodestable:
		return False
	if label[0] in ["0", "1", "3", "2", "4", "5", "6", "7", "8", "9"]:
		return False
	if all([char in VALID_LABEL_CHARS for char in label]) is not True:
		return False
	try:
		int(label, 0)
		return False	#if it's a number it's bad
	except:
		pass
	
	return True

#ah yes, stackoverflow
def StringToBytes(string, maxlength = -1):
	import re
	import codecs

	if len(string) == 0: raise ParamError("empty string")

	ESCAPE_SEQUENCE_RE = re.compile(r'''
		( \\U........      # 8-digit hex escapes
		| \\u....          # 4-digit hex escapes
		| \\x..            # 2-digit hex escapes
		| \\[0-7]{1,3}     # Octal escapes
		| \\N\{[^}]+\}     # Unicode characters by name
		| \\[\\'"abfnrtv]  # Single-character escapes
		)''', re.UNICODE | re.VERBOSE)

	def decode_escapes(s):
		def decode_match(match):
			return codecs.decode(match.group(0), 'unicode-escape')

		return ESCAPE_SEQUENCE_RE.sub(decode_match, s)
	
	bb = bytes(decode_escapes(string), 'utf-8')
	if maxlength > 0 and len(bb) > maxlength:
		bb = bb[:maxlength]
		warning("string given is too long, going to be resized to '" + string[:maxlength] + "'")	#warning

	return list(bb)

def GetBytesFromImmediate(param: str, maxbytes = -1) -> list[int]:
	#exceptions must be handled outside of this function

	if param[0] == param[-1] == '"' or param[0] == param[-1] == "'":	#this is so stupid I love it
		val = StringToBytes(param[1:-1], maxbytes)
		return val	#val is a list
	#for numbers
	val = int(param, 0)
	#keeps only the lowest (maxbytes) bytes of the end value
	if maxbytes != -1:
		max_factor = (1 << (maxbytes * 8)) - 1 
		if val != val & max_factor:
			val &= max_factor
			warning("value too large, changed to " + str(val))
	
	retval = []
	while val != 0:
		retval.append(val % 0x100)
		val >>= 8
	while len(retval) < maxbytes:
		retval += [0]
	if maxbytes > 0:
		while len(retval) > maxbytes:
			retval.pop()
	elif len(retval) == 0:
		retval = [0]

	return retval

#ADD_OPCODE:
#from instruction mnemonic to opcode
opcodestable = {
	#sys
	"nop"		: 0b0000_0000,
	"stop"		: 0b0000_0001,
	"halt"		: 0b0000_0010,
	"jp"		: 0b0000_0100,
	"call"		: 0b0000_1000,
	"ret"		: 0b0000_1100,
	"push"		: 0b0001_1000,
	"pop" 		: 0b0001_1010,
	#0b00000010 : "ooungh",

	#operations
	"add"		: 0b0010_0000,
	"sub"		: 0b0010_0100,
	"and"		: 0b0010_1000,
	"xor"		: 0b0010_1100,
	"or"		: 0b0011_0000,
	"inc"		: 0b0011_0100,
	"dec"		: 0b0011_1000,
	"not"		: 0b0111_0100,
	"ror"		: 0b0111_1000,
	"rol"		: 0b0111_1100,
	"cmp"		: 0b1000_0000,
	"adc"		: 0b1010_1000,

	#bit operations
	"bit"		: 0b1001_0000,
	"res"		: 0b1001_1000,
	"set"		: 0b1010_0000,
	"flag"		: 0b1000_1100,

	#loads
	"ld"		: 0b0100_0000,
	"ldi"		: 0b0110_1000,
	"ldd"		: 0b1000_0100,

	#video
	"stv"		: 0b1010_1100,
	"ldv"		: 0b1011_0000,
	"ccv"		: 0b1000_1100,
	"ccr"		: 0b1000_1101,

	#ldh
	"ldh"		: 0b0001_0110,
}


#returns the instruction offset given a condition (nz, etc)
def CheckConditional(cond):
	cond = cond.lower()
	if cond == "nz":
		return 1
	elif cond == "c":
		return 2
	elif cond == "z":
		return 3
	else:
		raise ParamError("invalid conditional '" + cond + "'")
#END


#these functions work by checking the parameter and returning a list of bytes corresponding to the instruction given

#sys instructions
def nop(param):
	if len(param) != 0:
		raise ParamError("expected no parameters after nop")
	return [opcodestable["nop"]]

def stop(param):
	if len(param) != 0:
		raise ParamError("expected no parameters after stop")
	return [opcodestable["stop"]]

def halt(param):
	if len(param) != 0:
		raise ParamError("expected no parameters after halt")
	return [opcodestable["halt"]]

def jp(param):
	if len(param) == 1:	#no conditionals
		param = param[0].upper()
		if param == "(HL)":
			return [opcodestable["jp"] + 0b0110_1100]
		
		try:
			val = int(param, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [opcodestable["jp"], val % 0x100, val // 0x100]
		except:
			#the signal that this is to be added to the label waiting list is the name of the label
			#labels are case insensitive because i said so
			return [ opcodestable["jp"], param.lower(), None ]
	if len(param) == 2:	#there's a conditional
		cond = param[0].upper()
		addr = param[1].upper()
		cond = CheckConditional(cond)

		if addr == "(HL)":
			return [opcodestable["jp"] + cond]	#we add the condition as offset
		
		try:
			val = int(addr, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [(opcodestable["jp"] + cond), val % 0x100, val // 0x100]
		except:
			#add to label waiting list
			return [ (opcodestable["jp"] + cond), addr.lower(), None ]
	raise ParamError("invalid parameter number for jp")
def jnz(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for jnz")
	
	params = ["nz"] + param
	return jp(params)
def jc(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for jc")

	params = ["c"] + param
	return jp(params)
def jz(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for jz")
		
	params = ["z"] + param
	return jp(params)

def call(param):
	if len(param) == 1:	#no conditionals
		param = param[0].upper()
		if param == "(HL)":
			return [opcodestable["call"] + 0b0001_0100]
		
		try:
			val = int(param, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [opcodestable["call"], val % 0x100, val // 0x100]
		except:
			return [ opcodestable["call"], param.lower(), None ]
	if len(param) == 2:	#there's a conditional
		cond = param[0].upper()
		addr = param[1].upper()
		cond = CheckConditional(cond)

		if addr == "(HL)":
			return [opcodestable["call"] + cond]	#we add the condition as offset
		
		try:
			val = int(addr, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [(opcodestable["call"] + cond), val % 0x100, val // 0x100]
		except:
			return [ opcodestable["call"] + cond, addr.lower(), None ]
	raise ParamError("invalid param number for call")
def cnz(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for cnz")
		
	params = ["nz"] + param
	return call(params)
def callc(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for callc")
	
	params = ["c"] + param
	return call(params)
def callz(param):
	if len(param) != 1:
		raise ParamError("expected one parameter for callz")
		
	params = ["z"] + param
	return call(params)

def ret(param):
	if len(param) == 0:	#unconditional
		return [opcodestable["ret"]]
	
	if len(param) == 1:	#conditional
		cond = param[0].upper()
		cond = CheckConditional(cond)
		return [opcodestable["ret"] + cond]
	
	raise ParamError("invalid number of parameters for ret")
def retnz(param):
	if len(param) != 0:
		raise ParamError("no params expected for retnz")
	return [opcodestable["ret"] + CheckConditional("nz")]
def retc(param):
	if len(param) != 0:
		raise ParamError("no params expected for retc")
	return [opcodestable["ret"] + CheckConditional("c") ]
def retz(param):
	if len(param) != 0:
		raise ParamError("no params expected for retz")
	return [opcodestable["ret"] + CheckConditional("z") ]

def push(param):
	if len(param) != 1:
		raise ParamError("one parameter expected for push")
	
	if param[0].upper() == "HL":
		return [opcodestable["push"]]
	if param[0].upper() == "AB":
		return [opcodestable["push"] + 1]
	
	raise ParamError(f"invalid parameter for push: '{param[0]}'")
def pop(param):
	if len(param) != 1:
		raise ParamError("one parameter expected for pop")
	
	if param[0].upper() == "HL":
		return [opcodestable["pop"]]
	if param[0].upper() == "AB":
		return [opcodestable["pop"] + 1]
	
	raise ParamError(f"invalid parameter for pop: '{param[0]}'")

#alu isntructions
def add(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for add")
	
	if params[0].upper() == "A":
		param = params[1].upper()
		if param in regs:
			return [opcodestable["add"] + regs.index(param)]
		else:
			try:
				val = int(param.lower(), 0)
				if val != val % 0x100:
					val = val % 0x100
					warning("value can't fit in a byte, will be changed to " + str(val))
				return [ 0b00010011, val ]
			except:
				raise ParamError("second parameter for adding to A should be a register (A,B,C,D)")

	if params[0].upper() == "HL":
		if params[1].upper() == "AB":
			return [opcodestable["add"] + -0b1100]
		else:
			try:
				val = int(params[1].lower(), 0)
				if val != val % 0x10000:
					val = val % 0x10000
					warning("value can't fit in a word, will be changed to " + str(val))
				return [ 0b11000000, val % 0x100, val // 0x100 ]
			except:
				return [ 0b11000000, params[1].lower(), None ]

	raise ParamError("first parameter for add should be either A or HL")
def adc(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for adc")
	
	if params[0].upper() == "A":
		param = params[1].upper()
		if param in regs:
			return [opcodestable["adc"] + regs.index(param)]
		else: raise ParamError("second parameter for carry addition to A should be a register (A,B,C,D)")
	else: raise ParamError("first parameter for adc should be A")

def sub(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for sub")

	if params[0].upper() == "A":
		param = params[1].upper()
		if param in regs:
			return [opcodestable["sub"] + regs.index(param)]
		else:	#immediate
			try:
				val = int (params[1], 0)
				if val != val % 0x100:
					val = val % 0x100
					warning("value can't fit in a byte, will be changed to " + str(val))
				return [ 0b00010101, val ]
			except:
				raise ParamError("second parameter for subtraction from A should be a register (A,B,C,D)")
	elif params[0].upper() == "HL":	#sub hl, immediate
		param = params[1].lower()
		try:
			val = int(param, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [(opcodestable["sub"] + 0b10011101), val % 0x100, val // 0x100]
		except:
			return  [(opcodestable["sub"] + 0b10011101), param.lower(), None ]
	else:
		raise ParamError("first parameter for sub should be A or HL")

def And(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for and")

	if params[0].upper() == "A":
		param = params[1].upper()
		if param == "A":
			raise ParamError("logical operation 'A and A' is not allowed")
		if param in regs:
			return [opcodestable["and"] + regs.index(param)]
		else:
			#immediate
			try:
				val = int(param, 0)
				if val != val & 0xFF:
					val &= 0xFF
					warning("value too large, changed to " + str(val))
				return [
					opcodestable["and"],	#it takes the place of "and A,A"
					val
				]
			except:
				raise ParamError("invalid parameter for 'and' operation on A")
	else: raise ParamError("first parameter for and should be A")
def Or(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for or")

	if params[0].upper() == "A":
		param = params[1].upper()
		if param == "A":
			raise ParamError("logical operation 'A or A' is not allowed")
		if param in regs:
			return [opcodestable["or"] + regs.index(param)]
		else:
			#immediate
			try:
				val = int(param, 0)
				if val != val & 0xFF:
					val &= 0xFF
					warning("value too large, changed to " + str(val))
				return [
					opcodestable["or"],		#it takes the place of "or A,A"
					val
				]
			except:
				raise ParamError("invalid parameter for 'or' operation on A")
	else: raise ParamError("first parameter for or should be A")
def Xor(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for xor")

	if params[0].upper() == "A":
		param = params[1].upper()
		if param in regs:
			return [opcodestable["xor"] + regs.index(param)]
		else: raise ParamError("second parameter for xor operation on 'A' should be a register (A,B,C,D)")
	else: raise ParamError("first parameter for xor should be A")

def Not(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for not")

	param = params[0].upper()
	if param in regs:
		return [opcodestable["not"] + regs.index(param)]
	else: raise ParamError("parameter for not should be a register (A,B,C,D)")

def inc(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for inc")

	param = params[0].upper()
	if param in regs:
		return [opcodestable["inc"] + regs.index(param)]
	elif param == "HL":
		return [opcodestable["inc"] + 0b1000]
	elif param == "SP":
		return [opcodestable["inc"] + 0b1001]
	else: raise ParamError("parameter for inc should be a register (A,B,C,D)")
def dec(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for dec")

	param = params[0].upper()
	if param in regs:
		return [opcodestable["dec"] + regs.index(param)]
	if param == "HL":
		return [opcodestable["inc"] + 0b1010]	#THIS IS NOT A MISTKAE, it's right
	if param == "SP":
		return [opcodestable["inc"] + 0b1011]
	else: raise ParamError("parameter for dec should be a register (A,B,C,D)")

def ror(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for ror")
	
	param = params[0].upper()
	if param in regs:
		return [opcodestable["ror"] + regs.index(param)]
	else: raise ParamError("parameter for ror should be a register (A,B,C,D)")
def rol(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for rol")
	
	param = params[0].upper()
	if param in regs:
		return [opcodestable["rol"] + regs.index(param)]
	else: raise ParamError("parameter for rol should be a register (A,B,C,D)")

def cmp(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for cmp: expected two")

	if params[0].upper() == "A":
		param2 = params[1].upper()
		if param2 == "A":
			raise ParamError("can't compare A with itself")
		if param2 in regs:
			return [ opcodestable["cmp"] + regs.index(param2) ]
		else:
			try:
				val = int(param2, 0)
				if val != val & 0xFF:
					val &= 0xFF
					warning("value too large, changed to " + str(val))
				return [
					opcodestable["cmp"],	#it takes the place of "cmp A,A"
					val
				]
			except:
				raise ParamError("expected a register or an immediate as the second parameter")
	else: raise ParamError("first parameter for cmp should be A")

#bit instructions
def bit(params):
	if len(params) != 2:
		raise ParamError("expected two parameters for bit instruction")
	
	try:
		val1 = int(params[0], 0)
		if not 0 <= val1 <= 7:
			raise	#it's caught down there, bc the exception is same if the value is not a number as if it is a bad number
	except:
		raise ParamError("expected a number from zero to seven for the first parameter of bit")
	
	if params[1].upper() != "A":
		raise ParamError("expected A as the second parameter for bit instruction")
	
	return [opcodestable["bit"] + val1]		#"bit 0" is offset 0 from "bit", "bit 5" is offset 5 from "bit" and so on
def res(params):
	if len(params) != 2:
		raise ParamError("expected two parameters for res instruction")
	
	try:
		val1 = int(params[0], 0)
		if not 0 <= val1 <= 7:
			raise
	except:
		raise ParamError("expected a number from zero to seven for the first parameter of res")
	
	if params[1].upper() != "A":
		raise ParamError("expected A as the second parameter for res instruction")
	
	return [opcodestable["res"] + val1]
def Set(params):
	if len(params) != 2:
		raise ParamError("expected two parameters for set instruction")
	
	try:
		val1 = int(params[0], 0)
		if not 0 <= val1 <= 7:
			raise
	except:
		raise ParamError("expected a number from zero to seven for the first parameter of set")
	
	if params[1].upper() != "A":
		raise ParamError("expected A as the second parameter for set instruction")
	
	return [opcodestable["set"] + val1]

def flag(params):
	if len(params) != 1:
		raise ParamError("expected one parameter for 'flag' instruction")
	
	param = params[0]

	try:
		val1 = int(param, 0)
		if not 0 <= val1 <= 7:
			raise
	except:
		raise ParamError("expected a number from two to seven for parameter of 'flag'")
	
	if val1 == 0 or val1 == 1:	#flag 0 and 1 are prohibited
		raise ParamError("'flag 0' and 'flag 1' are prohibited")

	return [ opcodestable["flag"] + val1 ]

#loads
def ld(params):
	if len(params) != 2:
		raise ParamError("expected two arguments for ld")

	ldc = opcodestable["ld"]	#ld code

	param1 = params[0].upper()
	param2 = params[1].upper()
	if param1 in regs:
		if param2 in regs:	#ld reg,reg
			if param1 == param2:
				raise ParamError("loading from and to the same register is not allowed")
			return [
				#destination is in bits 2 and 3, other register in bits 0-1
				ldc + 0b100 * regs.index(param1) + regs.index(param2)
			]
		#END
		if param2 == "H":
			if param1 == "A":
				return [
					ldc - 0b101111
				]
			else: raise ParamError(f"can't load from H to {param1}")
		if param2 == "L":
			if param1 == "A":
				return [
					ldc - 0b101110
				]
			else: raise ParamError(f"can't load from H to {param1}")
		if param2 == "(HL)":
			return [
				ldc + regs.index(param1) + 0b10100
			]
		#END
		if param2 == "(HL+)":	#alternative for ldi
			return [
				ldc + regs.index(param1) + 0b101000
			]
		if param2 == "(HL-)":	#alternative for ldd
			return [
				ldc + regs.index(param1) + 0b1000100
			]
		if param2[0] == "(" and param2[-1] == ")":	#address
			param2 = param2[1:-1]		#remove parentheses
			try:
				val = int(param2, 0)
				if val != val % 0x10000:
					val %= 0x10000		#clamp
					warning("address value too large, will be changed to " + str(val))
				return [
					ldc + regs.index(param1) + 0b100100,
					val  % 0x100,
					val // 0x100,
				]
			except:
				return [
					ldc + regs.index(param1) + 0b100100,
					param2.lower(),
					None
				]

		#else it's an immediate:

		try:
			val = GetBytesFromImmediate(params[1], maxbytes=1)
			return [ ldc + regs.index(param1) ] + val
		except:
			raise ParamError(f"invalid parameter for load to address '{params[1]}'")
	#END of the "ld to reg" section

	if param1 == "(HL)":	#load to HL pointer
		if param2 not in regs:
			raise ParamError(f"cannot load from {param2} to an HL pointer")
		return [
			ldc + regs.index(param2) + 0b11000
		]
	if param1 == "(HL+)":	#ldi (HL),reg
		if param2 not in regs:
			raise ParamError(f"invalid parameter '{param2}' for ld (HL+)")
		return [
			ldc + regs.index(param2) + 0b101100
		]
	if param1 == "(HL-)":	#ldd (HL),reg
		if param2 not in regs:
			raise ParamError(f"invalid parameter '{param2}' for ld (HL-)")
		return [
			ldc + regs.index(param2) + 0b01001000
		]
	if param1 == "HL":
		if param2 == "AB":
			return [ 0b01011101 ]

		#else it's to immediate or label
		try:
			val = GetBytesFromImmediate(params[1], maxbytes=2)
			return [ ldc + 0b011100 ] + val
		except:
			return [ (ldc + 0b011100), param2.lower(), None ]
	if param1 == "AB":
		if param2 == "HL":
			return [ 0b11000010 ]
		else:
			raise ParamError("invalid parameter for load to AB: " + param2)
	if param1 == "H":
		if param2 == "A":
			return [ ldc + 0b011110 ]
		else: raise ParamError(f"can't load to L from {param2}")
	if param1 == "L":
		if param2 == "A":
			return [ ldc + 0b011111 ]
		else: raise ParamError(f"can't load to L from {param2}")
	
	#load to address
	if param1[0] == "(" and param1[-1] == ")":
		param1 = param1[1:-1]
		if param2 not in regs:
			raise ParamError("invalid second parameter for ld to address")
		try:
			val = int(param1, 0)
			if val != val & 0xFFFF:
				val &= 0xFFFF
				warning("address too large, changed to " + str(val))
			return [
				ldc + 0b100000 + regs.index(param2),
				val  % 0x100,
				val // 0x100
			]
		except:
			#we try to see if it's a label
			return [
				ldc + 0b100000 + regs.index(param2),
				param1.lower(),
				None
			]
	
	#reached if error
	raise ParamError("invalid parameters for ld")
def ldi(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for ldi")

	if params[0].upper() in regs:
		if params[1].upper() != "(HL)":
			raise ParamError("expected '(HL)' after register for ldi")
		return [
			#to this we add the register
			opcodestable["ldi"] + regs.index(params[0].upper())
		]
	if params[0].upper() == "(HL)":
		if params[1].upper() not in regs:
			raise ParamError("expected register after '(HL)' for ldi")
		return {
			# + 4 because offset
			opcodestable["ldi"] + 4 + regs.index(params[1].upper())
		}
	
	raise ParamError("first parameter for ldd should be either a register or pointer to HL")
def ldd(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for ldd")

	if params[0].upper() in regs:
		if params[1].upper() != "(HL)":
			raise ParamError("expected '(HL)' after register for ldd")
		return [
			#to this we add the register
			opcodestable["ldd"] + regs.index(params[0].upper())
		]
	if params[0].upper() == "(HL)":
		if params[1].upper() not in regs:
			raise ParamError("expected register after '(HL)' for ldd")
		return {
			# + 4 because offset
			opcodestable["ldd"] + 4 + regs.index(params[1].upper())
		}
	
	raise ParamError("first parameter for ldd should be either a register or pointer to HL")

def stv(params):
	if len(params) != 2:
		raise ParamError("invalid parameter number for stv")
	
	if params[1].upper() not in regs:
		raise ParamError("invalid second parameter for stv instruction")
	if params[0][0] != "(" or params[0][-1] != ")":
		raise ParamError("invalid first argument for stv")

	params[0] = params[0][1:-1]

	#if it's an HL pointer
	if params[0].upper() in ["HL", "HL+", "HL-"]:
		if params[1].upper() != "A":
			raise ParamError("second parameter for stv to HL pointer should be A")
		return [
			#stv (HL) is at 8 offsets from the first stv. to this, we add which type of pointer it is
			opcodestable["stv"] + 8 + ["HL", "HL+", "HL-"].index(params[0].upper())
		]
	try:
		val = int(params[0], 0)		#if it is a number
		if val != val & 0xFFFF:
			val &= 0xFFFF
			warning("address too large, changed to " + str(val))
		return [
			opcodestable["stv"] + regs.index(params[1].upper()),
			val  % 0x100,
			val // 0x100
		]
	except:
		return [
			opcodestable["stv"] + regs.index(params[1].upper()),
			params[0].lower(),
			None
		]

def ldv(parmas):
	if len(parmas) != 2:
		raise ParamError("invalid parameter number for ldv")
	
	if parmas[0].upper() not in regs:
		raise ParamError("invalid first parameter for ldv instruction")
	if parmas[1][0] != "(" or parmas[1][-1] != ")":
		raise ParamError("invalid second argument for ldv")

	parmas[1] = parmas[1][1:-1]

	if parmas[1].upper() in ["HL"]:
		if parmas[0].upper() != "A":
			raise ParamError("first parameter for ldv to HL pointer should be A")
		return [
			opcodestable["ldv"] + 7		#ok
		]
	try:
		val = int(parmas[1], 0)		#if it is a number
		if val != val & 0xFFFF:
			val &= 0xFFFF
			warning("address too large, changed to " + str(val))
		return [
			opcodestable["ldv"] + regs.index(parmas[0].upper()),
			val  % 0x100,
			val // 0x100
		]
	except:
		return [
			opcodestable["ldv"] + regs.index(parmas[0].upper()),
			parmas[1].lower(),
			None
		]

#IMPORTANT: these two are pretty much the same, so if you change one cahnge the other too
def ccv(params):
	if len(params) != 2:
		raise ParamError("expected two parameters for ccv")

	if params[0].lower() != "(hl)":
		raise ParamError("first parameter for ccv should be a pointer to HL")
	if params[1][0] != "(" or params[1][-1] != ")":
		raise ParamError("invalid second argument for ccv")

	params[1] = params[1][1:-1]

	if params[1].lower() == "ab":
		return [ opcodestable["ccv"] + 2 ]
	
	try:
		#immediate
		val = int(params[1], 0)
		if val != val & 0xFFFF:
			val &= 0xFFFF
			warning("address too large, changed to " + str(val))
		return [
			opcodestable["ccv"],
			val  % 0x100,
			val // 0x100
		]
	except:
		#label
		return [
			opcodestable["ccv"],
			params[1].lower(),
			None
		]
def ccr(params):
	if len(params) != 2:
		raise ParamError("expected two parameters for ccr")

	if params[0].lower() != "(hl)":
		raise ParamError("first parameter for ccr should be a pointer to HL")
	if params[1][0] != "(" or params[1][-1] != ")":
		raise ParamError("invalid second argument for ccr")

	params[1] = params[1][1:-1]

	if params[1].lower() == "ab":
		return [ opcodestable["ccr"] + 2 ]
	
	try:
		#immediate
		val = int(params[1], 0)
		if val != val & 0xFFFF:
			val &= 0xFFFF
			warning("address too large, changed to " + str(val))
		return [
			opcodestable["ccr"],
			val  % 0x100,
			val // 0x100
		]
	except:
		#label
		return [
			opcodestable["ccr"],
			params[1].lower(),
			None
		]

def ldh(params):
	if params[0].upper() == "A":
		if params[1][0] != "(" or params[1][-1] != ")":
			raise ParamError("expected an address as second parameter to ldh")
		params[1] = params[1][1:-1]

		try:
			val = int(params[1], 0)
			if val % 0x100 != val:
				val = val % 0x100
				warning("address doesn't fit in an unsigned byte, will be resized to " + str(val))
		except:
			raise ParamError("expected an 8bit address for ldh")
		return [
			(opcodestable["ldh"] + 1),
			val
		]
	elif params[1].upper() == "A":
		if params[1][0] != "(" or params[1][-1] != ")":
			raise ParamError("expected an address as first parameter to ldh")
		params[0] = params[0][1:-1]

		try:
			val = int(params[0], 0)
			if val % 0x100 != val:
				val = val % 0x100
				warning("address doesn't fit in an unsigned byte, will be resized to " + str(val))
		except:
			raise ParamError("expected an 8bit address for ldh")
		return [
			opcodestable["ldh"],
			val
		]
	else:
		raise ParamError("invalid parameters for ldh")

#assembler commands
def db(params):
	if len(params) == 0:
		raise ParamError("at least one byte expected after db")
	
	Bytes = []
	for param in params:
		try:
			val = GetBytesFromImmediate(param)
			Bytes += val
		except Exception as e:
			print(e)
			#labels
			Bytes.append(param)
			Bytes.append(None)
	return Bytes

def org(params):
	if len(params) not in [2, 3]:
		#i swear this makes sense
		#we add manually an argument
		raise ParamError("expected one or two parameter for org")
	try:
		addr = int(params[0], 0)
	except:
		raise ParamError("expected an address after 'org' directive")
	if addr < params[-1]:
		raise ParamError("cannot 'org' backwards")

	if len(params) == 2:	#there was no specified value, so set 0
		filler = 0
	if len(params) == 3:	#there was a value for the filler
		if params[1] == "r":	#an 'r' for the filler stands for 'random bytes'
			filler = 'r'
		else:
			try:
				filler = int(params[1], 0)
				if not 0 <= filler <= 255:
					filler %= 0x100
					warning("filler value too large, will be rounded to " + str(filler))
			except:
				raise ParamError("expected a byte value for second parameter of org")

	# we write bytes until we reach the desired address. this is a cool shortcut
	if filler != 'r': retval = [filler] * (addr - params[-1])
	else:
		#if the filler is 'r' we want a set of random bytes
		import random
		import time
		random.seed(time.time())
		retval = random.randbytes(addr - params[-1])
	
	#this means we now have the correct amount of bytes to append
	return retval

def include(args):
	from sys import argv
	import os
	if len(args) != 2: raise ParamError("invalid number of parameters for include directive")
	OUT_PATH = "o.tmp"
	#if there is no abspath set it to the path of the script we're currently assembling
	path = args[0]
	if not os.path.isabs(path):
		path = os.path.dirname(os.path.abspath(argv[1])) + "\\" + path

	#call the assembler
	assembly_result = os.system(f"{argv[0]} {path} -np -ds -q -org {args[1]} -o {OUT_PATH}")
	if assembly_result != 0:
		raise InstrError(f"could not include file '{path}'")

	#add the compile to the current list of bytes
	with open(OUT_PATH, "rb") as assembledfile:
		retval = list(assembledfile.read(-1))

	#remove the temp file
	os.remove(OUT_PATH)

	return retval

#ADD_OPCODE:
#table that returns the checker function for the given mnemonic
checkertable = {
	#sys
	"nop"		: nop,
	"stop"		: stop,
	"halt"		: halt,
	"jp"		: jp,
	"jnz"		: jnz,
	"jc"		: jc,
	"jz"		: jz,
	"call"		: call,
	"cnz"		: cnz,
	"callc"		: callc,
	"callz"		: callz,
	"ret"		: ret,
	"retnz"		: retnz,
	"retc"		: retc,
	"retz"		: retz,
	"push"		: push,
	"pop" 		: pop,
#	0b00000010 : "ooungh",

	#operations
	"add"		: add,
	"sub"		: sub,
	"and"		: And,
	"xor"		: Xor,
	"or"		: Or,
	"inc"		: inc,
	"dec"		: dec,
	"not"		: Not,
	"ror"		: ror,
	"rol"		: rol,
	"cmp"		: cmp,
	"adc"		: adc,

	#bit operations
	"bit"		: bit,
	"res"		: res,
	"set"		: Set,
	"flag"		: flag,

	#loads
	"ld"		: ld,
	"ldi"		: ldi,
	"ldd"		: ldd,

	#video
	"stv"		: stv,
	"ldv"		: ldv,
	"ccv"		: ccv,
	"ccr"		: ccr,

	#ldh
	"ldh"		: ldh,

	#assembler commands
	"db"		: db,
	"org"		: org,
	"include"	: include,
}
