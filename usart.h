/***********************************************************************
*                              File: usart.h
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

#ifndef __HEX_USART__
  #define __HEX_USART__ 1
  #include "common.h"

  void initUSART(uint16_t ubrr);
  void txUSART(uint8_t data);

#endif 
