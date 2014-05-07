#include "REG932.H"
#include "uart.h"

//mode 0 = play song, mode 1 = keyboard
unsigned char mode = 0;

void interrupt0(void) interrupt 1
{
	switch(mode)
	{
	 	case 0:	 //playing song
		//go to next tone if overflow number reached
		break;

		case 1:  //keyboard note
		//complement bit
		break;

		default: //unknown error
		//this should never be reached
		break;
	}
}

void main(void)
{
    while(1)
    {
		//check for buttons pressed and set mode acordingly
		//call function to turn off interrupts
		//turn on needed interrupts
		//if keyboard mode check for additional buttons
    }
}

void setTimer(unsigned char count)
{
   //sets timer to play frequency
}