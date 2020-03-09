/***********************************************************************
*                              File: twi.h
*                     Copyright (c): 2019, Dan Purgert
*                                  : dan@djph.net
*                   
*                           License: GNU GPL v2 only
*                       Description: TWI Setup / configuration routines
*                                  : for the project.
*                       
*                     Prerequisites: 
*                                  : avr-gcc >= 4.9.2
*                                  : avrdude >= 6.3-2 (Debian)
*                                  : make
************************************************************************/

/**
 * @file
 * @brief TWI control header
 *
 * Header file for TWI control / initialization routines. 
*/

#ifndef __HEX_TWI__
  #define __HEX_TWI__ 1
  #include "common.h"

  /**
   * @brief Initialize TWI  
   * Subroutine to set the TWI0 control registers 
  */
  void initTWI();
  /**
   * @brief Transmit Byte
   *
   * Transmit a byte to a TWI slave
  */
  void txTWI(uint8_t adr, uint8_t data);

#endif 
