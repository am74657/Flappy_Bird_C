// Lab9.c
// Runs on LM4F120 or TM4C123
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 4/10/2017 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats
// UART1 on PC4-5
// * Start with where you left off in Lab8. 
// * Get Lab8 code working in this project.
// * Understand what parts of your main have to move into the UART1_Handler ISR
// * Rewrite the SysTickHandler
// * Implement the s/w FiFo on the receiver end 
//    (we suggest implementing and testing this first)

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
#include "UART.h"
#include "FiFo.h"
#include <MATH.h>

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on Nokia
// main3 adds your convert function, position data is no Nokia

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void SysTick_Init(void);
void SysTick_Handler(void);
char *UDec2Str(unsigned long n);
	
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
uint32_t Data;      // 12-bit ADC
uint32_t Position;  // 32-bit fixed-point 0.001 cm
char data[8];

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void)
{
	
  volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |=0x20;			//activate port F clock	
	delay = SYSCTL_RCGCGPIO_R;			//delay
	GPIO_PORTF_DIR_R |= 0x0E;			//Make PF1-3 outputs
	GPIO_PORTF_DEN_R |= 0x0E;			//Enable the diigital function for PF1-3
	GPIO_PORTF_AFSEL_R &= ~0x0E;			//Disable Alternate function
	GPIO_PORTF_PUR_R |= 0x0E;
}

// Get fit from excel and code the convert routine with the constants
// from the curve-fit
uint32_t Convert(uint32_t input)
{
	if(input == 0 ) input=input;
	if(input > 0 && input < 50) input=(386*input)/10000 + 7857/10000;		// for distance less than 0.2 cm
		else if(input < 1400)  input = (381*input)/1000 + 20597/100;		//if less than 0.7cm
				else if(input <= 4030 && input ) input = (4198*input)/10000 + 15637/100;	//if less than 1.7cm
					else if(input <= 4090) input = (10274*input)/10000 - 22072/10;		//if greater than 1.7cm
						else input = 2000;							//if greater then 2cm
	return input;
}


// final main program for bidirectional communication
// Sender sends using SysTick Interrupt
// Receiver receives using RX
int main(void){ 
  
  TExaS_Init();       // Bus clock is 80 MHz 
  FiFo_Init();
	ST7735_InitR(INITR_REDTAB);
  ADC_Init();    // initialize to sample ADC
  PortF_Init();
	UART_Init();       // initialize UART
  SysTick_Init();
//Enable SysTick Interrupt by calling SysTick_Init()
  EnableInterrupts();
  char tempChar = 'a';
  while(1)
	{
		while(tempChar != 0x02) {FiFo_Get(&tempChar);}
		ST7735_SetCursor(0,0);
		while(tempChar != 0x03)
		{
			FiFo_Get(&tempChar);
			if(tempChar != 0x03 && tempChar != '\r')
			{
				ST7735_OutChar(tempChar);
			}
		}
	}
	
}

/* SysTick ISR
*/
void SysTick_Handler(void)
{ // every 25 ms
 //Similar to Lab9 except rather than grab sample and put in mailbox
 //        format message and transmit
  int i;
	int data;
	GPIO_PORTF_DATA_R ^= 0x1C;
	int dataTemp = ADC_In();
	if(fabs((dataTemp-data))>10 || fabs((data-dataTemp))>10){
		data = dataTemp;
	}
	int position = Convert(data);
	char *letters = UDec2Str(position);
	for(i = 0; i < 8; i++){UART_OutChar(letters[i]);}
	GPIO_PORTF_DATA_R ^= 0x1C;
	return;
	
}


uint32_t Status[20];             // entries 0,7,12,19 should be false, others true
char GetData[10];  // entries 1 2 3 4 5 6 7 8 should be 1 2 3 4 5 6 7 8
int mainfifo(void){ // Make this main to test FiFo
  FiFo_Init(); // Assuming a buffer of size 6
  for(;;){
    Status[0]  = FiFo_Get(&GetData[0]);  // should fail,    empty
    Status[1]  = FiFo_Put(1);            // should succeed, 1 
    Status[2]  = FiFo_Put(2);            // should succeed, 1 2
    Status[3]  = FiFo_Put(3);            // should succeed, 1 2 3
    Status[4]  = FiFo_Put(4);            // should succeed, 1 2 3 4
    Status[5]  = FiFo_Put(5);            // should succeed, 1 2 3 4 5
    Status[6]  = FiFo_Put(6);            // should succeed, 1 2 3 4 5 6
    Status[7]  = FiFo_Put(7);            // should fail,    1 2 3 4 5 6 
    Status[8]  = FiFo_Get(&GetData[1]);  // should succeed, 2 3 4 5 6
    Status[9]  = FiFo_Get(&GetData[2]);  // should succeed, 3 4 5 6
    Status[10] = FiFo_Put(7);            // should succeed, 3 4 5 6 7
    Status[11] = FiFo_Put(8);            // should succeed, 3 4 5 6 7 8
    Status[12] = FiFo_Put(9);            // should fail,    3 4 5 6 7 8 
    Status[13] = FiFo_Get(&GetData[3]);  // should succeed, 4 5 6 7 8
    Status[14] = FiFo_Get(&GetData[4]);  // should succeed, 5 6 7 8
    Status[15] = FiFo_Get(&GetData[5]);  // should succeed, 6 7 8
    Status[16] = FiFo_Get(&GetData[6]);  // should succeed, 7 8
    Status[17] = FiFo_Get(&GetData[7]);  // should succeed, 8
    Status[18] = FiFo_Get(&GetData[8]);  // should succeed, empty
    Status[19] = FiFo_Get(&GetData[9]);  // should fail,    empty
  }
}

void SysTick_Init(void)
{
	NVIC_ST_CTRL_R = 0x00;			   //
  NVIC_ST_RELOAD_R = 2000000;    // reload value
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;    
  NVIC_ST_CTRL_R = 0x07;		
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
}

char *UDec2Str(unsigned long n) //will return a 32 bit number(like 1332) and turn it into a string like 1.332 which is all ready to print
{
  data[0] = 0x02;
  data[1] = n/1000 + 0x30; // hundreds digit
  data[2] = 0x2E;					//making this a period
	n = n%1000;              // n is now between 0 and 99
  data[3] = n/100 + 0x30;  // tens digit
  n = n%100;               // n is now between 0 and 9
  data[4] = n/10 + 0x30;     // ones digit
  n %= 10;
	data[5] = n + 0x30;            // null termination
	data[6] = '\r';
	data[7] = 0x03;
	return data;
}
