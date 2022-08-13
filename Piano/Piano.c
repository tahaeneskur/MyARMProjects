// Piano.c
// Runs on LM4F120 or TM4C123, 
// There are four keys in the piano
// Enes Kur
// July 3, 2022

// Port E bits 3-0 have 4 piano keys

#include "Piano.h"
#include "..//tm4c123gh6pm.h"

// **************Piano_Init*********************
// Initialize piano key inputs
// Input: none
// Output: none
void Piano_Init(void){ unsigned long delay;
  SYSCTL_RCGC2_R |= 0x10;						// Enable PortE clock
	delay = SYSCTL_RCGC2_R;						// For Clock to be stable
	GPIO_PORTE_AFSEL_R &= ~0x0F;			// Disable alt funct
	GPIO_PORTE_AMSEL_R &= ~0x0F;			// Disable analog mode
																		// Disable port control
	GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R & 0xFFFF0000;
	GPIO_PORTE_DIR_R &= ~0x0F;				// PE0-3 input
	GPIO_PORTE_DR8R_R |= 0x0F;				// 8mA drive
	GPIO_PORTE_DEN_R |= 0x0F;					// Enable PE0-3
}

// **************Piano_In*********************
// Input from piano key inputs
// Input: none 
// Output: 0 to 15 depending on keys
// 0x01 is key 0 pressed, 0x02 is key 1 pressed,
// 0x04 is key 2 pressed, 0x08 is key 3 pressed
unsigned long Piano_In(void){
  return GPIO_PORTE_DATA_R & 0x0F;
}
