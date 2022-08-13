// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 SS3 to be triggered by
// software and trigger a conversion, wait for it to finish,
// and return the result. 
// Daniel Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "ADC.h"
#include "..//tm4c123gh6pm.h"

// This initialization function sets up the ADC 
// Max sample rate: <=125,000 samples/second
// SS3 triggering event: software trigger
// SS3 1st sample source:  channel 1
// SS3 interrupts: enabled but not promoted to controller
void ADC0_Init(void){ unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_DIR_R &= ~0x04;
	GPIO_PORTE_DEN_R &= ~0x04;
	GPIO_PORTE_AFSEL_R |= 0x04;
	GPIO_PORTE_AMSEL_R |= 0x04;
	
	SYSCTL_RCGC0_R |= 0x00010000;
	delay = SYSCTL_RCGC2_R;
	SYSCTL_RCGC0_R = (SYSCTL_RCGC0_R & 0xFFFFFCFF);
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~0x08;
	ADC0_EMUX_R = (ADC0_EMUX_R & 0xFFFF0FFF);
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFF0) + 0x01;
	ADC0_SSCTL3_R |= 0x06;
	ADC0_ACTSS_R |= 0x08;
}


//------------ADC0_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC0_In(void){unsigned long data;
	ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R & 0x08) == 0){}
	data = ADC0_SSFIFO3_R & 0xFFF;
	ADC0_ISC_R |= 0x08;
  return data; // replace this line with proper code
}
