/***********************************************************************
*                              File: main.h
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

#ifndef __HEX_COMMON__
  #define __HEX_COMMON__ 1

  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <stdint.h>
  #include "main.h"
  #include "usart.h"

#endif


