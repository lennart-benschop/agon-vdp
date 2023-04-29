#!/usr/bin/env pytho3
import sys

fontname = sys.argv[1]
infile=open(fontname+".bdf","r")
outfile=open("showfont.bas","w")
outfile.write("""10 REM Show all characters in the UTF-8 font.
20 MODE 0:VDU 23,26,11,0,0,0
30 REPEAT
40 READ C
45 IF C=-1 THEN END
50 IF C<128 THEN VDU C ELSE IF C<2048 THEN VDU 192+C DIV 64,128+(C AND 63) ELSE VDU 224+C DIV 4096,128+((C DIV 64) AND 63),128+(C AND 63)
60 UNTIL 0
""")
lineno=100
ls = []
def write_data(c):
  global lineno,ls
  ls.append(str(c))
  if len(ls)==10 or c==-1:
     outfile.write("%d DATA %s\n"%(lineno,",".join(ls)))
     ls=[]
     lineno+=10
     
for line in infile.readlines():
    if line.startswith("ENCODING"):
        charcode = int(line.split(" ")[1])
        if charcode >= 32:
          write_data(charcode)
write_data(-1)
outfile.close()
     
