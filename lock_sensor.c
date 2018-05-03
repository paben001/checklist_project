#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
//#include "timer.h"
#include "usart_ATmega1284.h"


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
	// ANDing with ?7? will always keep the value
	// of ?ch? between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ?1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0; again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
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

enum States {init_state, idle_state, send_state} state;
	
void tick(){
	char signal_flag;
	unsigned char status_flag;
	//char test_flag; // can be set to buttons in order to test if functions are working approppriately
	//test_flag = ~PINA;
	
	switch(state){ // transitions
		case init_state:
			state = idle_state;
			break;
		case idle_state:
			if(USART_HasReceived(0)){ // if sensors receive signal, prepare to send status
				USART_Flush(0);
				state = send_state;
			}
			break;
		case send_state:
			if(USART_IsSendReady(0)){ // transmit data through TX
				status_flag = ~PINA;
				USART_Send(status_flag, 0);
				while (!USART_HasTransmitted(0));
			}
			state = idle_state;
			break;
		default: break;
	}// transitions
	
	switch(state){// state actions
		case init_state:
			break;
		case idle_state:
			break;
		case send_state:
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
	

	//TimerOn();
	//TimerSet(50);
	initUSART(0);
	
	state = init_state;
	unsigned char tempo;
	char lock_flag = 0x00;
	
	while(1){
		
		//tick();
		
		//IRTest();
		// photoresistorTest();
		
		tempo = ~PINA;
		/*
		if(USART_IsSendReady(0)){
			USART_Send(tempo, 0);
			while (!USART_HasTransmitted(0));
		}//*/

		
		if(tempo == 0x01){
			lock_flag = 0x04;
			PORTC = 0x01;
		}else{
			lock_flag = 0x00;
			PORTC = 0x00;
		}
		
		if(USART_HasReceived(0)){
			if(USART_IsSendReady(0)&& (USART_Receive(0) == 0x04)){
				_delay_ms(50);
				USART_Send(lock_flag, 0);
				while (!USART_HasTransmitted(0));
			}
			USART_Flush(0);
		}
		
		//while(!TimerFlag);
		//TimerFlag = 0;
	}
}
