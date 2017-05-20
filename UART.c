// UART1.c
// Runs on LM4F120/TM4C123
// Use UART1 to implement bidirectional data transfer to and from 
// another microcontroller in Lab 9.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// November 17, 2014
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* Lab solution, Do not post
 http://users.ece.utexas.edu/~valvano/
*/

// U1Rx (VCP receive) connected to PC4
// U1Tx (VCP transmit) connected to PC5
#include <stdint.h>
#include "FiFo.h"
#include "UART.h"
#include "tm4c123gh6pm.h"
#include "LCD.h"
#include "ST7735.h"

uint32_t DataLost; 
// Initialize UART1
// Baud rate is 115200 bits/sec
// Make sure to turn ON UART1 Receiver Interrupt (Interrupt 6 in NVIC)
// Write UART1_Handler
void UART_Init(void)
{
	/*SYSCTL_RCGC1_R |= 0x00000002;  // activate UART1
  SYSCTL_RCGC1_R |= 0x00000004;  // activate port C
	int i = 0;
	i++;
	i++;
	i++;
	i++;
	UART1_IM_R |= 0x10;
	UART1_IFLS_R |= 0x10;
	UART1_IFLS_R &= ~0x28;
	NVIC_PRI1_R |= 0xE00000;
	NVIC_EN0_R |= 0x40;
  UART1_CTL_R &= ~0x00000001;    // disable UART
  UART1_IBRD_R = 43;     // IBRD = int(80,000,000/(16*115,200)) = int(43.40278)
  UART1_FBRD_R = 26;     // FBRD = round(0.40278 * 64) = 26
  UART1_LCRH_R = 0x00000070;  // 8 bit, no parity bits, one stop, FIFOs
  UART0_CTL_R &= ~UART_CTL_HSE; 
  UART1_CTL_R |= 0x00000001;     // enable UART
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R0) == 0){};
  GPIO_PORTC_AFSEL_R |= 0x30;    // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;      // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;   // disable analog on PC5-4 
	// Assumes a 80 MHz bus clock, creates 115200 baud rate */
	
//	int ErrorCount =0; 
	// Assumes a 80 MHz bus clock, creates 115200 baud rate 
	SYSCTL_RCGCUART_R |= 0x0002; // activate UART1 
	SYSCTL_RCGCGPIO_R |= 0x0004; // activate port C 
	FiFo_Init(); 
	UART1_CTL_R &= ~0x0001;	// disable UART 
	UART1_IBRD_R = 43; 
// IBRD=int(80000000/(16*115,200)) = int(43.40277) 
	UART1_FBRD_R = 26;	
// FBRD = round(0.1267 * 64) = 8 
	UART1_LCRH_R = 0x0070;	// 8-bit length, enable FIFO 
	UART1_IFLS_R &= ~0x3F; 
	UART1_IFLS_R += 0x10;		//enabling fifo to interrupt when it is half full 
	UART1_IM_R |= 0x10; 
	UART1_CTL_R |= 0x0301; // enable RXE, TXE and UART 
	GPIO_PORTC_AFSEL_R |= 0x30; // alt funct on Pc4 & pc5 
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000; 
	GPIO_PORTC_DEN_R |= 0x30; // digital I/O on Pc45 
	GPIO_PORTC_AMSEL_R &= ~0x30; // No analog on pc4-5 
		NVIC_PRI1_R |= 0x0E00000; 
		NVIC_EN0_R = NVIC_EN0_INT6;
	

	
}

// input ASCII character from UART
// spin if RxFifo is empty
char UART_InChar(void)
{
  while((UART1_FR_R&0x0010) != 0);      // wait until RXFE is 0
  return((UART1_DR_R&0xFF)); 
}
//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data)
{
  while((UART1_FR_R&0x0020) != 0);      // wait until TXFF is 0
  UART1_DR_R = data;
}

// hardware RX FIFO goes from 7 to 8 or more items
// UART receiver Interrupt is triggered; This is the ISR
void UART1_Handler(void)
{
  char data = 'a';
	while(data != 0x03)
	{
		data = UART_InChar();
		FiFo_Put(data);
	}

	
	
}
