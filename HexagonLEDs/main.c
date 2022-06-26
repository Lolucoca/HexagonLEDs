/*
 * HexagonLEDs.c
 *
 * Created: 26.05.2022 14:41:03
 * Author : lucas
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "General.h"
#include "TinyTouchLib.h"

void init(){
	
	CCP = 0xD8; //Unlock protected registers
	CLKPSR = 0x00;	//Set clock prescaler to 1
	CLKMSR = 0x00;  
	//Pin Configuration: PB0: Comm Port, PB1: Touch Input, PB2: FET Gate 
	DDRB = GATE_MASK; //Only set FET Gate to Output
}

void fadeLEDs(u8 dir){ //DIR: 0: Down, 1: Up
	
	#ifdef __AVR_ATtiny5__
		PORTB = dir & GATE_MASK;
	#else
	for(u8 i = 0; i < 255; i++){
		for(u8 j = 0; j < 255; j++){
			if((dir && (j > i)) || (!dir && (j <= i))){
				PORTB &= ~GATE_MASK; //Turn off
			} else {
				PORTB |= GATE_MASK; //Turn on for the rest
			}
		}
	}
	#endif
}

int main(void){
	u8 ledState = 0; 
	
	/*Initialize the touch library and initialize ports*/
	init();
	tinytouch_init();
	
    while (1){
		
		//The touch button was pushed
		if(tinytouch_sense() == tt_push){ 
			
			//Toggle LED state and set the output	
			ledState = ~ledState;
			fadeLEDs(ledState);
			
			/*This code is there so that with a double press all the other leds are toggled*/
			
			//Initialize a counter variable to keep track of the time.
			//The goal is to have a window after the first push in which a second push triggers the COMM port to toggle all other modules.
			u8 counter = 0; 
			
			//Wait for a potential second push of the button. The inner things of the while loop take around 5ms, so the total window is around a second.
			while((tinytouch_sense() != tt_push) && (counter <= 200)){
				//_delay_ms(1);
				counter++; 
			}
			
			//If the button has been pressed, the while loop breaks with the counter staying under 200.
			//Send a message on the COMM port.
			if(counter < 200){
				DDRB = GATE_MASK | COMM_MASK;
				//Set COMM to low to signal other modules
				PORTB &= ~COMM_MASK;
				_delay_ms(100);
			
				//Set COMM to tri state
				PORTB |= COMM_MASK;
				DDRB = GATE_MASK;
			}
		}
		
		//look for a message on the COMM port.
		if(!((PINB & COMM_MASK) == COMM_MASK)){
			
			//Toggle the LED state.
			ledState = ~ledState;
			fadeLEDs(ledState);
		}
    }
}