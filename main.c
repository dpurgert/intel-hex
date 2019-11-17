/***********************************************************************
*                              File: main.c
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

#include "main.h"
uint8_t rxbuf[BUFSZ];
uint8_t txbuf[BUFSZ];
uint8_t ibuf,obuf,tin,tout,rc,tc;


ISR(USART_RX_vect){
  if (rc<BUFSZ){
    ++rc;
    if (ibuf>(BUFSZ-1)) {
      ibuf=0;
    }
    rxbuf[ibuf++]=UDR0;
  }
  else {
    //just read into trash to clear the int
    uint8_t trash=UDR0;
  }
}

ISR(USART_UDRE_vect){
  if (tc>0) {
    //we have stuff to transmit
    if (tout>(BUFSZ-1)) {
      tout=0;
    }
    UDR0=txbuf[tout++];
    --tc;
  }
  else {
    //nothing more to send, shut down the transmitter
    UCSR0B &= ~(1<<UDRIE0);
  }    
}

void init(){
  initUSART(MYUBRR);
  sei(); 
}  


void main() {
  init();
  while(1) {
      rxbtxb();
      sendout();
  }
}

void rxbtxb(){
  if (rc>0) {
    --rc;
    ++tc;
    if (tin > (BUFSZ-1)) {
      //if we reached the end of the fifo, rollover to start
      tin=0;
    }
    if (obuf > (BUFSZ-1)) {
      //if we reached the end of the fifo, rollover to start
      obuf=0;
    }
    txbuf[tin++]=rxbuf[obuf++];
  }
}

void sendout (){
  if (tc>0) {
    UCSR0B |= (1<<UDRIE0);
  }
}
