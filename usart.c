/***********************************************************************
*                              File: usart.C
*                     Copyright (c): 2019, Dan Purgert
*                                  : dan@djph.net
*                   
*                           License: GNU GPL v2 only
*                       Description: UART Setup / configuration routines
*                                  : for the project.  TX & RX will be 
*                                  : interrupt driven
*                       
*                     Prerequisites: 
*                                  : avr-gcc >= 4.9.2
*                                  : avrdude >= 6.3-2 (Debian)
*                                  : make
************************************************************************/

/**
 * @file
*/

#include "usart.h"
void initUSART(uint16_t ubrr){
  //UBRR0H = (uint8_t)(ubrr>>8);
  //UBRR0L = (uint8_t) ubrr;
  /**
   * To facilitate testing, UBRR0H/UBRR0L are hardcoded to 4800 baud.
   * this needs to be changed.
  */
  UBRR0H = 0x00;
  UBRR0L = 0x0C;
  /**
   * @brief Clear control register UCSR0A
   * Clear UCSR0A - handles interrupts, frame errors,
   * double-speed, and MCPM. 
  */
  UCSR0A = 0;

  /**
   * @brief Set control register UCSR0B
   * Enable Transmitter, Receiver, and Receive Complete Interrupt
   * flags.  UCSR0B also handles the Data Register Empty interrupts
   * for transmission ..
  */
  UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

  /**
   * @brief Set transmission parameters to 8N1 
   * UCSR0C also handles Mode select (SPI / Sync / Async UART)
  */
  UCSR0C = (1<<USBS0) | (3<<UCSZ00);

}

void txUSART (uint8_t data){
    while ( !(UCSR0A & (1<<UDRE0))) 
    ;
  UDR0=data;
}
