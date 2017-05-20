; Print.s

; Student names: Armin and Artin Mohammadtaher

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

		

Pointer_In 	EQU 0

Pointer_D 	EQU 4

Pointer_Fix EQU 8

    AREA    |.text|, CODE, READONLY, ALIGN=2

    THUMB



  



;-----------------------LCD_OutDec-----------------------

; Output a 32-bit number in unsigned decimal format

; Pointer_In: R0 (call by value) 32-bit unsigned number

; Output: none

; Invariables: This function must not permanently modify registers R4 to R11



LCD_OutDec

	PUSH 	{R4-R12, LR} 				

	PUSH	{R1,R0} 					

	STR 	R0, [SP, #Pointer_In] 	

	MOV 	R2, #1						

	MOV 	R3, #9					

	MOV 	R1, #10 					

	CMP 	R0, #0 						

	BEQ 	First0 			

	

CreateaMilliard 

	MUL 	R2, R1 						

	SUB 	R3, #1  								

	CMP 	R3, #0						

	BNE 	CreateaMilliard				

	STR 	R2, [SP, #Pointer_D] 	

	

Find_Digit1

	LDR 	R2, [SP, #Pointer_D] 	

	UDIV 	R3, R0, R2					

	CMP 	R3, #0 						

	BEQ 	Error404 					

	B 		First_Found 			

	

Error404

	MOV 	R4, #10						

	UDIV 	R2, R4 						 

	STR 	R2, [SP, #Pointer_D] 	

	B 		Find_Digit1 				

	

First_Found 

	MOV 	R0, R3 						

	ADD 	R0, #0x30 					

	MOV 	R9, R3 						

	B 		LOOP 						

	

First0 

	MOV 	R0, #0						

	ADD 	R0, #0x30					

	BL 		ST7735_OutChar 				

	B 		DONE						

	

LOOP

	PUSH	{R0,R11} 					

	BL 		ST7735_OutChar 				

	POP 	{R0,R11} 					

	MOV 	R3, R9 						

	LDR 	R2, [SP, #Pointer_D] 	

	LDR 	R0, [SP, #Pointer_In] 	

	MUL 	R5, R2, R3 					

	SUB 	R0, R0, R5 					

	STR 	R0,  [SP, #Pointer_In] 	

	MOV 	R6, #10						

	UDIV 	R2,R2, R6 					 

	CMP 	R2, #0 						

	BEQ		DONE 						

	UDIV 	R0, R0, R2 					

	STR 	R2,  [SP, #Pointer_D] 	

	MOV 	R9, R0 						

	ADD 	R0, #0x30 					

	B 		LOOP 						

	

DONE

	POP 	{R0,R1} 					

	POP 	{R4-R12, LR}				



      BX  LR ; end subroutine 

	  

;* * * * * * * * End of LCD_OutDec * * * * * * * *



; -----------------------LCD _OutFix----------------------

; Output characters to LCD display in fixed-point format

; unsigned decimal, resolution 0.001, range 0.000 to 9.999

; InputPointers:  R0 is an unsigned 32-bit number

; Outputs: none

; E.g., R0=0,    then output "0.000 "

;       R0=3,    then output "0.003 "

;       R0=89,   then output "0.089 "

;       R0=123,  then output "0.123 "

;       R0=9999, then output "9.999 "

;       R0>9999, then output "*.*** "

; Invariables: This function must not permanently modify registers R4 to R11

LCD_OutFix

	PUSH 	{R4-R12 ,LR} 					

	PUSH 	{R1, R0} 						

	STR 	R0, [SP, #Pointer_Fix] 		

	CMP 	R0, #0							

	BEQ 	ZeroDecimal 					

	

	MOV 	R3, #10											

	UDIV 	R2, R0, R3 						

	CMP 	R2, #0 							

	BEQ 	OneDigit						

	

	MOV 	R3, #100						

	UDIV 	R2, R0, R3 						

	CMP 	R2, #0							

	BEQ 	TwoDigit						

	

	MOV 	R3, #1000						

	UDIV 	R2, R0, R3 						

	CMP 	R2, #0							

	BEQ 	ThreeDigit						

	B 		FourDigit						

	

ZeroDecimal

	MOV 	R5, #3							

	MOV 	R0, #0							

	BL 		LCD_OutDec						

	MOV 	R0, #0x2E 						

	BL 		ST7735_OutChar  				

zeroloop	

	MOV 	R0, #0

	BL 		LCD_OutDec

	SUB 	R5, #1

	CMP 	R5, #0

	BNE 	zeroloop 						

	B 		Done 							

	

OneDigit

	LDR 	R7, [SP,#Pointer_Fix]		

	MOV 	R0, #0							

	BL 		LCD_OutDec						

	MOV 	R0, #0x2E ; decimal point		

	BL 		ST7735_OutChar 					

	MOV 	R0, #0								

	BL 		LCD_OutDec									

	BL 		LCD_OutDec						

	MOV 	R0, R7 							

	BL 		LCD_OutDec						

	B 		Done							

	

TwoDigit

	LDR 	R7, [SP,#Pointer_Fix]		

	MOV 	R0, #0						

	BL 		LCD_OutDec						

	MOV 	R0, #0x2E 						

	BL 		ST7735_OutChar 					

	MOV 	R0, #0							

	BL 		LCD_OutDec						

	MOV 	R0, R7						

	BL 		LCD_OutDec						

	B 		Done							

	

ThreeDigit

	LDR 	R0, [SP,#Pointer_Fix]

	MOV 	R7, R0 

	MOV 	R0, #0

	BL 		LCD_OutDec

	MOV 	R0, #0x2E 

	BL 		ST7735_OutChar 

	MOV 	R0, R7 

	BL 		LCD_OutDec

	B 		Done

	

FourDigit

	LDR 	R0, [SP,#Pointer_Fix]		

	MOV 	R2, #9000						

	ADD 	R2, #1000						

	CMP 	R0, R2							

	BCS 	Stars 							

	MOV 	R2, #1000						

	UDIV 	R7, R0, R2						

	MOV 	R0, R7							

	BL 		LCD_OutDec 						

	MOV 	R0, #0x2E 						

	BL 		ST7735_OutChar  				

	LDR 	R0, [SP, #Pointer_Fix]

	MOV 	R1, #1000

	MUL 	R7, R1

	SUB 	R0, R7 							

	BL 		LCD_OutDec 						

	B 		Done

	

	

Stars

	MOV R0, #0x2A; stars

	BL ST7735_OutChar  ;print the star out

	MOV R0, #0x2E ; decimal point

	BL ST7735_OutChar  

	MOV R0, #0x2A; stars

	BL ST7735_OutChar

	MOV R0, #0x2A; stars

	BL ST7735_OutChar

	MOV R0, #0x2A; stars

	BL ST7735_OutChar

	B Done

	



	

     

	 

Done	 

	MOV R0, #0

	POP {R1, R0}

	POP {R4- R12, LR} 

	 BX   LR 



     ALIGN

;* * * * * * * * End of LCD_OutFix * * * * * * * *



     ALIGN                           ; make sure the end of this section is aligned

     END                             ; end of file