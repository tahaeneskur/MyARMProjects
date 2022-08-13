// Main.c
// Runs on LM4F120 or TM4C123
// Uses SysTick interrupts to implement a 4-key digital piano
// Enes Kur
// July 3, 2022
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

	 UT.6.20x Embedded Systems - Shape The World: Multi-Threaded Interfacing 
 
	 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
*/

/*
SysTick reload values
Piano key 3: G generates a sinusoidal DACOUT at 783.991 Hz : 3188
Piano key 2: E generates a sinusoidal DACOUT at 659.255 Hz : 3792
Piano key 1: D generates a sinusoidal DACOUT at 587.330 Hz : 4256
Piano key 0: C generates a sinusoidal DACOUT at 523.251 Hz : 4778
*/

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void delay(unsigned long msec);
void PLL_Init(void);

int main(void){ 
	unsigned char input;
// PortE used for piano keys, PortB used for DAC
	PLL_Init();		// 80 MHz clock
  Sound_Init(); // initialize SysTick timer and DAC
  Piano_Init();	// Port E init
  EnableInterrupts();  // enable after all initialization are done
  while(1){                
// input from keys to select tone
// tone goes to SysTick 
		input = Piano_In();
		switch (input){
			case 1:			// key 0 pressed, note C playing
				Sound_Tone(4778);
				break;
			case 2:			// key 1 pressed, note D playing
				Sound_Tone(4256);
				break;
			case 4:			// key 2 pressed, note E playing
				Sound_Tone(3792);
				break;
			case 8:			// key 3 pressed, note G playing
				Sound_Tone(3188);
				break;
			case 0:			// no key pressed
				Sound_Off();
				break;
			default:		// default
				Sound_Off();
				break;
		}
		delay(5);
	}
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
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
