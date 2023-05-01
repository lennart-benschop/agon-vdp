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
    if line.startswith("BBX"):
        fw = int(line.split(" ")[1])
        fh = int(line.split(" ")[2])
        outfile.write("    {2*0x%04x, {"%charnum)
    if line.startswith("BITMAP"):
        righthalf=[]
        for i in range(fh):
            line=infile.readline()
            byteval=int(line,16)
            if fw>8:
                righthalf.append(byteval & 0xff)
                byteval >>=8
            outfile.write("0x%02x,"%byteval)
        outfile.write("}},\n")
        if fw>8:
            outfile.write("    {0x%04x, {"%(2*charnum+1))
            for b in righthalf:
                outfile.write("0x%02x,"%b)
            outfile.write("}},\n")
            
outfile.write("};\n")
outfile.close()
