#include <avr/io.h>
#include <stdio.h>
#include <string.h>
//#include <util/delay.h>
#include "io.h"
#include "timer.h"
#include "usart_ATmega1284.h"

#include "tests.h"


void photoresistorTest(){
	
	char int_buffer[10];
	uint16_t x = adc_read(0);
	itoa(x, int_buffer, 10);
	LCD_ClearScreen();
	LCD_DisplayString(1, int_buffer);
	
	return;
}

void IRTest(){
	// functionally the same as photoresistor test, except must connected
	// emitter to an output
	char int_buffer[10];
	uint16_t x = adc_read(0);
	PORTC = x;
	itoa(x, int_buffer, 10);
	LCD_ClearScreen();
	LCD_DisplayString(1, int_buffer);
	
	return;

}

enum LCD_Test_States{t1_init_state, t1_unpress_state, t1_inc_state, t1_dec_state} t1_state;
// Figuring out how to configure LCD display to different resolution
void LCD_Test_tick(){
	unsigned char col;
	unsigned char t1_buttonPress = ~PINA;
	
	switch(t1_state){
		case t1_init_state:
		col = 1;
		t1_state = t1_unpress_state;
		LCD_Cursor(col);
		break;
		case t1_unpress_state:
		t1_buttonPress = ~PINA;
		if(t1_buttonPress == 0x01){
			t1_state = t1_inc_state;
		}
		else if(t1_buttonPress == 0x02){
			t1_state = t1_dec_state;
		}
		break;
		case t1_inc_state:
		t1_buttonPress = ~PINA;
		if(t1_buttonPress == 0x00){
			t1_state = t1_unpress_state;
			col = col + 1;
			LCD_Cursor(col);
		}
		break;
		case t1_dec_state:
		t1_buttonPress = ~PINA;
		if(t1_buttonPress == 0x00){
			t1_state = t1_unpress_state;
			col = col - 1;
			LCD_Cursor(col);
		}
		break;
		default: break;
	}
}

