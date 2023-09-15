import os

from preprocessing import Preprocess
from body import EncodeInstructions, FixLabels, WriteOutputFile

from common import errormsg, warning, isvalid, RemoveTempFiles
from argscheck import PATH, NOPAUSE, KEEP_TEMP_FILES


#TODO:
#	eval
# 	rimuovi cose non necessarie (come tutti i toupper / tolower / strip visto che sono fatti già nel preprocessor)

#stuff like define, removing comments, etc
Preprocess(PATH)

#key is name of label, value is address it represents
labels = { }
#all the assembled bytes
allbytes = []

try:
	EncodeInstructions(allbytes, labels)

	FixLabels(allbytes, labels)

	if isvalid() is True:
		WriteOutputFile(allbytes, labels)
except Exception:
	import traceback
	tb_str = traceback.format_exc()
	errormsg("\n" + tb_str, False)

#remove temp file
if KEEP_TEMP_FILES is False:
	RemoveTempFiles()
if NOPAUSE is False:
	import platform
	os.system("pause" if platform.system() == "Windows" else 'read -r -p "Press enter to continue..." key')

exit(0 if isvalid() is True else 1)
