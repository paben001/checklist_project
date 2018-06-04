#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "timer.h"
#include "usart_ATmega1284.h"


void turnOffLight(){
	unsigned char rotate[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
	int next = 0;
	int numPhases;
	numPhases = ((90/5.625) * 64)/2;
	//rotate clockwise to turn off switch
	while(numPhases > 0){
		if(next >= 7){
			next = -1;
		}
		else{
			next++;
		}
		numPhases--;
		_delay_ms(10);
		PORTB = rotate[next];
	}
	
	//rotate counter clockwise to return to original position
	numPhases = ((90/5.625) * 64)/2;
	while(numPhases > 0){
		if(next <= 0){
			next = 8;
		}
		else{
			next--;
		}
		numPhases--;
		_delay_ms(10);
		PORTB = rotate[next];
	}
		
	return;
}



int main(void){
	DDRA = 0x00; PORTA = 0xFF; // ~PINA is input
	DDRB = 0x0F; PORTB = 0xF0; // PORTB is output
	DDRC = 0xFF; PORTC = 0x00; // PORTC is output
	DDRD = 0xFF; PORTD = 0x00; // PORTD is output
	initUSART(0);

	unsigned char light_sense;
	char light_flag = 0x00;
	
	while(1){

		light_sense = ~PINA; // assign sensor value

		PORTC = light_sense; // display status of PINA for userinterface/debugging ease
		// updating light flag based on  sensor values
		if(light_sense == 0x01){
			light_flag = 0x01;
		}else{
			light_flag = 0x00;
		}
		
		// if checklist asks to be updated, send the appropriate signal dependent on the condition of light sense
		if(USART_HasReceived(0)){
			if(USART_IsSendReady(0)&& (USART_Receive(0) == 0x01)){
				_delay_ms(50);
				USART_Send(light_flag, 0);
				while (!USART_HasTransmitted(0));
			}
			if(USART_Receive(0) == 0x10){
				if(light_flag == 0x01){
					turnOffLight();
				}
			}
			USART_Flush(0);
		}

	}
}
