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

/**
 * @file
 * @brief USART control header
 *
 * Header file for USART control / initialization routines.  As we're
 * using interrupt-based serial, there's not much to do here.
*/

#ifndef __HEX_USART__
  #define __HEX_USART__ 1
  #include "common.h"

  /**
   * @brief Initialize USART
   * Subroutine to set the USART0 control registers (baudrate, receive
   * enable, etc.) in preparation of using the USART for asynchronous
   * serial communication.
  */
  void initUSART(uint16_t ubrr);
  /**
   * @brief Transmit Byte
   * Can send a byte without needing to use the FIFOs.  Only for testing
   * and should not be used anywhere in live code.
  */
  void txUSART(uint8_t data);

#endif 
