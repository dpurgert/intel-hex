compile: main.c 
	avr-gcc -std=c99 -mmcu=atmega88p -DF_CPU=1000000UL main.c usart.c \
    -o main.elf

upload: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avrdude -e -patmega88p -carduino -P/dev/ttyUSB0 -b19200 \
		-Uflash:w:main.hex:i

read_fuses:
	avrdude -e -patmega88p -carduino -P/dev/ttyUSB0 -b19200 \
		-Ulfuse:r:-:i -Uhfuse:r:-:i -Uefuse:r:-:i

8m_int_fuse:
	avrdude -e -patmega88p -carduino -P/dev/ttyUSB0 -b19200 \
		-Ulfuse:w:0xe2:m -Uhfuse:w:0xdf:m -Uefuse:w:0xf9:m

def_fuse:
	avrdude -e -patmega88p -carduino -P/dev/ttyUSB0 -b19200 \
		-Ulfuse:w:0x62:m -Uhfuse:w:0xdf:m -Uefuse:w:0xf9:m

clean:
	rm *hex *o *elf
