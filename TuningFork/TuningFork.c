// TuningFork.c
// Runs on LM4F120/TM4C123
// Uses SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3.
// There is an output on PA2. The output is 
// connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Enes Kur
// July 3, 2022

/* This example accompanies the book and the course
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
		
	 UT.6.20x Embedded Systems - Shape The World: Multi-Threaded Interfacing
   
	 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.ed
*/

#include "..//tm4c123gh6pm.h"

// Global variables for wave status
unsigned long WaveStatus, CStatusPrev;

// basic functions defined at end of startup.s
void DisableInterrupts(void); 	// Disable interrupts
void EnableInterrupts(void);  	// Enable interrupts

// pre-defined functions
void WaitForInterrupt(void);  	// low power mode
void PLL_Init(void);						// 80 MHz clock

// input from PA3, output to PA2, SysTick interrupts
void Sound_Init(void){ 
	unsigned long delay;					// dummy 
	SYSCTL_RCGC2_R |= 0x01;				// Enable PortA Clock
	WaveStatus = 0;								// 1: Output Wave, 0: Do not output
	CStatusPrev = 0;							// Holds the previous switch condition
	delay = SYSCTL_RCGC2_R;				// For Clock to be stable
	GPIO_PORTA_AFSEL_R &= ~0x0C;	// Disable alt funct
	GPIO_PORTA_AMSEL_R &= ~0x0C;	// Disable analog mode
																// Disable port control
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFF00FF) + 0x00000000;
	GPIO_PORTA_DIR_R |= 0x04;			// Make PA2 output
	GPIO_PORTA_DIR_R &= ~0x08;		// Make PA3 input
	GPIO_PORTA_DR8R_R |= 0x04;		// Allow 8mA current on PA2
	GPIO_PORTA_DEN_R |= 0x0C;			// Enable digital mode for PA2, PA3
	
	NVIC_ST_CTRL_R = 0;						// Disable SysTick before init
	NVIC_ST_RELOAD_R = 90908; 		// 90908 ~ 880Hz
																// priority: 0
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R & 0x00FFFFFF;
	NVIC_ST_CTRL_R = 0x00000007;	// Enable SysTick with interrupt
}

// called at 880 Hz
// if Input is PosEdge, changes output condition
void SysTick_Handler(void){
	
	// Check if Input is PosEdge(Current Input is 1, prev is 0)
	if((GPIO_PORTA_DATA_R & 0x08) == 0x08){
		// if prev input is 0
		if(CStatusPrev == 0x00){
			// if Wave is on, makes off and vice versa
			if(WaveStatus == 1)
				WaveStatus = 0;
			else
				WaveStatus = 1;
		}
	}
	
	// Save Input 
	CStatusPrev = GPIO_PORTA_DATA_R & 0x08;
	
	// is Wave is on, toggles Output for 440Hz output, else closes
	if (WaveStatus == 1){
		GPIO_PORTA_DATA_R ^= 0x04;
	}
	else
		GPIO_PORTA_DATA_R &= ~0x04;
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
                  + (4<<22);      // configure for 80 MHz clock ( writes on SYSDIV2 field )
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}

int main(void){
	PLL_Init();									// 80 Mhz clock
  Sound_Init();  							// initialize PA2, PA3
	EnableInterrupts();					// enabling interrupts after initialization
  while(1){}
}

