#include "REG932.H"
#include "uart.h"

#define KEYBOARD 0
#define PLAY_SONG_1 1
#define PLAY_SONG_2 2
#define MAX_MODE 2
#define MAX_SONG_LENGTH 26

//not defined by default for some reason
sfr IE = 0xA8;

// Speaker
sbit SPEAKER = P1^7;

// Buttons
sbit MODE_TOGGLE_BUTTON = P2^2;	   //Switch 9
sbit KEYBOARD_BUTTON_1= P2^0;	   //Switch 1
sbit KEYBOARD_BUTTON_2= P0^1;	   //Switch 2
sbit KEYBOARD_BUTTON_3= P2^3;	   //Switch 3

sbit PLAY_SONG = P2^0;			   //Switch 1
sbit STOP_SONG = P0^1;			   //Switch 2
sbit PAUSE_SONG = P2^3;			   //Switch 3

// LEDs
sbit LED1 = P2^4;
sbit LED2 = P0^5;

unsigned char mode = KEYBOARD;

int dummy;

unsigned char song_location = 0;	//Current location in the song
unsigned char song_delay_counter = 0;	//counter to increment for note delay
unsigned char current_note_length = 0;
unsigned char freq_multiplier = 20; // Count to allow for longer freq delays
unsigned char song_index = 1;

code const char* const SONG_NAME_1 = "Mary had a little lamb.\n\r";
code const char* const SONG_NAME_2 = "Different song\n\r";

code const unsigned char song_notes[2][MAX_SONG_LENGTH]=
{
//song 1
{69, 47, 21, 47, 69, 69, 69, 47, 47, 47, 69, 99, 99, 69, 47, 21, 47, 69, 69, 69, 69, 47, 47, 69, 47, 21},
//song 2
{-100, -200, -250, -255, -250, -160}
};

//each is 150 times as long as stated
code const unsigned char note_lengths[2][MAX_SONG_LENGTH]=
{
//song 1
{25, 28, 31, 28, 25, 25, 50, 28, 28, 56, 25, 21, 42, 25, 28, 31, 28, 25, 25, 25, 25, 28, 28, 25, 28, 125},
//song 2
{20, 30, 40, 50, 55, 60}
};

code const unsigned char song_lengths[2]=
{
//song 1
26,
//song 2
6,
};

void set_timer(unsigned char count)
{ //sets timer to play frequency
  TMOD |= 0x02;
  TH0 = count;
  TR0 = 1;
}

void interrupt0(void) interrupt 1
{
  freq_multiplier--; // Do this up here so not another nested
  if(freq_multiplier != 0)
    return;

  freq_multiplier = 20;
  song_index = 1;
  switch(mode)
  {
    case PLAY_SONG_1:
      song_index = 0;
    case PLAY_SONG_2:
      ++song_delay_counter;	//lengthen delay
      if(song_delay_counter != 7) // Some multiplier. Avoiding nesting again.
        break; // Break so can complement

      song_delay_counter = 0;

      current_note_length--;
      if(current_note_length != 0) // Checks if note is done.
        break; // Break to complement.

      song_location++;
      if(song_location == song_lengths[song_index])	//if at the end of the song
      {
        IE &= 0xFD;	//turn off timer interrupt
        break;
      }

      set_timer(song_notes[song_index][song_location]);	//set timer to next note frequency
      current_note_length = note_lengths[song_index][song_location]; //set next note duration
      break;
    case KEYBOARD:
       //speaker is being complemented below
    break;
  }

  SPEAKER = ~SPEAKER; // Complement speaker no matter what
}

void increment_mode()
{
  song_location = 0; //reset this to normal value
  ++mode;
  if(mode > MAX_MODE) // should be highest mode
    mode = 0;
}

void init()
{
  P2M1 = 0x00;
  P1M1 = 0x00;
  P0M1 = 0x00;
  set_timer(1); // Some arbitrary set to make sure timer is running
  IE = 0x90;  //Enable interrupts by default
}

void update_lights()
{ // Updates the lights to show mode
  LED1 = !(mode>>1);  //2nd bit of mode
  LED2 = !(mode%2);	  //lowest bit of mode
}

void update_interrupts()
{
  IE = 0x90; // Set interrupt to only the global enable and serial
  //other interrupts are not needed
}

void serial_transmit(const char* string)
{
	unsigned char i;
    unsigned char length = 0;
    uart_init();
	for(length = 0; string[length] != 0; length++);  //get the length of the string
	for(i = 0; i < length; i++)
	{
		uart_transmit(string[i]);
	}
}

void start_song(int song_index)
{
  song_location = 0;
  current_note_length = note_lengths[song_index][0];	//first note length
  set_timer(song_notes[song_index][0]);	//first note freq
}

void main(void)
{
    init();

    while(1)
    {
      if(!MODE_TOGGLE_BUTTON)
	  {
        increment_mode();
        update_interrupts();
        update_lights();
        do{ for(dummy = 0; dummy < 1000; dummy++); }while(!MODE_TOGGLE_BUTTON); // Wait until button up
      }

      if(mode == KEYBOARD)
      {
        if(!KEYBOARD_BUTTON_1)
        {
          set_timer(-100);
        }
		else if(!KEYBOARD_BUTTON_2)
        {
          set_timer(-150);
        }
		else if(!KEYBOARD_BUTTON_3)
        {
          set_timer(-75);
        }
        if(!KEYBOARD_BUTTON_1 || !KEYBOARD_BUTTON_2 || !KEYBOARD_BUTTON_3) // If button pressed turn on interrupt
		{
          IE |= 0x02;
		}
        else // else turn it off
		{
          IE &= 0xFD;
		}
      }
	  else
	  {
	  	if(!PLAY_SONG && ((IE & 0x02) == 0))  //only start playing song if it hasn't started
		{
			if(mode == PLAY_SONG_1)
			{
                start_song(0);
                serial_transmit(SONG_NAME_1);
			}
			else
			{
                start_song(1);
                serial_transmit(SONG_NAME_2);
			}
			IE |= 0x02; //tell song to start playing
			for(dummy = 0; dummy < 1000; dummy++);
            while(!PLAY_SONG); // Wait until button up
		}
		if(!STOP_SONG)
		{
			IE &= 0xFD;	//song stops playing
			song_location = 0; //reset location
            for(dummy = 0; dummy < 1000; dummy++);
			while(!STOP_SONG); // Wait until button up
		}
		if(!PAUSE_SONG)
		{
			IE &= 0xFD; //song stops playing
            for(dummy = 0; dummy < 1000; dummy++);
			while(!PAUSE_SONG); // Wait until button up
		}
	  }
    }
}

