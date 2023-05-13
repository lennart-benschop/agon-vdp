   10 MODE 7
   20 FOR I=1 TO 2:VDU 141:PRINT "Double Height";:VDU140:PRINT"Single Height":NEXT
   30 VDU 129:PRINT "RED ON BLACK";:VDU 157,135:PRINT "WHITE ON RED";:VDU156,131:PRINT "YELLOW"
   40 VDU 146:PRINT "GREEN GRAFIX 0123456abcdef"
   50 VDU 150,154:PRINT "CYAN GRAFIX 0123456abcdef"
   60 FOR I=160 TO 255:VDU I:NEXT:PRINT
   70 VDU132:PRINT"BLUE";:VDU131,136:PRINT"FLASH";:VDU 133,137:PRINT "MAGENTA"
   80 PRINT "Testing hold graphics semantics and"
   81 PRINT "attribute changes"
   90 PRINT "156, 157, 158, 137, 140, 141 take effect";
  100 PRINT "immediately others only in next"
  101 PRINT "character cell"
  120 VDU 158,151,166,150,150,150,135,135:PRINT "2 white 3 cyan"
  130 VDU 151,166,158,135,135: PRINT "3 white"
  140 VDU 158,151,193,150,135: PRINT "Letter not replciated"
  150 VDU 158,151,166,136,150,137,135,135: PRINT "2 white 1 flashing, 2 cyan"
  160 VDU 158,151,157,150,166,156,156,135,135:PRINT "1 cyan on white, 3 cyan on blk"
  170 VDU 158,151,166,159,159,166,135:PRINT "2 white 1 blank 1 white"
  180 VDU 158,151,166,141,166,150,140,135:PRINT "1 single 2 double height"
  190 VDU 158,151,166,141,166,150,140,135:PRINT "1 single 2 double height"
