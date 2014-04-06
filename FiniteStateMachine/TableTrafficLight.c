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
void PortB_Init(void);
void PortE_Init(void);
void PortF_Init(void);

struct State {
  unsigned long OutLight; // PB5-3 to west light, PB2-0 to south light
	unsigned long OutWalk;	// PF3 green walk, PF1 red dont walk
  unsigned long Time;			// in 10ms units    
  unsigned long Next[8];	// input 0x00 means no cars or pedestrians,
													//       0x01 means cars on west and no pedestrians,
													//       0x02 means cars on south and no pedestrians,
													//       0x03 means cars on west and south,
													//       0x04 means pedestrians and no cars,
													//       0x05 means pedestrians and cars on west,	
													//       0x06 means pedestrians and cars on south,
													//       0x07 means pedestrians and cars on west and south,	
}; 
typedef const struct State STyp;

#define goW   	0
#define waitW 	1
#define goS   	2
#define waitS 	3
#define goP			4
#define waitAll	5
#define	flashP1	6
#define flashP2	7
#define flashP3	8

STyp FSM[9]={
 {0x0C, 0x02, 10,{goW,goW,waitW,waitW,waitW,waitW,waitW,waitW}},												// S0) West green, South red, Walk red
 {0x14, 0x02, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,goS}},				// S1) West yellow, South red, Walk red
 {0x21, 0x02, 10,{goS,waitS,goS,waitS,waitS,waitS,waitS,waitS}},												// S2) West red, South green, Walk red
 {0x22, 0x02, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,goP}},				// S3) West red, South yellow, Walk red
 {0x24, 0x08, 10,{goP,flashP1,flashP1,flashP1,goP,flashP1,flashP1,waitAll}},						// S4) West red, South red, Walk green
 {0x24, 0x02, 10,{waitAll,goW,goS,goW,goP,goP,goP,flashP1}},														// S5) West red, South red, Walk red
 {0x24, 0x00, 10,{flashP2,flashP2,flashP2,flashP2,flashP2,flashP2,flashP2,flashP2}},		// S6) West red, South red, Walk off
 {0x24, 0x02, 10,{flashP3,flashP3,flashP3,flashP3,flashP3,flashP3,flashP3,flashP3}},		// S7) West red, South red, Walk red
 {0x24, 0x00, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,goW}}					// S8) West red, South red, Walk off
};		

unsigned long cState;  // index to the current state 
unsigned long Input; 

// ***** 3. Subroutines Section *****

int main(void){ 
	TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
 
  PLL_Init2();       // 80 MHz, Program 10.1
  SysTick_Init();   // Program 10.2
	//Gpio_Init();
	PortF_Init();
	PortB_Init();
	PortE_Init();
	  
  cState = waitAll; // Initial state
	
  EnableInterrupts();
  while(1){
		// output based on current state
		GPIO_PORTB_DATA_R = FSM[cState].OutLight;
		// output PF3 and PF1
		GPIO_PORTF_DATA_R = FSM[cState].OutWalk;
		// wait for time according to state
    SysTick_Wait10ms(FSM[cState].Time);
    // get input
		Input = GPIO_PORTE_DATA_R&0x07; // Input 0,1,2
    // change the state based on input and current state
		cState = FSM[cState].Next[Input];  
  }
}

void Gpio_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x32;          // 1) activate Port B, E and F
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock
  GPIO_PORTB_AMSEL_R &= ~0xFF;     // 3) disable analog functionality on PB7-0
  GPIO_PORTB_PCTL_R = 0x00000000;  // 4) configure PB7-0 as GPIO
  GPIO_PORTB_DIR_R |= 0xFF;        // 5) make PB7-0 out
  GPIO_PORTB_AFSEL_R &= ~0xFF;     // 6) disable alt funct on PB7-0
  GPIO_PORTB_DR8R_R |= 0xFF;       // enable 8 mA drive on PB7-0
  GPIO_PORTB_DEN_R |= 0xFF;        // 7) enable digital I/O on PB7-0
	
	GPIO_PORTE_AMSEL_R &= ~0x07;     // 8) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF;// 9) configure PE2-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x07;       // 10) make PE2-0 in
  GPIO_PORTE_AFSEL_R &= ~0x07;     // 11) disable alt funct on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;        // 12) enable digital I/O on PE2-0
	
	GPIO_PORTF_LOCK_R = 0x4C4F434B;  // 13) unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;         // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R &= 0x00;      // 14) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000; // 15) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R &= ~0x11;       // 16) PF4,PF0 input, PF3,PF2,PF1 output
	GPIO_PORTF_DIR_R |= 0xE;
  GPIO_PORTF_AFSEL_R &= 0x00;      // 17) no alternate function
  GPIO_PORTF_PUR_R |= 0x11;        // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R |= 0x1F;        // 18) enable digital pins PF4-PF0        
}

void PortB_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;          // activate Port B
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
  GPIO_PORTB_AMSEL_R &= ~0x3F;     // disable analog functionality on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;// configure PB5-0 as GPIO
  GPIO_PORTB_DIR_R |= 0x3F;        // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;     // disable alt funct on PB5-0
  GPIO_PORTB_DR8R_R |= 0x3F;       // enable 8 mA drive on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;        // enable digital I/O on PB5-0
}

void PortE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;          // activate Port E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
	GPIO_PORTE_AMSEL_R &= ~0x07;     // disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x00000FFF;// configure PE2-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x07;       // make PE2-0 in
  GPIO_PORTE_AFSEL_R &= ~0x07;     // disable alt funct on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;        // enable digital I/O on PE2-0
}


void PortF_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x20;          // activate Port F
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
	GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;         // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R &= 0x00;      // disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000; // GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R &= ~0x11;       // PF4,PF0 input, PF3,PF2,PF1 output
	GPIO_PORTF_DIR_R |= 0xE;
  GPIO_PORTF_AFSEL_R &= 0x00;      // no alternate function
  GPIO_PORTF_PUR_R |= 0x11;        // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R |= 0x1F;        // enable digital pins PF4-PF0        
}

