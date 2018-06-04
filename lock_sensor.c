#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"


void lockDoor(){
	unsigned char rotate[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
	int next = 0;
	int numPhases;
	numPhases = ((90/5.625) * 64);

	
	//rotate counter clockwise to return to original position
	numPhases = ((90/5.625) * 64);
	while(numPhases > 0){
		if(next < 0){
			next = 7;
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

void unlockDoor(){
	unsigned char rotate[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
	int next = 0;
	int numPhases;
	numPhases = ((90/5.625) * 64);
	 	//rotate clockwise to turn off switch
	 while(numPhases > 0){
		 if(next > 7){
			 next = 0;
		 }
		 else{
			 next++;
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

	unsigned char lock_sense;
	char lock_flag = 0x00;
	
	while(1){
		
		// match lock_sense to the status of IR receiver plugged into atmega
		lock_sense = ~PINA;

		// update lock flag based on the value of atmega values input
		if((lock_sense&0x01) == 0x01){
			lock_flag = 0x04;
			PORTC = 0x01;
		}else{
			lock_flag = 0x00;
			PORTC = 0x00;
		}
		
		// based on what checklist has sent, either update or lock the door
		if(USART_HasReceived(0)){
			if(USART_IsSendReady(0)&& (USART_Receive(0) == 0x04)){ // 0x04: checklist asking for status
				_delay_ms(50);
				USART_Send(lock_flag, 0);
				while (!USART_HasTransmitted(0));
			}
			else if(USART_Receive(0) == 0x20){ // 0x20: checklist asking to lock door
				if(lock_flag == 0x04){
					lockDoor();
				}
			}
			USART_Flush(0);
		}
		
		//button to manually lock or unlock the door
		if((lock_sense&0x02)==0x02){
			if(lock_flag == 0x04){
				lockDoor();
			}
			else{
				unlockDoor();
			}
		}
	}
}