///@mainpage

# intel-hex
AVR parser for intel hex

If this works, long term plan is to make an EEPROM programmer.

##Revision History
version 0.4 (sure, let's go with that)
 - Successfully reads and verifies a data record.
 - Successfully determines an EOF record.

 - TODO:
    - Add TWI interface for EEPROM (it's all I have)
    - Update 'CKSUM' state to kickoff write to EEPROM
    - Break 'help' functions out of main.c
    - Documentation

## Documentation
Full documentation can be generated using doxygen on the included
Doxyfile (note, you will need to create a "docs" subdirectory first).
It is currently a work in progress, and for the moment, tracking the
docs is a waste of time and bandwidth.
