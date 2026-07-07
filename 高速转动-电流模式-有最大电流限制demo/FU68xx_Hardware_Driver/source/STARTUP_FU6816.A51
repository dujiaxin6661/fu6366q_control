$NOMOD51

IDATALEN        EQU     256

XDATASTART      EQU     0
XDATALEN        EQU     4096

IBPSTACK        EQU     0
IBPSTACKTOP     EQU     IDATALEN

XBPSTACK        EQU     0
XBPSTACKTOP     EQU     XDATALEN

ACC     DATA    0E0H
B       DATA    0F0H
SP      DATA    81H
DPL     DATA    82H
DPH     DATA    83H

                NAME    ?C_STARTUP
?C_C51STARTUP   SEGMENT   CODE
?STACK          SEGMENT   IDATA

                RSEG    ?STACK
                DS      1

                EXTRN CODE (?C_START)
                PUBLIC  ?C_STARTUP

                CSEG    AT     0000H
?C_STARTUP:     LJMP    STARTUP1

                RSEG    ?C_C51STARTUP
STARTUP1:

IF IDATALEN <> 0
                MOV     R0,#IDATALEN - 1
                CLR     A
IDATALOOP:      MOV     @R0,A
                DJNZ    R0,IDATALOOP
ENDIF

IF XDATALEN <> 0
                MOV     DPTR,#XDATASTART
                MOV     R7,#LOW(XDATALEN)
  IF (LOW (XDATALEN)) <> 0
                MOV     R6,#HIGH(XDATALEN) +1
  ELSE
                MOV     R6,#HIGH(XDATALEN)
  ENDIF
                CLR     A
XDATALOOP:      MOVX    @DPTR,A
                INC     DPTR
                DJNZ    R7,XDATALOOP
                DJNZ    R6,XDATALOOP
ENDIF

IF IBPSTACK <> 0
EXTRN DATA (?C_IBP)
                MOV     ?C_IBP,#LOW IBPSTACKTOP
ENDIF

IF XBPSTACK <> 0
EXTRN DATA (?C_XBP)
                MOV     ?C_XBP,#HIGH XBPSTACKTOP
                MOV     ?C_XBP+1,#LOW XBPSTACKTOP
ENDIF
                MOV     SP, #?STACK-1
                LJMP    ?C_START

                END
