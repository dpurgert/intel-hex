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
uint8_t hxbuf[BUFSZ*2];
uint8_t ibuf,obuf,tin,tout,rc,tc,pb,curst,dtsz,adrsz;
uint8_t rt,dtl,dtb,dtc,dtp,cksz,ckb;
uint16_t fin,fout,fct,adr;

enum data_states {
  INITST,  //FSM Initialization
  DATASZ,  // Data size
  ADDRLOC, // Address Offset
  RECTYP,  // Record Type
  DATA,    // Data 
  CKSUM,   // Checksum Verification
  END,     // EOF
  ERRORST, // Something went wrong. Send an alert, wait for reset
};

struct promData {
  uint16_t addr;
  uint8_t pagedata[BUFSZ];
};

struct promData prombuf[BUFSZ];

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
  curst=INITST;
  pb=0;
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
    ++fct;
    if (fin > ((BUFSZ*2)-1)) {
      //if we reached the end of the fifo, rollover to start
      fin=0;
    }
    if (obuf > (BUFSZ-1)) {
      //if we reached the end of the fifo, rollover to start
      obuf=0;
    }
    hxbuf[fin++]=rxbuf[obuf++];
  }
}

void prohex(){
  int tmp=0x0;
  if ((curst==INITST) && \
    (hxbuf[fout]==0x3a)||(hxbuf[fout]==0x0A)||(hxbuf[fout]==0x0D)) {
    //strip off :, \n, or \r, as they're control / human readability
    //characters, and they're not necessary to the state machine
    ++fout;
    --fct;
  }
  //don't bother unless we have at least 2 bytes to work with
  if ((fct-2)>=0) {
    switch((int)curst) {
      case INITST: {
        curst=DATASZ;
        dtsz=2; //2 characters to 1 byte data size (record length)
        adrsz=4;//4 characters to 2 bytes address location
        rt=2; //2 characters to 1 byte record type
        cksz=2; //2 characters to 1 byte checksum
        dtl=0; //data length reset
        dtb=0; //data buffer reset
        dtc=0; //data counter reset
        dtp=0; //data position reset
        break;
      }

      case DATASZ: {
        //data record length
        --fct;
        if (dtsz==2) {
          tmp=(tohex(hxbuf[fout++])<<4);
          dtl=tmp;
          --dtsz;
        }
        else {
          tmp=(tohex(hxbuf[fout++]));
          dtl|=tmp;
          dtc=(dtl*2); //2 characters per byte
          --dtsz;
          curst=ADDRLOC;
        }
        break;
      }

      case ADDRLOC: {
        --fct;
        if (adrsz==4) {
          tmp=(tohex(hxbuf[fout++])<<12);
          adr=tmp;
          --adrsz;
        }
        else if (adrsz==3) {
          tmp=(tohex(hxbuf[fout++])<<8);
          adr|=tmp;
          --adrsz;
        }
        else if (adrsz==2) {
          tmp=(tohex(hxbuf[fout++])<<4);
          adr|=tmp;
          --adrsz;
        }
        else {
          tmp=(tohex(hxbuf[fout++]));
          adr|=tmp;
          prombuf[pb].addr=adr;
          curst=RECTYP;
        }
        break;
      }

      case RECTYP: {
        //check if it's data or EOF
        --fct;
        if (rt==2) {
          --rt;
          ++fout;
        }
        else {
          tmp=(tohex(hxbuf[fout++]));
          if (tmp==0x0) {
            curst=DATA;
          }
          else {
            curst=END;
          }
        }
        break;
      }

      case DATA: {
        --fct;
        if ((dtc%2==0)&&(dtl>0)) {
          //high nibble, dtc is even.
          tmp=(tohex(hxbuf[fout++])<<4);
          dtb=tmp;
          --dtc;
        }
        else if ((dtc%2!=0)&&(dtl>0)) {
          //low nibble, dtc is odd
          tmp=(tohex(hxbuf[fout++]));
          dtb|=tmp;
          --dtc;
          --dtl;
          prombuf[pb].pagedata[dtp++]=dtb;
        }
        else {
          //we're out of data, increment the prombuffer
          //and check the data
          curst=CKSUM; //finished 
        }
        break;
      }

      case CKSUM: {
        //store checksum to buffer, then check data
        --fct;
        if (cksz==2) {
          --cksz;
          tmp=(tohex(hxbuf[fout++])<<4);
          ckb=tmp;
        }
        else {
          --cksz;
          tmp=(tohex(hxbuf[fout++]));
          ckb|=tmp;
          if (cksum(ckb)==0) {
            //everything's OK
            ++pb;
            curst=INITST;
          }
          else {
            //checksum failed
            curst=ERRORST;
          }
        break;
        }
      }

      case ERRORST: {
        //do nothing for now.
        break;
      }

      case END: {
        //do nothing for now.
        break;
      }

      default: {
        //something went wrong.
        break;
      }

    }
  }
}

void sendout (){
  if (tc>0) {
    UCSR0B |= (1<<UDRIE0);
  }
}

uint8_t tohex(uint8_t byte){
  switch (byte) {
    case 0x30: {
      //receive ascii 0
      return 0x0;
    }
    default: {
      //received ascii F
      return 0x0F;
    }
  }
}

uint8_t cksum(uint8_t data){
    int sum=0;
    for (int i=0; i<=dtp; i++) {
      sum=sum+prombuf[pb].pagedata[i];
    }
    sum = sum + data;
    return sum;
}
