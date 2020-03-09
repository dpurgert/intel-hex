/***********************************************************************
*                              File: pinouts/chips.h
*                     Copyright (c): 2019, Dan Purgert
*                                  : dan@djph.net
*                   
*                           License: GNU GPL v2 only
*                       Description: Common includes, so we don't have  
*                                  : to include things 900 billion times
*                                  : throughout the project.  
*                       
*                     Prerequisites: 
*                                  : avr-gcc >= 4.9.2
*                                  : avrdude >= 6.3-2 (Debian)
*                                  : make
************************************************************************/

/**
 * @file
 * @brief Common pin naming scheme, for future expandability.
 *
 * This file is a common chip definition library to allow simpler port 
 * and pin names to be referenced in the rest of the program. 
*/

#ifndef __CHIP_INC_H__
  #define __CHIP_INC_H__ 1

  #include "../common.h"
  
  #ifndef PB0
    #define PB0     PORTB0
    #define PB1     PORTB1
    #define PB2     PORTB2
    #define PB3     PORTB3
    #define PB4     PORTB4
    #define PB5     PORTB5
    #define PB6     PORTB6
    #define PB7     PORTB7
  #endif
  #ifndef PC0
    #define PC0     PORTC0
    #define PC1     PORTC1
    #define PC2     PORTC2
    #define PC3     PORTC3
    #define PC4     PORTC4
    #define PC5     PORTC5
    #define PC6     PORTC6
    #define PC7     PORTC7
  #endif
  #ifndef PD0
    #define PD0     PORTD0
    #define PD1     PORTD1
    #define PD2     PORTD2
    #define PD3     PORTD3
    #define PD4     PORTD4
    #define PD5     PORTD5
    #define PD6     PORTD6
    #define PD7     PORTD7
  #endif
    #ifndef PH0
    #define PH0     PORTH0
    #define PH1     PORTH1
    #define PH2     PORTH2
    #define PH3     PORTH3
    #define PH4     PORTH4
    #define PH5     PORTH5
    #define PH6     PORTH6
    #define PH7     PORTH7
  #endif

/* Chip Specific Pinouts */

#if defined (__AVR_ATmega48__) \
  || defined (__AVR_ATmega48P__) \
  || defined (__AVR_ATmega88__) \
  || defined (__AVR_ATmega88P__) \
  || defined (__AVR_ATmega168) \
  || defined (__AVR_ATmega168P__) \
  || defined (__AVR_ATmega328P__)
  #include "ATmegax8.h"
#else
  #error "Unknown Chip!"
#endif

#endif //ifndef CHIP_INC_H
