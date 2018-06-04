#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"


int main(void){
	DDRA = 0x00; PORTA = 0xFF; // ~PINA is input
	DDRB = 0xFF; PORTB = 0x00; // PORTB is output
	DDRC = 0xFF; PORTC = 0x00; // PORTC is output
	DDRD = 0xFF; PORTD = 0x00; // PORTD is output

	initUSART(0);
	unsigned char socket_sense;
	char socket_flag = 0x00;
	
	while(1){
		//update socket sense with the value from atmega
		socket_sense = ~PINA;

		PORTC = socket_sense; // PORTC reflect socket sense for UI led
		
		// socket flag updates upon the status of the sensor
		if(socket_sense == 0x01){
			socket_flag = 0x02;
		}else{
			socket_flag = 0x00;
		}
		
		// update checklist, if checklist calls for it
		if(USART_HasReceived(0)){
			if(USART_IsSendReady(0)&& (USART_Receive(0) == 0x02)){
				_delay_ms(50);
				USART_Send(socket_flag, 0);
				while (!USART_HasTransmitted(0));
			}
			USART_Flush(0);
		}

	}
}
