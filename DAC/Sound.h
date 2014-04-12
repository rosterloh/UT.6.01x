// Sound.h
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// March 13, 2014

enum Notes {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
};

#define C 9556 		// 80Mhz/(523.251*16) = 9555.6435
#define D 8513		// 80Mhz/(587.330*16) = 8513.1017
#define E 7584		// 80Mhz/(659.255*16) = 7584.3187
#define G 6378		// 80Mhz/(783.991*16) = 6577.6242

// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also initializes DAC
// Input: none
// Output: none
void Sound_Init(void);

// **************Sound_Tone*********************
// Change SysTick periodic interrupts to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period);


// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void);

