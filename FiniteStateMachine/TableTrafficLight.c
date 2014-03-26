// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// November 7, 2013

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "SysTick.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Gpio_Init(void);

#define LIGHT                   (*((volatile unsigned long *)0x400050FC))
#define SENSOR                  (*((volatile unsigned long *)0x4002400C))

struct State {
  unsigned long Out; 			// PB7-4 to right motor, PB3-0 to left
  unsigned long Time;			// in 10ms units    
  unsigned long Next[4];	// input 0x00 means ok,
													//       0x01 means right side bumped something,
													//       0x02 means left side bumped something,
													//       0x03 means head-on collision (both sides bumped something)
}; 
typedef const struct State STyp;

#define goN   0
#define waitN 1
#define goE   2
#define waitE 3

STyp FSM[4]={
 {0x21,3000,{goN,waitN,goN,waitN}},	// S0) initial state and state where bumpers are checked
 {0x22, 500,{goE,goE,goE,goE}},			// S1) both forward [1]
 {0x0C,3000,{goE,goE,waitE,waitE}},	// S2) both forward [2]
 {0x14, 500,{goN,goN,goN,goN}}			// S2) both forward [2]
};		

unsigned long cState;  // index to the current state 
unsigned long Input; 

// ***** 3. Subroutines Section *****

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
 
	volatile unsigned long delay;
  PLL_Init();       // 80 MHz, Program 10.1
  SysTick_Init();   // Program 10.2
	Gpio_Init();
  
  cState = goN; // Initial state
	
  EnableInterrupts();
  while(1){
		// output based on current state
		GPIO_PORTB_DATA_R = FSM[cState].Out;
		// wait for time according to state
    SysTick_Wait10ms(FSM[cState].Time);
    // get input
		Input = GPIO_PORTE_DATA_R&0x03; // Input 0,1,2,3
    // change the state based on input and current state
		cState = FSM[cState].Next[Input];  
  }
}

void Gpio_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x12;          // 1) activate Port B and E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock
  GPIO_PORTB_AMSEL_R &= ~0xFF;     // 3) disable analog functionality on PB7-0
  GPIO_PORTB_PCTL_R = 0x00000000;  // 4) configure PB7-0 as GPIO
  GPIO_PORTB_DIR_R |= 0xFF;        // 5) make PB7-0 out
  GPIO_PORTB_AFSEL_R &= ~0xFF;     // 6) disable alt funct on PB7-0
  GPIO_PORTB_DR8R_R |= 0xFF;       // enable 8 mA drive on PB7-0
  GPIO_PORTB_DEN_R |= 0xFF;        // 7) enable digital I/O on PB7-0
	
	GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
	
	GPIO_PORTE_AMSEL_R &= ~0x03;     // 8) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF;// 9) configure PE1-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x03;       // 10) make PE1-0 in
  GPIO_PORTE_AFSEL_R &= ~0x03;     // 11) disable alt funct on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;        // 12) enable digital I/O on PE1-0
}
