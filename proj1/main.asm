#include <reg932.inc>

  incBit   equ P2.0 			;switch 1
  decBit   equ P0.1				;switch 4
  LEDbit0  equ P0.6				;LED 4 [amber]/bit 0
  LEDbit1  equ P2.7				;LED 3 [green]/bit 1
  LEDbit2  equ P0.5				;LED 2 [yellow]/bit 2
  LEDbit3  equ P2.4				;LED 1 [red]/bit 3
  speaker  equ P1.7				;Speaker

  cseg at 0
	ljmp main 					;jump to main functionality
	
  mydata segment data
  rseg mydata
  
  count: ds 1
  timerValue: ds 1

  mybits segment bit
  rseg mybits

  playSound: dbit 1
	
  prog segment code
  rseg prog

main:
	mov P2M1,  #0				;set port modes to bi-directional
	mov P1M1,  #0
	mov P0M1,  #0
	mov count, #0				;initalize count
	mov SP, #5Fh				;Set up stack pointer
	mov timerValue, #256-207	;Set inital timer value for 742Hz
loop:
	jb incBit, skipIncrease		;skip increasing if button is not pressed
	mov A, count				;load count
	add A, #1					;add one; don't use inc as it doesn't set AC
	mov count, A				;put value back into count
	mov c, AC 					;store first into carry
	mov playSound, c			;store overflow for later
	lcall updateLights			;update lights
	jnb playSound, doneSound	;skip sound if there isn't overflow
		lcall makeSound			;make the sound
doneSound:
		jnb incBit, doneSound		;stay until the button is released
skipIncrease:
	jb decBit, skipDecrease		;skip decreasing if button is not pressed
	mov A, count				;load count
	clr c						;clear carry for subb
	subb A, #1					;add one
	mov count, A				;put value back into count 
	mov c, AC					;store first into carry
	mov playSound, c			;store overflow for later
	lcall updateLights			;update lights
	jnb playSound, doneSound2	;skip sound if there isn't overflow
		lcall makeSound			;make the sound		
doneSound2:
		jnb decBit, doneSound2	;This is to prevent one button press to count as many
skipDecrease:
	sjmp loop					;loop back
	
updateLights:
	mov count, A				;load count for rotating
	rrc A						;put zeroth bit into carry
	cpl c						;active low
	mov LEDbit0, c				;light LED0 if set
	rrc A						;1st bit		  
	cpl c						;active low
	mov LEDbit1, c				;LED 1
	rrc A						;2nd bit
	cpl c						;active low
	mov LEDbit2, c				;LED 2
	rrc A						;3rd bit [last bit]
	cpl c						;active low
	mov LEDbit3, c				;LED 3
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
	
	
end
