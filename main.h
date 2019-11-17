/***********************************************************************
*                              File: main.h
*                     Copyright (c): 2019, Dan Purgert
*                                  : dan@djph.net
*                   
*                           License: GNU GPL v2 only
*                       Description: Main Setup / configuration routines
*                                  : for the project.  Ties all the 
*                                  : various modules together
*                       
*                     Prerequisites: 
*                                  : avr-gcc >= 4.9.2
*                                  : avrdude >= 6.3-2 (Debian)
*                                  : make
************************************************************************/
#ifndef __HEX_MAIN__
  #define __HEX_MAIN__ 1
  #include "common.h"

  //TODO: replace these hard definitions with a read from EEPROM
  #define BAUD 4800
  #define MYUBRR (F_CPU/16/BAUD-1)
  #define BUFSZ 16
  extern uint8_t rxbuf[BUFSZ];
  extern uint8_t txbuf[BUFSZ];

  void init();
  void main();
  void sendout();
  void rxbtxb();
 
#endif
