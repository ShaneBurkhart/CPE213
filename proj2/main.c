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
                  // Don't need to do anything right now.  Complemented below.
                break;
	}
        SPEAKER = ~SPEAKER; // No matter what need to complement the speaker
}

void set_timer(unsigned char count)
{ //sets timer to play frequency
  TMOD |= 0x02;
  TH0 = count;
  TR0 = 1;
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
  set_timer(1); // Some arbitrary set to make sure timer is running
}

void update_lights()
{ // Updates the lights to show mode
  LED1 = 1;
}

void update_interrupts(){
  IE = 0x90; // Set interrupt to only the global enable and serial
  switch(mode)
  {
    case PLAY_SONG_1:
    case PLAY_SONG_2:
      IE |= 0x02; // Right now all need same interrupt but may need more for new modes.
    break; // Don't need to update keyboard since that is done in UI.
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
        if(!KEYBOARD_BUTTON_1)
        {
          set_timer(100);
        }else if(!KEYBOARD_BUTTON_1)
        {
          set_timer(150);
        }else if(!KEYBOARD_BUTTON_1)
        {
          set_timer(75);
        }
        if(!KEYBOARD_BUTTON_1 || !KEYBOARD_BUTTON_2 || !KEYBOARD_BUTTON_3) // If button pressed turn on interrupt
          IE |= 0x02;
        else // else turn it off
          IE &= 0xFD;
      }

      update_lights();
    }
}

