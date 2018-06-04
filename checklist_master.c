#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "io.h"
#include "timer.h"
#include "usart_ATmega1284.h"
#include "tests.h"



// update LCD to reflect the values taken in from checking the sensors
void LCD_checklist(unsigned char sensorFlag){

  LCD_ClearScreen();
  // takes in char and sets display based on what the flag is showing

  // Messages to output
  const char *bathroom_msg = "Light:            On"; // 20 characters a line
  const char *coffee_msg = "Socket:   Plugged In";
  const char *lock_msg = "Door:       Unlocked";
	const char *OK_msg = "    Good to go!";

  // Add message strings to output string if appropriate flags are found
  char output_msg[100];
  output_msg[0] = '\0';
	
	if((sensorFlag & 0x07) == 0x00){
		strcat(output_msg,OK_msg);
	}
	else{
		if((sensorFlag &0x01) == 0x01){ // light flag
			strcat(output_msg,bathroom_msg);
		}
		if((sensorFlag & 0x02) == 0x02){ // socket flag
			strcat(output_msg,coffee_msg);
		}
		if((sensorFlag & 0x04) == 0x04){ // lock flag
			strcat(output_msg,lock_msg);
		}
	}
  // output_msg is copied onto lcd_output which is what the LCD will display
  const unsigned char lcd_output[100];
  strcpy(lcd_output,output_msg);
  LCD_DisplayString(1, lcd_output);
  
  // After an iteration of LCD, output_msg and lcd_output must be cleared
  output_msg[0] = '\0';
  strcpy(lcd_output,output_msg);
  
  
  return;
}

// Display Ask prompt on LCD based on the sensor flags
void LCD_ask(unsigned char sensorFlag){
	LCD_ClearScreen();
	const char *ask_msg = "Do you want to      ";
	const char *light_msg = "turn off the light  ";
	const char *lock_msg = "lock the door       ";
	const char *prompt_msg = "Left: Yes  Right: No";
	
	char output_msg[100];
	output_msg[0] = '\0';
	
	strcat(output_msg,ask_msg);
	
	
	if((sensorFlag & 0x01) == 0x01){
		strcat(output_msg,light_msg);
	}
	if((sensorFlag & 0x04) == 0x04){
		strcat(output_msg,lock_msg);
	}
	strcat(output_msg,prompt_msg);
	const unsigned char lcd_output[100];
	strcpy(lcd_output,output_msg);
	LCD_DisplayString(1, lcd_output);
	output_msg[0] = '\0';
	strcpy(lcd_output,output_msg);
	
	
	return;
}


enum States {init_state, idle_state, press_state, update_state, ask_state, off_state, press_yes,press_no} state;
char status_flag;
char light_flag;
char socket_flag;
char lock_flag;
void tick(){
	
	
	char ask_flag;
	// char test_flag; // can be set to buttons in order to test if functions are working appropriately
	unsigned char buttonPress;
	
	static int iterations = 40; // iterations * timerSet = time spent waiting in update state (should be about a second)
	
	switch(state){ // transitions
		case init_state:
			LCD_DisplayString(1, "Press Button to     check sensors.");
			state = idle_state;
			break;
		case idle_state:
			buttonPress = ~PINA;
			if(buttonPress == 0x01){
				state = press_state;
				status_flag = 0x00;
				light_flag = 0x00;
				socket_flag = 0x00;
				lock_flag = 0x00;
				ask_flag = 0x00;
			}
			break;
		case press_state:
			buttonPress = ~PINA;
			if(buttonPress == 0x00){
					state = update_state;
			}
			break;
		case update_state:
			break;
		case ask_state:
			buttonPress = ~PINA;
			if((buttonPress & 0x02) == 0x02){
				state = press_yes;
			}
			else if((buttonPress & 0x04) == 0x04){
				state = press_no;
			}
			break;
		case off_state:
			break;
		case press_yes:
			buttonPress = ~PINA;
			if(buttonPress == 0x00){
				state = off_state;
			}
			break;
		case press_no:
			state = idle_state;
			break;
		default: break;
	}// transitions
	
	switch(state){// state actions
		case init_state:
			break;
		case idle_state:
			break;
		case press_state:
			break;
		case update_state:
			updateALL();
			_delay_ms(250);
			status_flag = light_flag | socket_flag | lock_flag;
			if((status_flag&0x01) == 0x01){
				LCD_ask(status_flag);
				state = ask_state;
				break;
			}
			if((status_flag&0x04) == 0x04){
				LCD_ask(status_flag);
				state = ask_state;
				break;
			}
			LCD_checklist(status_flag);
			state = idle_state;
			break;
		case ask_state:
			break;
		case off_state:
			LCD_ClearScreen();
			LCD_DisplayString(1, "...Sending signal...");
			_delay_ms(250);
			if(USART_IsSendReady(0)){		// 0x10 signal means for light to turn off 
				USART_Send(0x10, 0);
				while (!USART_HasTransmitted(0));
			}
			_delay_ms(250);
			if(USART_IsSendReady(0)){		// 0x20 signal means for lock to lock
				USART_Send(0x20, 0);
				while (!USART_HasTransmitted(0));
			}
			LCD_DisplayString(1, "Successfully turned off/closed");
			state = idle_state;
			break;
		case press_yes:
			break;
		case press_no:
			LCD_checklist(status_flag);
			break;
		default: break;
	} // state actions
}

// function to check all sensors and update flags
void updateALL(){
	_delay_ms(200);
	if(USART_IsSendReady(0)){ // signal to sensor 1 and receive from it
		USART_Send(0x01, 0);
		while (!USART_HasTransmitted(0));
		_delay_ms(40);
		if(USART_HasReceived(0)){ // if checklist has received any data, then update flag
			light_flag = USART_Receive(0);
			USART_Flush(0);
		}
	}
	_delay_ms(250);
	if(USART_IsSendReady(0)){ // signal to sensor 2 and receive from it
		USART_Send(0x02, 0);
		while (!USART_HasTransmitted(0));
		_delay_ms(40);
		if(USART_HasReceived(0)){ // if checklist has received any data, then update flag
			socket_flag = USART_Receive(0);
			USART_Flush(0);
		}
	}
	_delay_ms(250);
	if(USART_IsSendReady(0)){ // signal to sensor 2 and receive from it
		USART_Send(0x04, 0);
		while (!USART_HasTransmitted(0));
		_delay_ms(40);
		if(USART_HasReceived(0)){ // if checklist has received any data, then update flag
			lock_flag = USART_Receive(0);
			USART_Flush(0);
		}
	}
	return;
}


int main(void){
  DDRA = 0x00; PORTA = 0xFF; // ~PINA is input
  DDRB = 0xFF; PORTB = 0x00; // PORTB is output
  DDRC = 0xFF; PORTC = 0x00; // PORTC is output
  DDRD = 0xFF; PORTD = 0x00; // PORTD is output
	
	// initialization
  TimerOn();
  TimerSet(50); // timer set to 50ms
  LCD_init();
	initUSART(0);
	state = init_state;
	// initialization

	while(1){
		// main tick function that drives the checklist component
		tick();
		
		// integration
		if(USART_HasReceived(0)){ // if checklist has received 0x40, Kris has sent a sleep signal
			if(USART_Receive(0) == 0x40){
				LCD_ClearScreen();
				LCD_DisplayString(1, "...Sending signal...");
				_delay_ms(250);
				if(USART_IsSendReady(0)){		// 0x10 signal means for light to turn off
					USART_Send(0x10, 0);
					while (!USART_HasTransmitted(0));
				}
				_delay_ms(250);
				if(USART_IsSendReady(0)){		// 0x20 signal means for lock to lock
					USART_Send(0x20, 0);
					while (!USART_HasTransmitted(0));
				}
				LCD_DisplayString(1, "Successfully turned off/closed");
			}
			USART_Flush(0);
		}// integration
		
	
		while(!TimerFlag);
		TimerFlag = 0;
  }
}
