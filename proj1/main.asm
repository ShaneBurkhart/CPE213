#include <reg932.inc>

  incBit   equ P2.0 			;switch 1
  decBit   equ P0.1				;switch 4
  LEDbit0  equ P0.6				;LED 4 [amber]/bit 0
  LEDbit1  equ P2.7				;LED 3 [green]/bit 1
  LEDbit2  equ P0.5				;LED 2 [yellow]/bit 2
  LEDbit3  equ P2.4				;LED 1 [red]/bit 3
  speaker  equ P1.7				;Speaker
  dice	   equ P1.4				;Toggle dice mode
  
  LED1	   equ P2.4				;LED1
  LED2	   equ P0.5				;LED2
  LED3	   equ P2.7				;LED3
  LED4	   equ P0.6				;LED4
  LED5	   equ P1.6				;LED5
  LED6     equ P0.4			    ;LED6
  LED7	   equ P2.5				;LED7
  LED8	   equ P0.7				;LED8
  LED9	   equ P2.6				;LED9

  cseg at 0
	ljmp main 					;jump to main functionality
	
  mydata segment data
  rseg mydata
  
  count: ds 1
  timerValue: ds 1
  compareValue: ds 1
  resetValue: ds 1

  mybits segment bit
  rseg mybits

  playSound: dbit 1
  diceMode:  dbit 1
	
  prog segment code
  rseg prog

main:
	mov P2M1,  #0				;set port modes to bi-directional
	mov P1M1,  #0
	mov P0M1,  #0
	mov count, #0				;initalize count
	mov SP, #5Fh				;Set up stack pointer
	mov timerValue, #256-207	;Set inital timer value for 742Hz
	clr diceMode				;Dice mode off by default
	mov compareValue,#16		;Set value to overflow at
	mov resetValue,#15			;Value to reset on underflow
loop:
	clr playSound				;reset the sound bit
	jb incBit, skipIncrease		;skip increasing if button is not pressed
	mov A, count				;load count
	add A, #1					;add one; don't use inc as it doesn't set AC
	mov count, A				;put value back into count
	clr c						;clear carry for subtract
	subb A, compareValue		;see if they are equal
	jnz skipSound1				;don't play the sound if not equal
		setb playSound			;say to play the sound
		mov count, 0			;set count to 0
skipSound1:
	acall updateLights			;update lights
	jnb playSound, doneSound	;skip sound if there isn't overflow
		acall makeSound			;make the sound
doneSound:
		jnb incBit, doneSound	;stay until the button is released
skipIncrease:
	jb decBit, skipDecrease		;skip decreasing if button is not pressed
	mov A, count				;load count
	clr c						;clear carry for subb
	subb A, #1					;add one
	mov count, A				;put value back into count 
	jnc skipSound2				;don't play the sound unless there's overflow
		setb playSound			;say to play the sound
		mov count,resetValue	;Reload the counter
skipSound2:
	acall updateLights			;update lights
	jnb playSound, doneSound2	;skip sound if there isn't overflow
		acall makeSound			;make the sound		
doneSound2:
		jnb decBit, doneSound2	;This is to prevent one button press to count as many
skipDecrease:
	jb dice, skipSwap			;skip swaping modes if button isn't pushed
		jnb diceMode, setDice	;set mode to dice mode
		clr diceMode			;Dice mode off
		mov compareValue, #16	;Set value to overflow at
		mov resetValue, #15		;Value to reset on underflow
		mov count, #0			;reset counter
		sjmp holdButton			;hold until button is released
setDice:
		setb diceMode			;Dice mode on
		mov compareValue, #7	;overflow at 7
		mov resetValue, #6		;re-roll to 6
		mov count, #0			;reset counter
holdButton:
		jnb dice, holdButton	;stay until the button is released
skipSwap:
	sjmp loop					;loop back
	
updateLights:
	jb diceMode, diceLights		;jump to dice mode if it is enabled
		mov count, A			;load count for rotating
		rrc A					;put zeroth bit into carry
		cpl c					;active low
		mov LEDbit0, c			;light LED0 if set
		rrc A					;1st bit		  
		cpl c					;active low
		mov LEDbit1, c			;LED 1
		rrc A					;2nd bit
		cpl c					;active low
		mov LEDbit2, c			;LED 2
		rrc A					;3rd bit [last bit]
		cpl c					;active low
		mov LEDbit3, c			;LED 3
		ret
diceLights:
		mov A, count			;move count into A
		mov DPTR, #dicebitArray	;move array start into DPTR
		movc A, @A + DPTR		;load bit into a
		rlc A					;rotate bit into c
		mov DPTR, #diceArray 	;move array start into DPTR
		movc A, @A + DPTR		;load value into A
		mov LED1, c				;light LED1 if needed
		rrc A					;get lowest bit into A
		mov LED9, c				;LED9
		rrc A					;same thing
		mov LED8, c				;repeat for all remaining LED's...
		rrc A
		mov LED7, c
		rrc A
		mov LED6, c
		rrc A
		mov LED5, c
		rrc A
		mov LED4, c
		rrc A
		mov LED3, c
		rrc A
		mov LED2, c				;done
		ret
		

makeSound:
	mov TMOD, #00000010b		;Timer 0 = 8-bit auto reload
	mov TH0, timerValue			;load reload value
	mov TL0, timerValue			;load default value
	setb TR0					;turn on timer 0
	clr TF0						;clear timer 0 overflow
	mov r7, #5					;loop for a while
startLoop:
		mov r6, #0				;inner loop to make sound last longer
innerLoop:
			mov r0, #24			;load number of times to loop for the frequency
delayLoop:
			jnb TF0, delayLoop	;delay until timer overflows
			clr TF0				;clear timer overflow
			djnz r0, delayLoop	;jump back to the delayLoop the proper number of times
		cpl speaker				;flip the speaker
		djnz r6, innerLoop		;make the sound last longer [inner]
	djnz r7, startLoop			;make the sound last longer [outer]
	clr TR0						;done with the timer; turn it off
	ret							;return

;LED's 2-9 [9 is least significant, 1 is most signficant
diceArray: db 11101111b, 11111110b, 11101110b, 10111010b, 10101010b, 10010010b
	
;LED 1
dicebitArray: db 11111111b, 11111110b, 11111110b, 11111110b, 11111110b, 11111110b
	
end
