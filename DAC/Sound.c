// Sound.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// March 13, 2014
// This routine calls the 4-bit DAC

#include "Sound.h"
#include "DAC.h"
#include "..//tm4c123gh6pm.h"

unsigned char Index;
//const unsigned char SineWave16[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};
const unsigned long SineWave16[16] = {8,11,13,14,15,14,13,11,8,5,3,2,1,2,3,5};
const unsigned char SineWave32[32] = {8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7};  // dac.xls
int sound_enable;

// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
void Sound_Init(void){
	DAC_Init();          					// Port B is DAC
	sound_enable=0;
	Index = 0;
	NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = C32;    // reload value
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
}

// **************Sound_Tone*********************
// Change Systick periodic interrupts to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period){
// this routine sets the RELOAD and starts SysTick
	NVIC_ST_RELOAD_R = period;    // reload value
	sound_enable = 1;
}


// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
 // this routine stops the sound output
	// NVIC_ST_CTRL_R &= ~0x00000002;		// disable the interrupt
	sound_enable=0;
	DAC_Out(0x00);
}


// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	#if defined SIN_32
	Index = (Index+1)&0x1F;  
	if(sound_enable) {
		DAC_Out(SineWave32[Index]);
	}
	#else
	Index = (Index+1)&0x0F;  
	if(sound_enable) {
		DAC_Out(SineWave16[Index]);
	}
	#endif
}
