#include "REG932.H"
#include "uart.h"

#define KEYBOARD 0
#define PLAY_SONG_1 1
#define PLAY_SONG_2 2
#define MAX_MODE 2
#define MAX_SONG_LENGTH 10

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

//makes it easier to make normal sounds
unsigned char overflowCount = 0;

unsigned char mode = KEYBOARD;

unsigned char song_overflow_count = 0; // Num used to count duration
unsigned char song_overflow_target = 0; // Num used to check overflow count to. Pre calculated
unsigned char song_location = 0;	//Current location in the song
unsigned char song_delay_counter = 0;	//counter to increment for note delay

const char* SONG_NAME_1 = "Example song 1\n\r";
const char* SONG_NAME_2 = "Different song\n\r";

const unsigned char songNotes[2][MAX_SONG_LENGTH]=
{
//song 1
{-50, -150, -200, -50, -150, -200},
//song 2
{-100, -200, -250, -255, -250, -160}
};

//each is 200 times as long as stated
const unsigned char noteLengths[2][MAX_SONG_LENGTH]=
{
//song 1
{-50, -50, -50, -100, -100, -100},
//song 2
{-50, -100, -200, -40, -80, -160}
};

const unsigned char songLengths[2]=
{
//song 1
6,
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
	unsigned char songIndex = 1;
	overflowCount++;
	if(overflowCount >= 20)
	{
		overflowCount = 0;
	    switch(mode)
		{
		 	case PLAY_SONG_1:
				songIndex = 0;
	        case PLAY_SONG_2:
				++song_delay_counter;	//lengthen delay
				if(song_delay_counter == 10)
				{
					song_delay_counter = 0;					
		            ++song_overflow_count;
	                if(song_overflow_count >= song_overflow_target)
				    {
					   song_overflow_count = 0;
	                   song_location++;
					   if(song_location == songLengths[songIndex])	//if at the end of the song
					   {
					   	  IE &= 0xFD;	//turn off timer
						  song_location = 0;	//move back to start of song
						  break;
					   }	
					   set_timer(songNotes[songIndex][song_location]);	//set timer to next note frequency
					   song_overflow_target = noteLengths[songIndex][song_location];	//set next note duration
	                }
				}
			break;
	
	        case KEYBOARD:
	           //speaker is being complemented below
	        break;
		}	   
		SPEAKER = ~SPEAKER; // Complement speaker no matter what
	}  
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

void serialTransmit(const char* string)
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

void main(void)
{
	int dummy;
    init();
    while(1)
    {
      if(!MODE_TOGGLE_BUTTON)
	  {
        increment_mode();
        update_interrupts();	
        update_lights();
		//Rebound delay
		for(dummy = 0; dummy < 1000; dummy++);
		// Wait until button up
		while(!MODE_TOGGLE_BUTTON); 
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
				song_overflow_target = noteLengths[0][0];	//first note length
				set_timer(songNotes[0][0]);	//first note freq
				serialTransmit(SONG_NAME_1);
			}
			else
			{
				song_overflow_target = noteLengths[1][0];	//first note length
				set_timer(songNotes[1][0]);	//first note freq
				serialTransmit(SONG_NAME_2);
			}	   
			IE |= 0x02; //tell song to start playing
			//Rebound delay
			for(dummy = 0; dummy < 1000; dummy++);
			// Wait until button up
			while(!PLAY_SONG);
		}											
		if(!STOP_SONG)
		{
			IE &= 0xFD;	//song stops playing
			song_location = 0; //reset location
		}
		if(!PAUSE_SONG)
		{
			IE &= 0xFD; //song stops playing
		}
	  }
    }
}

