#include "REG932.H"
#include "uart.h"

#define KEYBOARD 0
#define PLAY_SONG_1 1
#define PLAY_SONG_2 2
#define EFFECT 3
#define MAX_MODE 3
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

sbit METER_0 = P3^1;
sbit METER_1 = P3^0;
sbit METER_2 = P1^4;
sbit METER_3 = P0^2;
sbit METER_4 = P0^0;



// LEDs
sbit LED1 = P2^4;
sbit LED2 = P0^5;
sbit LED3 = P2^7;
sbit LED4 = P0^6;
sbit LED5 = P1^6;
sbit LED6 = P0^4;
sbit LED7 = P2^5;
sbit LED8 = P0^7;
sbit LED9 = P2^6;

unsigned char mode = KEYBOARD;

int dummy;

unsigned char song_location = 0;	//Current location in the song
unsigned char song_delay_counter = 0;	//counter to increment for note delay
unsigned char current_note_length = 0;
unsigned char freq_multiplier = 20; // Count to allow for longer freq delays
unsigned char song_index = 1;
unsigned char lyric_char_index = 0;
unsigned char current_lyric_length;
unsigned char effect_index = 0;
unsigned char effect_length;
unsigned char is_effecting = 0;
unsigned char effect_direction = 1;


unsigned char teach_index = 0;

code const char* const SONG_NAME_1 = "Mary Had a Little Lamb\n\r";
code const char* const SONG_NAME_2 = "Hot Cross Buns\n\r";

code const unsigned char song_notes[2][MAX_SONG_LENGTH]=
{
//song 1
{69, 47, 21, 47, 69, 69, 69, 47, 47, 47, 69, 99, 99, 69, 47, 21, 47, 69, 69, 69, 69, 47, 47, 69, 47, 21},
//song 2
{69,47,21,69,47,21,21,21,21,21,47,47,47,47,69,47,21}
};

//each is 140 times as long as stated
code const unsigned char note_lengths[2][MAX_SONG_LENGTH]=
{
//song 1
{25, 28, 31, 28, 25, 25, 50, 28, 28, 56, 25, 21, 42, 25, 28, 31, 28, 25, 25, 25, 25, 28, 28, 25, 28, 125},
//song 2
{50,56,125,50,56,125,31,31,31,31,28,28,28,28,50,56,125}
};

code const char* song_lyrics[2][MAX_SONG_LENGTH]=
{
{"Ma", "ry", " had", " a", " lit", "tle", " lamb,", " lit", "tle", " lamb,", " lit", "tle", " lamb.\r\n", "Ma", "ry", " had", " a" , " lit","tle", " lamb.", " Its", " fleece", " was", " white", " as", " snow.\r\n"},
{"Hot", " cross", " buns.", " Hot", " cross", " buns.", " One", " a", " pen", "ny,", " two", " a", " pen", "ny.", " Hot", " cross", " buns.\r\n"}
};

code const unsigned char song_lengths[2]=
{
//song 1
26,
//song 2
17,
};

code const unsigned char effect_tones[9] =
{
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,
	200
};

void set_timer(unsigned char count)
{ //sets timer to play frequency
  TMOD |= 0x02;
  TH0 = count;
  TR0 = 1;
}

void interrupt0(void) interrupt 1
{
  short seperate_notes = 0;
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

	  if(lyric_char_index < current_lyric_length)	//transmit lyrics
	  {
	  	uart_transmit(song_lyrics[song_index][song_location][lyric_char_index]);
		lyric_char_index++;
	  }

      song_delay_counter = 0;

      current_note_length--;
      if(current_note_length != 0) // Checks if note is done.
        break; // Break to complement.

	  for(seperate_notes = 0; seperate_notes<10000; seperate_notes++);

      song_location++;		   //move to next note
      if(song_location == song_lengths[song_index])	//if at the end of the song
      {
        IE &= 0xFD;	//turn off timer interrupt
        break;
      }
	  lyric_char_index = 0;	   //reset lyric location
	  for(current_lyric_length = 0; song_lyrics[song_index][song_location][current_lyric_length] != 0; current_lyric_length++);	//get lyric length

      set_timer(song_notes[song_index][song_location]);	//set timer to next note frequency
      current_note_length = note_lengths[song_index][song_location]; //set next note duration
      break;
    case KEYBOARD:
       //speaker is being complemented below
    break;
	case EFFECT:
		effect_length--;
		if(effect_length == 0)
		{
			effect_index += effect_direction;
			if(effect_index == 8) effect_direction = -1;
			if(effect_index == 0) effect_direction = 1;
			set_timer(effect_tones[effect_index]);
			effect_length = 200 / effect_tones[effect_index] * 50;
		}
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
  P3M1 = 0x00;
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
  lyric_char_index = 0;	   //reset lyric location
  for(current_lyric_length = 0; song_lyrics[song_index][0][current_lyric_length] != 0; current_lyric_length++);	//get lyric length
  song_location = 0;
  current_note_length = note_lengths[song_index][0];	//first note length
  set_timer(song_notes[song_index][0]);	//first note freq
}

void update_freq_lights()
{
	if((IE & 0x02) == 0)  //if timer interrupt is off
	{
		METER_0 = 1;
		METER_1 = 1;
		METER_2 = 1;
		METER_3 = 1;
		METER_4 = 1;
		return;
	}

	if(TH0 > 20)
		METER_0	= 0;
	else
		METER_0 = 1;

	if(TH0 > 35)
		METER_1 = 0;
	else
		METER_1 = 1;

	if(TH0 > 50)
		METER_2 = 0;
	else
		METER_2 = 1;

	if(TH0 > 65)
		METER_3 = 0;
	else
		METER_3 = 1;

	if(TH0 > 80)
		METER_4 = 0;
	else
		METER_4 = 1;
}

void keyboard_input()
{
	if(!KEYBOARD_BUTTON_1)
    {
      set_timer(69);
    }
	else if(!KEYBOARD_BUTTON_2)
    {
      set_timer(47);
    }
	else if(!KEYBOARD_BUTTON_3)
    {
      set_timer(21);
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

void failed_noise()
{
	set_timer(200);
	IE |= 0x02;
	for(dummy = 0; dummy < 10000; dummy++);
	IE &= 0xFD;
}

void success_noise()
{
	set_timer(100);
	IE |= 0x02;
	for(dummy = 0; dummy < 10000; dummy++);
	IE &= 0xFD;
}

void turn_off_lights()
{
	LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1;	LED5 = 1;
	LED6 = 1; LED7 = 1; LED8 = 1; LED9 = 1;
}

void effect_lights()
{
	turn_off_lights();
	switch(effect_index){
		case 0: LED1 = 0; break;
		case 1: LED2 = 0; break;
		case 2: LED3 = 0; break;
		case 3: LED4 = 0; break;
		case 4: LED5 = 0; break;
		case 5: LED6 = 0; break;
		case 6: LED7 = 0; break;
		case 7: LED8 = 0; break;
		case 8: LED9 = 0; break;
	}
}

void main(void)
{
    init();

    while(1)
    {
	  update_freq_lights();
      if(!MODE_TOGGLE_BUTTON)
	  {
	  	is_effecting = 0;
        increment_mode();
        update_interrupts();
		turn_off_lights();
        update_lights();
        for(dummy = 0; dummy < 1000; dummy++);
		while(!MODE_TOGGLE_BUTTON); // Wait until button up
      }

      if(mode == KEYBOARD)
      {
        keyboard_input();
      }
	  else if(mode == EFFECT)
	  {
	  	if(is_effecting){
			effect_lights();
		}
	  	if(!PLAY_SONG)
		{
			is_effecting = 1;
			effect_index = 0;
			effect_direction = 1;
			effect_length = 200 / effect_tones[effect_index] * 50;
			set_timer(effect_tones[effect_index]);
			for(dummy = 0; dummy < 1000; dummy++);
			while(!PLAY_SONG); // Wait until button up
			IE |= 0x02;
		}
	  }
	  else
	  {
	  	if(!PLAY_SONG && ((IE & 0x02) == 0))  //only start playing song if it hasn't started
		{
			if(mode == PLAY_SONG_1)
			{  
                serial_transmit(SONG_NAME_1);
                start_song(0);
			}
			else
			{					 
                serial_transmit(SONG_NAME_2);
                start_song(1);
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

