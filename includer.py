import sys as sus

ifile = open(sus.argv[1], "r")
ofile = open(sus.argv[1] + ".inc", "w")

for line in ifile:
	if line.startswith( "#include "):
		line = line[len("#include "):]
		if line[-1] == "\n":
			line = line[:-1]
		line = line[1:-1] if (line[0] == line[-1] and line[0] in ["'", '"']) else line
		tmp = open(line, "r")
		ofile.write(f"\n\n; INCLUDING '{line}':\n; {'-' * 30}\n")
		for line2 in tmp:
			ofile.write(line2)
		ofile.write("\n; " + ("-" * 30) + "\n\n")
		tmp.close()
	else:
		ofile.write(line)
