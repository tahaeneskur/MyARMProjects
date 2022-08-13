// MeasurementOfAngle.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal angle, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.
// July 3, 2022

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

		Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
*/

// Pot pin 3 connected to +3.3V
// Pot pin 2 connected to PE2(Ain1)
// Pot pin 1 connected to ground

#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"

void EnableInterrupts(void);  // Enable interrupts
unsigned long Pow(unsigned long k, unsigned long l);

unsigned char String[10]; // null-terminated ASCII string
unsigned long Angle;   		// units 0.1 deg
unsigned long ADCdata;    // 12-bit 0 to 4095 sample
unsigned long Flag;       // 1 means valid Angle, 0 means Angle is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point angle (resolution 0.1 deg).  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit angle (resolution 0.1 deg)
unsigned long Convert(unsigned long sample){
  return (0.73*sample);	// 0.73*4095 = 300 degrees which is pot turn amount
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;						// Disable SysTick before init
	NVIC_ST_RELOAD_R = period; 	// 1999999 ~ 40Hz
																// priority: 0
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R & 0x00FFFFFF;
	NVIC_ST_CTRL_R = 0x00000007;	// Enable SysTick with interrupt
}
// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){
										// Sample data from ADC
	ADCdata = ADC0_In();
										// Convert 12-bit ADC data to degree format
	Angle = Convert(ADCdata);
	Flag = 1;					// mailbox is full
}

//-----------------------UART_ConvertAngle-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.1 deg)
// Output: store the conversion in global variable String[10]
// Fixed format 3 digits, point, 1 digit, space, units, null termination
// Examples
//    4 to "000.4 deg"  
//   31 to "003.1 deg" 
//  102 to "010.2 deg" 
// 2210 to "221.0 deg"
//10000 to "***.* deg"  any value larger than 9999 converted to "***.* deg"
void UART_ConvertAngle(unsigned long n){
	unsigned long cnt, cnt1, dec, check, ree;
	cnt = 0;					// for digits of number
	cnt1 = 0;					// for chars of string
	dec = 0;					// for putting dot
	check = 0;				// check if number < 10000
	if(n < 10000){
		check = 1;
	}
	// executes 4 times because number has 4 digit
	while(cnt <= 4){
		if(check){
			// number < 10000
			// adding digits to string
			ree = n/Pow(10, 3-cnt1);
			String[cnt] = (ree + 0x30);
			ree = Pow(10, 3-cnt1);
			n = n%ree;
		}
		else
			// number >= 10000
			// adding stars to string
			String[cnt] = '*';
		
		if(dec == 2){
			// adding point to string
			cnt++;
			String[cnt] = '.';
		}
		cnt++;
		cnt1++;
		dec++;
	}
	
	String[cnt] = ' ';		// adding space
	// adding units
	String[cnt+1] = 'd';
	String[cnt+2] = 'e';
	String[cnt+3] = 'g';
}

int main(void){ 
  volatile unsigned long delay;
	ADC0_Init();					// initialize ADC0, channel 1, sequencer 3
	Nokia5110_Init();			// initialize Nokia5110 LCD (optional)
	SysTick_Init(1999999);// initialize SysTick for 40 Hz interrupts
	EnableInterrupts();		// enable interrupts
  while(1){ 
// read mailbox
// output to Nokia5110 LCD 
		if(Flag){
												// convert degree value to string
			UART_ConvertAngle(Angle); 
												// start to print up-left corner of Nokia 5110
			Nokia5110_SetCursor(0, 0);
												// print the degree value to Nokia 5110
			Nokia5110_OutString(String);
			Flag = 0;					// mailbox is used
		} 
  }
}

//-------------------- Pow --------------------
// takes the exponent with recursive method
// Inputs: 32-bit base, 32-bit exponent
// Output: 32-bit calculated number
unsigned long Pow(unsigned long k, unsigned long l){
	if(k == 0) return 0;
	if(l == 0) return 1;
	return k*Pow(k, l-1);
}
