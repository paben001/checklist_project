#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "io.h"
#include "timer.h"
#include "usart_ATmega1284.h"
#include "tests.h"


void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0; again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}



void LCD_checklist(unsigned char sensorFlag){

  LCD_ClearScreen();
  // takes in char and sets display based on what the flag is showing

  // Messages to output
  const char *bathroom_msg = "Light:            On";
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
/*
/////////////////////////////////////////////////////////////////////////////
// useful code to copy/paste, should turn these into functions when possible

if(USART_HasReceived(0)){ // if checklist has received any data, then update flag
	state = update_state;
	status_flag = USART_Receive(0)&0x07;
	USART_Flush(0);
}

LCD_checklist(status_flag); // calling LCD display function

if(USART_IsSendReady(0)){ // transmit data through TX 
	USART_Send(tempo, 0);
	while (!USART_HasTransmitted(0));
}

/////////////////////////////////////////////////////////////////////////////
*/

enum States {init_state, idle_state, press_state, update_state} state;
//int counter;
unsigned char status_flag;
void tick(){
	
	char status_flag;
	char light_flag;
	char socket_flag;
	char lock_flag;
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
		default: break;
	}// transitions
	
	switch(state){// state actions
		case init_state:
			break;
		case idle_state:
			break;
		case press_state:
			//counter = 0;
			break;
		case update_state:
			//counter++;
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
// 			
// 			if(counter > iterations){
// 				state = idle_state;
// 				status_flag = light_flag | socket_flag;
// 				LCD_checklist(status_flag);
// 			}
			_delay_ms(250);
			status_flag = light_flag | socket_flag | lock_flag;
			LCD_checklist(status_flag);
			state = idle_state;
			break;
		default: break;
	} // state actions
}



int main(void){
  DDRA = 0x00; PORTA = 0xFF; // ~PINA is input
  DDRB = 0xFF; PORTB = 0x00; // PORTB is output
  DDRC = 0xFF; PORTC = 0x00; // PORTC is output
  DDRD = 0xFF; PORTD = 0x00; // PORTD is output

	// adc_init(); // ADC is PIN A0 Analog
	

  TimerOn();
  TimerSet(50);
  LCD_init();
	initUSART(0);
	
	state = init_state;
	//t1_state = t1_init_state;
	//unsigned char buttonPress;
	
  
	
	while(1){
		
		tick();
		
		//IRTest();
		//LCD_Test_tick();
		/*
		unsigned char buttonPress;
		if(USART_HasReceived(0)){
			buttonPress = USART_Receive(0);
			USART_Flush(0);
		}*/
		
	
		//buttonPress = ~PINA;
		
		//LCD_checklist(buttonPress);
	
		//PORTC = buttonPress;
	
		while(!TimerFlag);
		TimerFlag = 0;
  }
}
