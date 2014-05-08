#include "REG932.H"
#include "uart.h"

#define KEYBOARD 0
#define PLAY_SONG_1 1
#define PLAY_SONG_2 2

// Speaker
sbit SPEAKER = P1^7;

// Buttons
sbit MODE_TOGGLE_BUTTON = P2^2;
//sbit KEYBOARD_BUTTON_1= P2^2;
//sbit KEYBOARD_BUTTON_2= P2^2;
//sbit KEYBOARD_BUTTON_3= P2^2;

// LEDs
sbit LED1 = P2^4;

unsigned char mode = KEYBOARD;

unsigned char song_overflow_count = 0; // Num used to count duration
unsigned char song_overflow_target = 0; // Num used to check overflow count to. Pre calculated

void interrupt0(void) interrupt 1
{
        switch(mode)
	{
	 	case PLAY_SONG_1:
                case PLAY_SONG_2:
	          ++song_overflow_count;
                  if(song_overflow_count >= song_overflow_target){
                    // Go to next note
                  }
		break;

                case KEYBOARD:
                  // Don't need to do anything right now
                break;
	}
        SPEAKER = ~SPEAKER; // No matter what need to complement the speaker
}

void setTimer(unsigned char count)
{ //sets timer to play frequency
  TMOD |= 0x20;
  TH0 = count;
}

void increment_mode()
{
  ++mode;
  if(mode > PLAY_SONG_2) // should be highest mode
    mode = 0;
}

void init()
{
  P2M1 = 0x00;
  P1M1 = 0x00;
  P0M1 = 0x00;
}

void update_lights()
{ // Updates the lights to show mode
  LED1 = 1;
}

void update_interrupts(){
  // Set interrupt to only the global enable and serial
  switch(mode)
  {
    case PLAY_SONG_1:
    case PLAY_SONG_2:
    case KEYBOARD:
      IE |= 0x02; // Right now all need same interrupt but may need more for new modes.
    break;
  }
}

void main(void)
{
    init();
    while(1)
    {
      if(!MODE_TOGGLE_BUTTON){
        do{ for(int i = 0; i < 1000; ++i); }while(!MODE_TOGGLE_BUTTON); // Wait until button up
        increment_mode();
        update_interrupts();
      }

      if(mode == KEYBOARD)
      {

      }

      update_lights();
    }
}

