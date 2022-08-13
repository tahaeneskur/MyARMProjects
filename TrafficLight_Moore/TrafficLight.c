// ***** 0. Documentation Section *****
// TrafficLight.c
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Enes Kur
// June 26, 2022

/* This example accompanies the book and the course
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
	 
	 UTAustinX - UT.6.10x Embedded Systems - Shape The World: 
	 Microcontroller Input/Output

   Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
*/

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
#include "tm4c123gh6pm.h"
#define EO 		0								// East open
#define EW 		1								// East yellow
#define NO 		2								// North open
#define NW 		3								// North yellow
#define WO 		4								// Peds open
#define WH1 	5								// Peds first red
#define WC1 	6								// Peds no light
#define WH2 	7								// Peds second red
#define WC2 	8								// Peds no light
#define WH3 	9								// Peds third and last red
#define shortWait 75					// 750 msec
#define longWait 300					// 3000 msec
// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Ports_Init(void);				// Init ports B, E and F
void SysTick_Init(void);			// Init SysTick
															// 10 ms wait func
void SysTick_Wait10ms(unsigned long delay);
															// 1 ms wait func
void SysTick_Wait(unsigned long delay);
void PLL_Init(void);					// 80 MHz clock

															// Traffic Lights Output(LEDs)
void LightOut(unsigned long output);
unsigned long SensorIn(void);	// Sensor Inputs(buttons)

// STRUCTURE INIT
struct State{
	unsigned long Out;					// Most significant 6 bits for car lights, last 2 bits for peds light
	unsigned long Time;					// State wait time
	unsigned long Next[8];			// Next possible states
};

typedef const struct State SType;

SType Fsm[10] = {
															// East green
	{0x31, longWait, {EO, EO, EW, EW, EW, EW, EW, EW}},
															// East yellow
	{0x51, shortWait, {NO, NO, NO, NO, WO, WO, WO, NO}},
															// North green
	{0x85, longWait, {NO, NW, NO, NW, NW, NW, NW, NW}},
															// North yellow
	{0x89, shortWait, {EO, EO, EO, EO, WO, WO, WO, WO}},
															// Peds green
	{0x92, longWait, {WO, WH1, WH1, WH1, WO, WH1, WH1, WH1}},
															// Peds flashing red for three times
	{0x91, shortWait, {WC1, WC1, WC1, WC1, WC1, WC1, WC1, WC1}},
	{0x90, shortWait, {WH2, WH2, WH2, WH2, WH2, WH2, WH2, WH2}},
	{0x91, shortWait, {WC2, WC2, WC2, WC2, WC2, WC2, WC2, WC2}},
	{0x90, shortWait, {WH3, WH3, WH3, WH3, WH3, WH3, WH3, WH3}},
	{0x91, shortWait, {EO, EO, NO, EO, EO, EO, NO, EO}},
};

unsigned long CState;					// Current state
unsigned long Input;					// from sensors(buttons)
unsigned long Output;					// to traffic lights(LEDs)

// ***** 3. Subroutines Section *****

int main(void){ 
  PLL_Init();									// Activates 80 MHz clock
  Ports_Init();								// Activates ports B, E and F
	SysTick_Init();							// Activates SysTick
	CState = NO;								// North open by default
  EnableInterrupts();  				// enable after all initialization are done
	while(1){
		LightOut(CState);					// Outputs current state
															// Waits on that state
		SysTick_Wait10ms(Fsm[CState].Time);
		Input = SensorIn();				// Gets input
															// Switches to next state
		CState = Fsm[CState].Next[Input];
  }
}

void Ports_Init(void) {
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x32; 		// Activating clock for Ports B, E and F
	delay = SYSCTL_RCGC2_R;			// for clock to be stable
	
															// No alternating func
	GPIO_PORTF_PCTL_R = 0x00000000;
	GPIO_PORTB_PCTL_R = 0x00000000;
	GPIO_PORTE_PCTL_R = 0x00000000;
								
	GPIO_PORTF_DIR_R |= 0x0A; 	// PF3 and PF1 are output
	GPIO_PORTE_DIR_R &= ~0x07;	// PE0, PE1 and PE2 are input
	GPIO_PORTB_DIR_R |= 0x3F; 	// PB0-PB5 are output
															
															// Enable ports		
	GPIO_PORTF_DEN_R |= 0x0A;
	GPIO_PORTE_DEN_R |= 0x07;
	GPIO_PORTB_DEN_R |= 0x3F;
}

void LightOut(unsigned long out){
	unsigned long W;
	W = 0;
															// Most significant 6 bits goes to Port B
	GPIO_PORTB_DATA_R = (Fsm[out].Out) >> 2;
	
															// Last 2 bits goes to Port F
	W = ((Fsm[out].Out & 0x01) << 1) + ((Fsm[out].Out & 0x02) << 2);
	GPIO_PORTF_DATA_R = W;
}

unsigned long SensorIn(void){
															// Input from Port E
	return (GPIO_PORTE_DATA_R & 0x07);
}

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;// enable SysTick with core clock
}

void SysTick_Wait10ms(unsigned long delay){
	unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  		// wait 10ms
  }
}

void SysTick_Wait(unsigned long delay){
	NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}

void PLL_Init(void){
  // 0) Use RCC2
  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock ( writes on SYSDIV2 feild )
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}
