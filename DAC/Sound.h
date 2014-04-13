// Sound.h
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// March 13, 2014

#define SIN32 1

#define C16 9555 		// 80Mhz/(523.251*16)-1 = 9554.6435
#define D16 8512		// 80Mhz/(587.330*16)-1 = 8512.1017
#define E16 7583		// 80Mhz/(659.255*16)-1 = 7583.3187
#define G16 6377		// 80Mhz/(783.991*16)-1 = 6576.6242

#define C32 4777 		// 80Mhz/(523.251*32)-1 = 4776.8217
#define D32 4256		// 80Mhz/(587.330*32)-1 = 4255.5508
#define E32 3791		// 80Mhz/(659.255*32)-1 = 3791.1593
#define G32 3188		// 80Mhz/(783.991*32)-1 = 3187.8121

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

