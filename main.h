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

//Testing doxygen for this project.  I'm not entirely sure if this helps
//or hinders.

/**
 * @file
 * @brief Main configuration
 *
 * In addition to standard header definitions, we also define a few of
 * the static definitions.  Some may be better served in other places.
*/
#ifndef __HEX_MAIN__
  #define __HEX_MAIN__ 1
  #include "common.h"

  /**
   * @brief debug mode.
  */
  #define DEBUG 1

  //TODO: replace the USART hard definitions with a read from EEPROM
  #define BAUD 4800
  #define MYUBRR (F_CPU/16/BAUD-1)
  /**
   * @brief EEPROM Page size.
   * 
   * Default assumes 64 byte pages, as per 28Cxx EEPROM
  */
  #define PGSZ 64
  /**
   * @brief Rx buffer size - 3x PGSZ (192 Bytes)
  */
  #define BUFSZ PGSZ*3
  /**
   * @brief Tx buffer size - PGSZ
  */
  #define TBUFSZ PGSZ 
  /**
   * @brief Hex File buffer size. PGSZ*2 for now, testing may prove it
   * can be smaller.
  */ 
  #define HXSZ PGSZ*2 
  /**
   * @brief FIFO buffer for USART Rx
  */
  extern uint8_t rxbuf[BUFSZ];
  /**
   * @brief FIFO buffer for USART Tx
  */
  extern uint8_t txbuf[TBUFSZ];
  /**
   * @brief Hex "file" processing buffer
  */
  extern uint8_t hxbuf[HXSZ];

  /**
   * @brief Initialize peripherals
   */
  void init();
  /**
   * @brief Main program / loop
   */
  void main();
  /**
   * @brief Enable USART transmitter
   * 
   * Enables the transmitter if there's anything in the transmit counter.
  */
  void sendout();
  /**
   * @brief Receive buffer to Hex Buffer
   *
   * This function moves data out of the receive buffer and into the
   * larger hexfile buffer.  May be able to do without this, and process
   * directly out of RX buffer.
  */
  void rxbtohex();
  /**
   * @brief Process Hex Buffer
   *
   * Work through the hex buffer.  For the moment, this is just copying
   * our hxbuf into our txbuf in order to echo the message back.  This is
   * to prove our FIFOs are behaving.
  */
  void prohex();
  void printMsg(uint8_t *data, uint8_t len);
  /**
   * @brief Verify data record checksum
   *
   * This function sums all bytes of a data record along with the final
   * checksum byte.  By definition, this should return '0' for
   * successful reception of data and checksum.  Any other answer
   * indicates there was either a transmission error, or the input file
   * itself was malformed.
   */
  uint8_t cksum(uint8_t data);
  /**
   * @brief Translate incoming ASCII 
   *
   * This function translates the incoming ASCII character stream into
   * the correct high/low nibble for writing to the target.
   *
   * For example, the incoming 24 bytes of incoming ASCII text
   *  48656C6C6F20576F726C6421
   * 
   * Will be converted to the 12 bytes
   *  0x48 0x65 0x6C 0x6C 0x6F 0x20 0x57 0x6F 0x72 0x6C 0x64 0x21 
   *   (ASCII - Hello World!)
   */
  uint8_t tohex(uint8_t byte);

  /**
   * @brief Translate outgoing data
   *
   * This function translates a byte of data into a 2-byte ASCII
   * representation, suitable for transmission back out over the USART.
   *
   * For example, an integer with the value "0x01" will be converted
   * into the char array [0,1] (ASCII 0x30, 0x31) for transmission.
   */
  void printAscii(uint8_t data);
  
#endif
