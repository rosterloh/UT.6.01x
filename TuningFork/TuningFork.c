// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// March 8, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"


// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

static int flag, last, current;

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000001; // activate clock for port A
	GPIO_PORTA_AMSEL_R &= ~0x0C;      // disable analog functionality on PA2 and PA3
  GPIO_PORTA_PCTL_R &= ~0x0000FF00; // configure PA2 and PA3 as GPIO
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 out
	GPIO_PORTA_DIR_R &= ~0x08;    // make PA3 in
  GPIO_PORTA_DR8R_R |= 0x04;    // enable 8 mA drive on PA2
  GPIO_PORTA_AFSEL_R &= ~0x0C;  // disable alt funct on PA2 and PB3
	GPIO_PORTA_PDR_R |= 0x08;     // pull-up on PA3
  GPIO_PORTA_DEN_R |= 0x0C;     // enable digital I/O on PA2 and PB3
  GPIO_PORTA_DATA_R &= ~0x04;   // make PA2 low
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 90909-1;   // reload value for 880Hz => (1/880Hz) / (1/80Mhz)
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
}

// called at 880 Hz
void SysTick_Handler(void){
	if(flag == 1) {
		GPIO_PORTA_DATA_R ^= 0x04;  // toggle PA2
	} else {
		GPIO_PORTA_DATA_R &= ~0x04; // PA2 off
	}
	NVIC_ST_RELOAD_R = 90909-1;   // reload value for high phase
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
	flag = 0;
	last = 0;
	current = 0;
  EnableInterrupts();   // enable after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
		int i=999;
		while (i--);
		current = GPIO_PORTA_DATA_R&0x08;
		if(current == 0x08 && last == 0x00) {
			flag ^= 1;  // toggle flag  ie if flag is 0, set to 1, if flag is one, set to zero.
		}
		last = current;		
  }
}
