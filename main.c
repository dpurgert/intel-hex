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
uint8_t txbuf[TBUFSZ];
uint8_t hxbuf[HXSZ];
 
uint8_t rict, //!<Receive input counter (wire / UDR0 -> rxbuf)
        roct, //!<Receive output counter (rxbuf -> hxbuf)
        hict, //!<Hexfile input counter (rxbuf -> hxbuf)
        hoct, //!<Hexfile output counter (hxbuf -> ???)
        tict, //!<Transmit input counter (??? -> txbuf)
        toct, //!<Transmit output counter (txbuf -> UDR0 / wire)
        rc, //!<Received Byte counter (if >0, bytes to process in rxbuf)
        tc, //!<Transmit Byte counter (if >0, bytes to process in txbuf)
        hxc,//!<Hexfile Byte counter (if >0, bytes to process in hxbuf)
        curst,//!<State Machine current state.
        dtsz,//!<Number of bytes left to read in ByteCount segment
        adrsz,//!<Number of bytes left to read in Address segment
        rtsz,//!<Number of bytes left to read in Record Type segment
        cksz,//!<Number of bytes left to read in Checksum segment
        rtd,//!<Record Type Data
        dtl,//!<Number of Bytes left to read in Data segment (2*dtsz)
        dtb,//!<DataBuffer
        ckb,//!<Checksum Buffer 
        dtc,//!<High nibble (0) or low nibble(1)
        rdbuf,//!<spare readbuffer.
        eofct,//!<to read in 'ff' for end record
        dtp;//!<Byte counter for EEPROM PageData

uint16_t adr; //!<EEPROM Address Word from Ihex file


/**
 * @breif State machine counters
 *
 * Enumerate the various states we could possibly be in while processing
 * the hexbuf.  
*/
enum data_states {
  INITST,  //!<FSM Initialization
  DATASZ,  //!<Data size byte (2 characters)
  ADDRLOC, //!<Address Offset bytes (4 characters)
  RECTYP,  //!<Record Type byte (2 characters)
  DATA,    //!<Data bytes (2*DATASZ characters) 
  CKSUM,   //!<Checksum Verification byte (2 characters)
  END,     //!<EOF Received, return to INITST
  ERRORST, //!<Something went wrong. Send an alert, wait for reset
};

/**
 * @brief EEPROM page storage
 * 
 * Stores the EEPROM page data until we've written it out to the
 * connected device.  16 bit addresses integer conforms to ihex format
 * using 16-bit address offsets.
*/
struct promData {
  uint16_t addr;
  uint8_t pagedata[PGSZ];
} PROM;

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
    if (toct>(TBUFSZ-1)) {
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
  curst=INITST;
  sei(); 
}  


void main() {
  init();
  #if DEBUG
    uint8_t mainmsg[]="Begin.\n";
    printMsg(mainmsg,7);
  #endif
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
    if ( hoct > (HXSZ-1)) {
      //rollover to start of hex buffer FIFO
      hoct=0;
    }
    uint8_t bt = hxbuf[hoct++];
    
    /**
     * @brief State Machine logic
     *
     * State machine to work through a data record.
    */
    switch((int)curst) {
      case INITST: {
        printAscii(bt);
        if (bt==0x0A || bt==0x0D) {
          //Carriage Return or Line Feed.  Do nothing
          ;
        }
        else if (bt!=0x3A) {
          //if we're in initstate and the character is NOT a ":", 
          //something is very wrong
          curst=ERRORST;
        } 
        else {
          //In INITST and received ":".  Reset positional data for
          //reading this record, and move into reading the ByteCount
          //segment (DATASZ state)
          curst=DATASZ;
          dtsz=2; //2 characters to 1 byte data size (record length)
          adrsz=4;//4 characters to 2 bytes address location
          rtsz=2; //2 characters to 1 byte record type
          cksz=2; //2 characters to 1 byte checksum
          eofct=2; //2 characters to 1 byte EOF 
          rtd=0; //record type reset
          dtl=0; //data length reset
          dtb=0; //data buffer reset
          dtc=0; //data counter reset
          dtp=0; //data position reset
          //reset EEPROM page
          for (int i=0;i<PGSZ;i++) {
            PROM.pagedata[i]=0x00;
          }
        }
        break;
      }
      
      case DATASZ: {
        if (dtsz==2) {
          dtl=(tohex(bt)<<4);
          --dtsz;
        }
        else if (dtsz>0){
          dtl|=(tohex(bt));
          dtl=dtl*2; //double because ihex uses 2B per actual byte
          --dtsz;
          curst=ADDRLOC;
        }
        else {
          //for some reason, we didn't get out of DATASZ
          curst==ERRORST;
        }
        break;
      }
   
      case ADDRLOC: {
        if (adrsz==4) {
          adr=(tohex(bt)<<12);
          --adrsz;
        }
        else if (adrsz==3) {
          adr|=(tohex(bt)<<8);
          --adrsz;
        }
        else if (adrsz==2) {
          adr|=(tohex(bt)<<4);
          --adrsz;
        }
        else if (adrsz>0){
          adr|=(tohex(bt));
          PROM.addr=adr;
          curst=RECTYP;
        }
        else {
          //for some reason, we didn't get out of DATASZ
          curst==ERRORST;
        }
        break;
      }

      case RECTYP: {
        //check if it's data or EOF
        if (rtsz==2) {
          rtd=(tohex(bt)<<4);
          --rtsz;
        }
        else if (rtsz>0){
          rtd|=(tohex(bt));
          --rtsz;
          if (rtd==0x00) {
            curst=DATA;
            #ifdef DEBUG
              uint8_t recmsg[]="todata.";
              printMsg(recmsg,7);
              printAscii(rtd);
            #endif
          }
          else if (rtd==0x01) {
            #ifdef DEBUG
              uint8_t recmsg2[]="toend.";
              printMsg(recmsg2,6);
            #endif
            curst=END;
            hxc=1; //make sure we have at least one byte to "process"
          }
        }
        else {
          curst=ERRORST;
        }
        break;
      }
      
      case DATA: {
        if ((dtc==0)&&(dtl>0)) {
          //high nibble, dtc is even.
          dtb=(tohex(bt)<<4);
          ++dtc;
          --dtl;
        }
        else if ((dtc==1)&&(dtl>0)) {
          //low nibble, dtc is odd
          dtb|=(tohex(bt));
          --dtc;
          --dtl;
          PROM.pagedata[dtp++]=dtb;
          printAscii(dtb);
          if (dtl==0) {
            //Finished reading the data
            curst=CKSUM; //finished 
          }
        }
        else if (dtc>1) {
          //uhoh...
          curst==ERRORST;
        }
        break;
      }  

      case CKSUM: {
        //store checksum to buffer, then check data
        if (cksz==2) {
          --cksz;
          ckb=(tohex(bt)<<4);
        }
        else if (cksz>0){
          --cksz;
          ckb|=(tohex(bt));
          if (cksum(ckb)==0x00) {
            //everything's OK
            #if DEBUG
              uint8_t ckmsg[]="OK!\n";
              printMsg(ckmsg,4);
            #endif
            curst=INITST;
          }
          else {
            //checksum failed
            #if DEBUG
              uint8_t ckmsg2[]="NOK!\n";
              printMsg(ckmsg2,5);
            #endif
            curst=ERRORST;
          }
        break;
        }

        case ERRORST: {
          uint8_t errout[]="ERROR.\n";
          printMsg(errout,7);
          printAscii(bt);
          break;
        }

        case END: {
          if (eofct==2) {
            --eofct;
            rdbuf=(tohex(bt));
          }
          else {
            rdbuf|=(tohex(bt));
          }
          if (rdbuf==0xFF && hxc==0x00) {
            curst=INITST;
            uint8_t outmsg[]="EOF.\n";
            printMsg(outmsg,5);
          }
          break;
        }
        
        default: {
          //do nothing
          break;
        }
      }
    }
  }
  //always try to enable the transmitter.
  //sendout();
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
  for (int i=0; i<len; i++) {
    if (tict>(TBUFSZ-1)) {
      //rollover to start of TX input buffer FIFO
      tict=0;
    }
    txbuf[tict++]=msg[i];
    tc++;
  }
  //turn on the transmitter
  sendout();
}
  


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
    case (0x41): {
      //receive ascii A
      return 0xA;
    }
    case 0x42: {
      //receive ascii B
      return 0xB;
    }
    case 0x43: {
      //receive ascii C
      return 0xC;
    }
    case 0x44: {
      //receive ascii D
      return 0xD;
    }
    case 0x45: {
      //receive ascii E
      return 0xE;
    }
    case (0x61): {
      //receive ascii a
      return 0xA;
    }
    case 0x62: {
      //receive ascii b
      return 0xB;
    }
    case 0x63: {
      //receive ascii c
      return 0xC;
    }
    case 0x64: {
      //receive ascii d
      return 0xD;
    }
    case 0x65: {
      //receive ascii e
      return 0xE;
    }
    default: {
      //received ascii F (or f)
      return 0xF;
    }
  }
}

uint8_t cksum(uint8_t data){
    /*  Verify the checksum.  For Intel *hex files, the checksum 
    *  value provided in a data line is the two's compliment of the 
    *  sum of all data bytes.  A record can be validated by adding all
    *  bytes received to the final checksum value; a result of zero (0)
    *  indicates all is well.  Any other value is an error.
    */
    uint8_t sum=0;
    #if DEBUG
      uint8_t summsg[]="cksum: ";
      printMsg(summsg,6);
      printAscii(data);
    #endif 
    for (int i=0; i<dtp; i++) {
      sum=sum+PROM.pagedata[i];
    }
    #if DEBUG
      uint8_t summsg2[]="datsum: ";
      printMsg(summsg2,7);
      printAscii(sum);
    #endif 
    sum = sum + data;
    #if DEBUG
      uint8_t summsg3[]="totsum: ";
      printMsg(summsg3,7);
      printAscii(sum);
    #endif 
    
    return sum;
}

void printAscii (uint8_t data){
  uint8_t outdat[4];
  switch ((data&0xF0)>>4){
    case 0x0: outdat[0]='0'; break;
    case 0x1: outdat[0]='1'; break;
    case 0x2: outdat[0]='2'; break;
    case 0x3: outdat[0]='3'; break;
    case 0x4: outdat[0]='4'; break;
    case 0x5: outdat[0]='5'; break;
    case 0x6: outdat[0]='6'; break;
    case 0x7: outdat[0]='7'; break;
    case 0x8: outdat[0]='8'; break;
    case 0x9: outdat[0]='9'; break;
    case 0xA: outdat[0]='A'; break;
    case 0xB: outdat[0]='B'; break;
    case 0xC: outdat[0]='C'; break;
    case 0xD: outdat[0]='D'; break;
    case 0xE: outdat[0]='E'; break;
    default: outdat[0]='F'; break;
  }
  switch ((data&0x0F)){
    case 0x0: outdat[1]='0'; break;
    case 0x1: outdat[1]='1'; break;
    case 0x2: outdat[1]='2'; break;
    case 0x3: outdat[1]='3'; break;
    case 0x4: outdat[1]='4'; break;
    case 0x5: outdat[1]='5'; break;
    case 0x6: outdat[1]='6'; break;
    case 0x7: outdat[1]='7'; break;
    case 0x8: outdat[1]='8'; break;
    case 0x9: outdat[1]='9'; break;
    case 0xA: outdat[1]='A'; break;
    case 0xB: outdat[1]='B'; break;
    case 0xC: outdat[1]='C'; break;
    case 0xD: outdat[1]='D'; break;
    case 0xE: outdat[1]='E'; break;
    default: outdat[1]='F'; break;
  }
  outdat[2]=0x0A;
  printMsg(outdat,3);
}
