; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
rmd EQU 0 ;32 bit unsigned dec
dec EQU 4

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH{R0,LR}
	SUB SP,#8 ;allocate local var
	MOV R3,#10
	CMP R0,R3
	
	BLO baseCase
	UDIV R1,R0,R3 ;R1 contains the MSV
	MLS R2,R1,R3,R0 ;R2 now contains remainder. this instruction does r0-r1*r3
	MOV R0,R1 ;R0 contains new value
	STR R2,[SP,#rmd] ;put remainder on stack
	BL LCD_OutDec ;call itself again
	;***return path***
	
	LDR R0,[SP,#rmd]
	
	
baseCase ADD R0,#0x30
	BL ST7735_OutChar

	;****restore stack****
	ADD SP,#8
	POP {R0,LR}

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH{R0,LR}
	SUB SP,#8
	
	LDR R1,=9999
	CMP R0,R1
	BHI ovf
	LDR R1,=1000
	UDIV R2,R0,R1 ;R2 contains the thousands place 
	MLS R3,R2,R1,R0 ;R3 contains remainder
	STR R3,[SP,#dec]
	MOV R0,R2
	BL LCD_OutDec ;should print out thousands place
	
	
	LDR R0,=0x2E 
	BL ST7735_OutChar ;should output a decimal
	
	LDR R0,[SP,#dec] ;takes value, max 999
	LDR R1,=100
	UDIV R2,R0,R1 ;divides it by 100, leaving hundreds place
	MLS R3,R2,R1,R0 ;we find the remainder, which is impt now
	STR R3,[SP,#rmd] ;R3 is stored as the remainder
	MOV R0,R2 ;divided value is loaded into R0
	ADD R0,R0,#0x30  ;convert to ASCII
	BL ST7735_OutChar ;should output hundreds digit, then subtract, leaving tens digit
	
	LDR R0,[SP,#rmd] ;take the remainder
	LDR R1,=10 
	UDIV R2,R0,R1 ;divide it by 10
	MLS R3,R1,R2,R0 ;find the new remainder
	STR R3,[SP,#rmd] ;store it
	MOV R0,R2 ;Move divided value into R0, prepare for output
	ADD R0,R0,#0x30 ;convert to ASCII
	BL ST7735_OutChar ;spit out
	
	LDR R0,[SP,#rmd] ;take final remainder. it is already a unit digit
	ADD R0,R0,#0x30 ;convert to ASCII
	BL ST7735_OutChar
	
	
	BL exitoutfix
	
ovf MOV R0,#0x2A
	BL ST7735_OutChar
	MOV R0,#0x2E
	BL ST7735_OutChar
	MOV R0,#0x2A
	BL ST7735_OutChar
	MOV R0,#0x2A
	BL ST7735_OutChar
	MOV R0,#0x2A
	BL ST7735_OutChar
	
exitoutfix	
	MOV R0,#0x20
	BL ST7735_OutChar
	MOV R0,#0x63
	BL ST7735_OutChar
	MOV R0,#0x6D
	BL ST7735_OutChar
	ADD SP,#8
	POP{R0,LR}
	
 BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
		 