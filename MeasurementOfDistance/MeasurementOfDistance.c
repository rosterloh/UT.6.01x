// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// April 8, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE1 and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts

unsigned char String[10];
unsigned long Distance; // units 0.001 cm
unsigned long ADCdata;  // 12-bit 0 to 4095 sample
unsigned long Flag;     // 1 means valid Distance, 0 means Distance is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
	unsigned long result;
	result = (2000*sample)/4095;
	return result;
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = period;		// reload value (80Mhz/40Hz)-1
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
}
// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
	//NVIC_ST_RELOAD_R = 1999999;		// reload value
	GPIO_PORTF_DATA_R |= 0x04;          // profile
	ADCdata = ADC0_In();
	GPIO_PORTF_DATA_R &= ~0x04;
	Distance = Convert(ADCdata);
	Flag = 1;
}

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){
// as part of Lab 11 you implemented this function
	if(n<10000) {
    String[0] = n/1000 +0x30;
    String[1] = '.';
    String[2] = (n/100)%10 +0x30;
    String[3] = (n/10)%10 +0x30;
    String[4] = n%10 +0x30;
    
  } else {
    String[0] = '*';
    String[1] = '.';
    String[2] = '*';
    String[3] = '*';
    String[4] = '*';
    String[5] = ' ';
    String[6] = 'c';
    String[7] = 'm';
  }
  String[5] = ' ';
  String[6] = 'c';
  String[7] = 'm';
  String[8] = 0;
}

int main(void){ 
  volatile unsigned long delay;
	//TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_NoScope);
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
	ADC0_Init();    											// initialize ADC0, channel 1, sequencer 3
  Nokia5110_Init();											// initialize Nokia5110 LCD
	SysTick_Init(1999999);								// initialize SysTick for 40 Hz interrupts
	// initialize profiling on PF1 (optional)
	//    wait for clock to stabilize
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // activate port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_DIR_R |= 0x04;             // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;             // enable digital I/O on PF2
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  EnableInterrupts();
	Nokia5110_Clear();
	Nokia5110_OutString((unsigned char *)"UT.6.01x Lab 14 Start");
  while(1){ 
		if(Flag) {
			UART_ConvertDistance(Distance);
			Nokia5110_SetCursor(0, 3);			
			Nokia5110_OutString(String);
			Flag = 0;
		}			
  }
}

