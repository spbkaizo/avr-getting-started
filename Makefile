P=lesson1
OBJECTS=lesson1.o
MCCPU=atmega328p
CFLAGS=-g -mmcu=${MCCPU} 
LDLIBS=
CC=avr-gcc

$(P): 	$(OBJECTS)


# our main target:
ledtest.hex: ledtest.out
      avr-size ledtest.out
      avr-objcopy -R .eeprom -O ihex ledtest.out ledtest.hex
#
# now all the small intermediate things that we need:
ledtest.out: ledtest.o
      avr-gcc -g -mmcu=atmega8 -o ledtest.out -Wl,-Map,ledtest.map ledtest.o
#
#
ledtest.o: ledtest.c
      avr-gcc -g -mmcu=atmega8 -Wall -Os -c ledtest.c

