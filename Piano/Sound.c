// Sound.c
// Runs on LM4F120 or TM4C123, 
// Uses the SysTick timer to request interrupts at a particular period.
// Enes Kur
// July 3, 2022
// This routine calls the 4-bit DAC

#include "Sound.h"
#include "DAC.h"
#include "..//tm4c123gh6pm.h"

const unsigned char SineWave[32] = {8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7};
unsigned char Index;


// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
void Sound_Init(void){
	Index = 0;
  DAC_Init();										// Port B init
	NVIC_ST_CTRL_R = 0;						// Disable SysTick before init
	NVIC_ST_RELOAD_R = 90908; 		// For init, changes with every button press
																// priority: 0
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R & 0x00FFFFFF;
	NVIC_ST_CTRL_R = 0x00000007;	// Enable SysTick with interrupt
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
	NVIC_ST_RELOAD_R = period - 1;
}


// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
 // this routine stops the sound output
	GPIO_PORTB_DATA_R &= ~0x0F;
}


// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	if((GPIO_PORTE_DATA_R & 0x0F) != 0){
		DAC_Out(SineWave[Index]);		// Outputs to DAC
		Index = (Index + 1) & 0x1F; // Next sequence, if 15 => 0
	}
}
