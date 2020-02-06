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
/**
 * @file
 * @brief main.c
 *
 * Core program file for the project that ties the modules together.
 * For detailed descriptions of the functions, see either generated
 * doxygen documentation, or descriptions in main.h
 * 
*/

uint8_t rxbuf[BUFSZ];
uint8_t txbuf[BUFSZ];
uint8_t hxbuf[BUFSZ*2];
 
uint8_t rict, //!<Receive input counter (wire / UDR0 -> rxbuf)
        roct, //!<Receive output counter (rxbuf -> hxbuf)
        hict, //!<Hexfile input counter (rxbuf -> hxbuf)
        hoct, //!<Hexfile output counter (hxbuf -> ???)
        tict, //!<Transmit input counter (??? -> txbuf)
        toct, //!<Transmit output counter (txbuf -> UDR0 / wire)
        rc, //!<Received Byte counter (if >0, bytes to process in rxbuf)
        tc, //!<Transmit Byte counter (if >0, bytes to process in txbuf)
        hxc;//!<Hexfile Byte counter (if >0, bytes to process in hxbuf)


/**
 * @brief USART RX Interrupt
 * 
 * ISR transfers data out of USART data register and into rx buffer
 * for temporary storage until the data can be read into the "hexfile
 * buffer" for hex-format processing.
 */
ISR(USART_RX_vect){
  if (rc<BUFSZ){
    ++rc;
    if (rict>(BUFSZ-1)) {
      //rollover to start of tx buffer FIFO
      rict=0;
    }
    rxbuf[rict++]=UDR0;
  }
  else {
    //just read into trash to clear the int
    uint8_t trash=UDR0;
  }
}

/**
 * @brief USART Tx Interrupt
 *
 * ISR transfers data out of tx buffer and into USART data register for
 * transmission to the remote system
 */
ISR(USART_UDRE_vect){
  if (tc>0) {
    if (toct>(BUFSZ-1)) {
      //rollover to start of tx buffer FIFO
      toct=0;
    }
    UDR0=txbuf[toct++];
    --tc;
  }
  else {
    //nothing more to send, shut down the transmitter
    UCSR0B &= ~(1<<UDRIE0);
  }    
}

void init(){
  initUSART(MYUBRR);
  for (int i = 0; i<BUFSZ; i++) {
    rxbuf[i]=0x00;
    txbuf[i]=0x00;
  }
  for (int i = 0; i<HXSZ; i++) {
    hxbuf[i]=0x00;
  }
  sei(); 
}  


void main() {
  init();
  while(1) {
    rxbtohex();
    prohex();
  }
}

void rxbtohex() {
  if (rc>0){
    --rc;
    ++hxc;
    if ( hict > (HXSZ-1)) {
      //rollover to start of hex buffer FIFO
      hict=0;
    }
    if (roct > (BUFSZ-1)) {
      //rollover to start of rx buffer FIFO
      roct=0;
    }
    hxbuf[hict++]=rxbuf[roct++];
  }
}

void prohex() {
  if (hxc>0){
    --hxc;
    ++tc;
    if ( hoct > (HXSZ-1)) {
      //rollover to start of hex buffer FIFO
      hoct=0;
    }
    if (tict > (BUFSZ-1)) {
      //rollover to start of tx buffer FIFO
      tict=0;
    }
    txbuf[tict++]=hxbuf[hoct++];
  }
  //always try to enable the transmitter.
  sendout();
}

  
void sendout (){
  // enable transmitter ...
  if (tc>0) {
    UCSR0B |= (1<<UDRIE0);
  }
}

/**
 * @brief Copy an arbitrary message into the txbuf FIFO, then enable the 
 * transmitter
*/
void printMsg(uint8_t *msg, uint8_t len){
  for (tc=0; tc<len; tc++) {
    txbuf[tc]=msg[tc];
    sendout();
  }
}
  

/*  Removed for now, since they didn't work.    

uint8_t tohex(uint8_t byte){
  // Convert an incoming character to the hexadecimal number it's
  // intended to convey.  This might be better as a series of 'if' 
  // conditions... 
  switch (byte) {
    case 0x30: {
      //receive ascii 0
      return 0x0;
    }
    case 0x31: {
      //receive ascii 1
      return 0x1;
    }
    case 0x32: {
      //receive ascii 2
      return 0x2;
    }
    case 0x33: {
      //receive ascii 3
      return 0x3;
    }
    case 0x34: {
      //receive ascii 4
      return 0x4;
    }
    case 0x35: {
      //receive ascii 5
      return 0x5;
    }
    case 0x36: {
      //receive ascii 6
      return 0x6;
    }
    case 0x37: {
      //receive ascii 7
      return 0x7;
    }
    case 0x38: {
      //receive ascii 8
      return 0x8;
    }
    case 0x39: {
      //receive ascii 9
      return 0x9;
    }
    case 0x65: {
      //receive ascii A
      return 0xA;
    }
    case 0x66: {
      //receive ascii B
      return 0xB;
    }
    case 0x67: {
      //receive ascii C
      return 0xC;
    }
    case 0x68: {
      //receive ascii D
      return 0xD;
    }
    case 0x69: {
      //receive ascii E
      return 0xE;
    }
    default: {
      //received ascii F
      return 0x0F;
    }
  }
}

uint8_t cksum(uint8_t data){
    /*  Verify the checksum.  For Intel *hex files, the checksum 
    *  value provided in a data line is the two's compliment of the 
    *  sum of all data bytes.  A record can be validated by adding all
    *  bytes received to the final checksum value; a result of zero (0)
    *  indicates all is well.  Any other value is an error.
    /
    int sum=0;
    for (int i=0; i<=dtp; i++) {
      sum=sum+prombuf[pb].pagedata[i];
    }
    sum = sum + data;
    return sum;
}
*/
