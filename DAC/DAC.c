// DAC.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Implementation of the 4-bit digital to analog converter
// Daniel Valvano, Jonathan Valvano
// March 13, 2014
// Port B bits 3-0 have the 4-bit DAC

#include "DAC.h"
#include "..//tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;          	// activate Port B
  delay = SYSCTL_RCGC2_R;         	// allow time for clock to stabilize
																		// no need to unlock
  GPIO_PORTB_AMSEL_R &= ~0x0F;     	// disable analog functionality on PB3-0
  GPIO_PORTB_PCTL_R &= ~0x0000FFFF;	// configure PB3-0 as GPIO
  GPIO_PORTB_DIR_R |= 0x0F;        	// make PB3-0 out
	GPIO_PORTB_DR8R_R |= 0x0F;				// enable 8 mA drive on PB3-0
  GPIO_PORTB_AFSEL_R &= ~0x0F;     	// disable alt funct on PB3-0
  GPIO_PORTB_DEN_R |= 0x0F;        	// enable digital I/O on PB3-0
}


// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(unsigned long data){
  GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & ~0x0f) | (data & 0x0f);
}
