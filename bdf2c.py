#!/usr/bin/env pytho3
import sys

fontname = sys.argv[1]
infile=open(fontname+".bdf","r")
fontname = fontname.replace("-","_")
outfile=open(fontname+".h","w")
while 1:
    line = infile.readline()
    if line.startswith('STARTCHAR'): break
    outfile.write("// " + line)
outfile.write("const CharDef16 %s_font_data[] = {\n" %fontname)

while 1:
    line = infile.readline()
    if line=="": break
    if line.startswith("ENCODING"):
        charnum = int(line.split(" ")[1])
        outfile.write("    {0x%04x, {"%charnum)
    if line.startswith("BITMAP"):
        for i in range(16):
            line=infile.readline()
            byteval=int(line,16)
            outfile.write("0x%02x,"%byteval)
        outfile.write("}},\n")
outfile.write("};\n")
outfile.close()
