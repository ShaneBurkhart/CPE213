#include <reg932.inc>

  incBit   equ P2.0 			;switch 1
  decBit   equ P0.1				;switch 4
  LEDbit0  equ P0.6				;LED 4 [amber]/bit 0
  LEDbit1  equ P2.7				;LED 3 [green]/bit 1
  LEDbit2  equ P0.5				;LED 2 [yellow]/bit 2
  LEDbit3  equ P2.4				;LED 1 [red]/bit 3
  speaker  equ P1.7				;Speaker
  dice	   equ P1.4				;Toggle dice mode  [Switch 5]
  songButton  equ P2.2				;Play song mode [Switch 9]

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

  sixteenthNotes: ds 1
  notePos: ds 1

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
	inc count				    ;increment count
	mov A, count			    ;load count to check for overflow
	clr c						;clear carry for subtract
	subb A, compareValue		;see if they are equal
	jnz skipSound1				;don't play the sound if not equal
		setb playSound			;say to play the sound
		mov count, #0			;set count to 0
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
		mov compareValue, #16	;Set value to overflow at 16
		mov resetValue, #15		;Value to reset on underflow
		mov count, #0			;reset counter
		acall blackOut			;update lights for new count
		sjmp holdButton			;hold until button is released
setDice:
		setb diceMode			;Dice mode on
		mov compareValue, #7	;overflow at 7
		mov resetValue, #6		;re-roll to 6
		mov count, #0			;reset counter
		acall blackOut			;update lights for new count
holdButton:
		jnb dice, holdButton	;stay until the button is released
skipSwap:
    jb songButton, skipSong
    jb diceMode, skipSong
    acall theFinalCountdown
skipSong:
	sjmp loop					;loop back

updateLights:
	jb diceMode, diceLights		;jump to dice mode if it is enabled
		mov A, count			;load count for rotating
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
		rrc A					;rotate bit into c
		mov A, count			;load count into A
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

blackOut:				 		;turn off all the lights
	setb LED1					;LED1...
	setb LED2
	setb LED3
	setb LED4
	setb LED5
	setb LED6
	setb LED7
	setb LED8
	setb LED9					;...LED9
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
;                     0,         1,         2,		   3,		  4,		 5,		    6
diceArray: db 11111111b, 11101111b, 11111110b, 11101110b, 10111010b, 10101010b, 10010010b

;LED 1
;                0, 1, 2, 3, 4,	5, 6
dicebitArray: db 1, 1, 0, 0, 0, 0, 0

theFinalCountdown:                      ;Play song
  mov DPTR, #theFinalCountdownNotes     ;move array start into DPTR
  mov A, #0
playNoteLoop:
  mov A, notePos
  movc A, @A + DPTR                     ;Read in freq coefficient
  mov timerValue, A
  inc notePos                           ;Go to next byte
  mov A, notePos
  movc A, @A + DPTR                     ;Read in number of sixteenth notes
  mov sixteenthNotes, A
  inc notePos
  orl A, timerValue                     ;Check if both sixteenthNotes and timerValue are zero. Sixteenth already there.
  jz playNoteReturn
  acall playNote                        ;Play notes
  sjmp playNoteLoop                     ;Jump to top of loop
playNoteReturn:
  ret

playNote: ;New makeSound function.  Not clean but easier than trying to modify code and potentially breaking someone else's stuff
          ;If given a 1, it plays a 16th note.  The coefficient will be how many 16th notes to play.  4 would be a quarter note.
          ;timerValue needs to be set and is satisfies this equation: timerValue = 255-some_number ;;; ticks_per_half_period = 50 * some_number
          ;quarterNotes needs to be set and is the number of quarter notes you want to play.
  mov TMOD, #00000010b		        ;Timer 0 = 8-bit auto reload
  mov TH0, timerValue			;load reload value.
  mov TL0, timerValue			;load default value
  setb TR0				;turn on timer 0
  clr TF0				;clear timer 0 overflow

  noteLoop:
    mov r5, #0
    outerNoteLoop:
      mov r7, #0			;The delay for a 16th will be 256 * 256 * 14 at 2 beats per second
      innerNoteLoop:
        mov r6, #14			;Will decrement first so 256 cycles
        freqLoop:
          mov r0, #50			;Coefficient to for freq.  Ticks_for_half_period = 50 * some_number
          timerLoop:
            jnb TF0, timerLoop	        ;delay until timer overflows
            clr TF0			;clear timer overflow
            djnz r0, timerLoop	        ;jump back to the delayLoop the proper number of times
          cpl speaker			;flip the speaker
          djnz r6, freqLoop		;First 14
        djnz r7, innerNoteLoop		;Second 256
      djnz r5, outerNoteLoop		;Third 256
    djnz sixteenthNotes, noteLoop         ;Number of quarter not loops

  clr TR0				;done with the timer; turn it off
  ret					;return


theFinalCountdownNotes:
;Measure 18
  db 1, 180
  db 4, 122
  db 4, 56
;Measure 19
  db 6, 255
  db 1, 130
  db 1, 122
  db 1, 130
  db 1, 255
  db 1, 114
  db 1, 255
  db 4, 180
;Measure 20
  db 2, 180
  db 4, 255
  db 1, 130
  db 1, 122
  db 4, 130
  db 4, 56
;Measure 21
  db 6, 255
  db 1, 180
  db 1, 171
  db 1, 180
  db 1, 255
  db 1, 171
  db 1, 255
  db 2, 78
  db 1, 180
  db 1, 255
;Measure 22
  db 6, 171
  db 1, 78
  db 1, 171
  db 6, 180
  db 1, 171
  db 1, 180
;Measure 23
  db 2, 122
  db 2, 180
  db 2, 171
  db 2, 78
  db 4, 56
  db 4, 130
;Measure 24
  db 9, 122
  db 2, 114
  db 3, 130
  db 1, 114
  db 1, 180
;Measure 25
  db 12, 122
  db 4, 255
;Ending Note
  db 0, 0


end
