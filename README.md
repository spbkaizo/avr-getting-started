```   
/*
 * Copyright (c) 2008-2010 Chris Kuethe <chris.kuethe@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

```

A Brief Tutorial on Programming the AVR without Arduino

   Motivation

      Just to get started on the wrong foot: "sometimes Arduino is the wrong
      choice." Depending on what you're doing, maybe your app's structure
      isn't very well suited to the Arduino framework. Maybe you need to
      write smaller code than what would be produced by Arduino, sometimes
      you just can't run Arduino at all...

      I'm in that latter category. I'm not such a big fan of Java, and I'm
      comfortable with a text editor and driving the compiler by hand, so I'm
      going to do this the hard way. Arduino trowels some nice plaster over
      top but it's nothing you can't do by hand. Really, all you should need
      is avr-gcc, avr-libc, avr-binutils and avrdude. Getting these packages
      is beyond the scope of this document, it's quite likely there are
      pre-built packages for your OS.

      This document was written give a basic introduction to some of the
      specifics of AVR programming, assuming you already have a handle on C.
      It approximates the order I came to understand things while learning to
      program the AVR. I'm doing a lot of this "the hard way", there are a
      number of macros in avr-libc or avrlib to do much of this but it's
      important to understand the underlying principles. If you understand
      what's going on, moving your code (from a '168 to a '644P for example)
      is very easy.
       1. [1]A blinking LED - Busy Waits and IO ports
       2. [2]Two blinking LEDs - Addressing Pins
       3. [3]A switched LED - Digital Input and output
       4. [4]Serial Output
       5. [5]Serial Input
       6. [6]Putting it together: printing button presses and controlling
          LEDs
       7. [7]Analog Output
       8. [8]avr-libc goodies
       9. [9]Analog Input
      10. [10]A blinking LED - Interrupts and timers
      11. [11]Persistent Storage
      12. [12]Input Capture
      13. [13]Watchdog
      14. [14]I2C/SPI peripherals

        * [15]ATmega644P-specific
        * [16]ATtiny85-specific
        * [17]AT90USB162-specific
        __________________________________________________________________

   Lesson 1: A blinking LED - Busy Waits and IO ports

      This section is an introduction to microcontrollers. Out of the box,
      your microcontroller won't do anything - you need to load a program
      before it's useful. Here I present the the hardware equivalent of
      "Hello World" - a blinking light.

      To begin, let's look at the [18]pin map. We see that Arduino pin 13 is
      PB5 on the ATmega168 - part of port B. To use this pin, port B must
      first be set to be an output pin. There are various ways to do this[2]
      - writing to Port B Data Direction Register[3] at address 0x24 or the
      lazy/better way, using the DDRB macro. Let's keep things simple and set
      the whole port to output with DDRB = 0xff;. Finally, we can start
      writing to PORTB (address 0x25).

      A simple approach would would be to do something like this:
   while(1){
           PORTB = 0xff;
           _delay_ms(500);
           PORTB = 0x00;
           _delay_ms(500);
   }

      which makes use of the delay routines defined in <util/delay.h>. If you
      try run this, you'll find your LED blinking very rapidly. Far more
   rapidly than you'd like.

   A brief inspection of delay.h is instructive: with respect to
   _delay_ms, "The maximal possible delay is 262.14 ms / F_CPU in MHz."
   This can easily be addressed by computing the maximum time _delay_ms
   will sleep, and given that, the number of times to call _delay_ms to
   achieve the desired delay interval. You may wish to put this
   computation into a separate function which you can call whenever you
   need to delay.

   [19]lesson1.c is one possible solution.
     __________________________________________________________________

Lesson 2: Two blinking LEDs - Addressing Pins

   This section builds on the previous section. By now you're probably
   bored of looking at the onboard LED blinking. That's good. Let's try
   hooking up some other LEDS. If you try this, you'll find they'll all
   blink together. Why?

   Because you asked for it with PORTB = 0xff;. More astute readers may
   have figured out that writing all 1's to the port would set all the
   pins to 1. If you're new to this... well, I just said it. By the same
   token, writing 1 to only a single bit in the port's register will only
   turn that pin on.

   Recall that the Arduino pin map said that pin 13 - the LED - is PB5. In
   other words, bit 5 of port B, or 0x20. Take a copy of your solution
   from lesson 1, and change it to write 0x20 to PORTB, rather than 0xff.
   After doing this, your program should blink just the onboard LED.

   Now that you have control over a given port, extend your program to
   alternate or cycle the LEDs [20]lesson2.c is one possible solution.
     __________________________________________________________________

Lesson 3: A switched LED - Digital Input and output

   We've now become comfortable with basic digital output, let's work on
   getting signals in.

   Bear with me a moment while we look at some background on inputs. We
   like to think of processors as purely digital entities - ones and
   zeros, highs and lows. Alas, there are plenty of analog signals inside,
   and depending on your processor (like AVRs and PICs) you may even have
   specific analog inputs. A pin connected to ground is logic 0, and a pin
   connected to power is logic 1, but what value does an unconnected pin
   have? Indeterminate. The pin is said to be floating.

   Typically, pins are connected to high or low through a relatively
   high-value resistor, called a pull-up or pull-down. This offers a weak
   signal to that pin, putting it into a known state. This known state is
   then selectively overriden by shorting the pin to low or high. Think of
   it as someone yelling "zero", and someone whispering "one". The very
   loud zero will overpower the quiet one, but in the absence of other
   input, you'll hear the one. As to what value of resistor to use, I've
   heard "it depends", "low enough to get the signal, high enough to not
   waste power", and "47K". My point in mentioning this is to let you know
   that something will need to be done to the input signal so the AVR
   knows it's there.

   If you're lazy (or wise), you can tell the AVR to use internal pull-ups
   rather than having to mess with them on your own. Using Port D as an
   example, let's set up Digital Pin 2 (PD2) as an input with internal
   pull-up:
DDRD &= 0xFB; /* leave all the other bits alone, just zero bit 2 */
PORTD |= 0x04 /* leave all the other bits alone, just set bit 2 */
value = PORTD & 0x04; /* and that's what's on the pin... */

   Try a program to read a switch connected between digital pin 2 and
   ground, and use this to control an LED. One possible solution is
   [21]lesson3.c
     __________________________________________________________________

Lesson 4: Serial Output

   Serial is icky. It took several passes through chapter 19 of the
   ATmega168 datasheet before all the pieces started to come together. I'd
   advise against copying the sample code from the datasheet, at least
   until you understand where the magic bits come from. I'd also recommend
   against making the init function too configurable; just set up the com
   port once and be done with it. Flexible configuration makes your
   program significantly larger.

   The first set of magic bits (Baud Rate Register) is found in table
   19-1. This is very machine dependent; you must recompile if you change
   your device's clock rate. The datasheet also includes tables listing
   the magic values. The BRR value is written into a clock generator which
   clocks the UART. Since the baud rate register is 12 bits wide and the
   AVR is an 8-bit core, we need to split the write into two halves.
   According to the datasheet the clock generator is updated when the low
   byte is written. Thus the baud rate register should be written to
   high-byte first.

   Next is framing, most things these days default to "8N1", and that's
   what we will do here. Section 19.10.4 (UCSR0C) describes the framing
   controls. To use asynchronous mode, set the top 2 bits to 00. To
   inhibit parity generation and checking, set the next 2 bits to 00. To
   use 1 stop bit, set the next bit to 0. To use 8 data bits, set the next
   2 bits to 11 (decimal 3). In async mode, the bottom bit should be 0.
   Altogether, UCSR0C = 0x06; or UCSR0c = (3 << UCSZ00); will program the
   register correctly.

   Once the serial port is initialized, you can write to it by writing to
   UDR0. You should check that the port is actually ready to transmit; the
   usual way to do this seems to be a busy wait that tests if the USART
   Data Register is Empty - (UCSR0A & (1 << UDRE0)).

   Try write a program that prints a fixed string to the serial port. One
   way to do this is [22]lesson4.c
     __________________________________________________________________

Lesson 5: Serial Input

   Building on lesson 4, we can now handle serial input. The companion
   function - receive - busy waits on USART Receive Complete, and then
   returns the Data Register (UCSR0A & (1 << RXC0)). The busy wait is not
   good for applications that shouldn't block on serial IO, but for now
   it's good enough.

   Reusing your code from lesson 4, write a program to read a character
   from the serial port and echo the next one back. If you type 'A' it
   should echo 'B'. One way to do this is [23]lesson5.c.
     __________________________________________________________________

Lesson 6: Putting it together: printing button presses and controlling LEDs

   I haven't written a sample solution to this yet, but at this point you
   should know everything you need to read a switch or the serial port and
   use this to control an LED (and print its status to the serial port).
   When I do finish this example, you can see [24]lesson6.c
     __________________________________________________________________

Lesson 7: Analog Output

   Digital outputs have a hard time generating outputs other than logic
   low and high - probably +0 and +Vcc in Arduino applications. It's easy
   enough to set an LED on a digital pin to 0% or 100% brightness, and we
   know it's possible to make LEDs glow at less than full brightness, but
   how? The answer is PWM - Pulse Width Modulation.

   PWM works by exploiting the fact that we're using rapidly changing
   signals to manipulate physical objects that can't react as quickly. By
   sending a series of fast, short pulses we can simulate their average
   effect. The math is simple: 100ms@100%+900ms@0% has the same average
   power over 1 second as 1000ms@10%. In other words, a low-pass filter.
   With that in mind, let's set up the the AVR to do PWM output.

   Try writing a program to slowly ramp the brightness of the LED. Or
   maybe have it fade in and out. [25]lesson7.c is one way to do this.
   Alas, this solution sucks - it's basically a big nasty busy wait that
   prevents you from getting any real work done while generating the PWM
   waveform.

   As we learn about hardware PWM, application Note 130 will be your
   friend. Though it's written for the AT90S8535 and doesn't use gcc, the
   general principles are still useful. First off, you need a timer to
   generate pulses. This is created from a counter, a periodic increment
   and a compare register.

   For this example, TCNT0 is the counter, the system clock is the
   periodic increment (CS = 1), and OCR0A as the compare register. Section
   13.3.3 of the ATmega168 datasheet says that PORTD.6 is the output pin
   for OC0A. First, the control register TCCR0A should be set for Fast PWM
   mode. Then the compare mode (inverting/non-inverting) is also set in
   the control register. At this point, it would be good to initialize the
   timer and comparator values because the timer isn't ticking until the
   clock source is set (control register TCCR0B). It is now safe to set
   the output bits on the output port. One way to do hardware PWM is
   [26]lesson7b.c.
     __________________________________________________________________

Lesson 8: avr-libc goodies

   Now that you've written a few programs, I imagine you're getting tired
   of typing in certain constructs (and your eyeballs are probably
   bleeding from the aggregated ugliness). It's time to introduce a few
   macros to make things simpler:

  By hand                                Macro
  while ( (UCSR0A & (1 << UDRE0)) ){ ; } loop_until_bit_is_clear(UCSR0A,
                                         UDRE0);
  while (!(PORTD & 0x4) ){ ; }           loop_until_bit_is_set(PORTD, _BV(PD2));
  PORTD = 0x4;                           PORTD = _BV(PD2);
  #define BAUD 9600
  UBRR0H = (((F_CPU/BAUD/16)-1) >> 8);
  UBRR0L = ((F_CPU/BAUD/16)-1);
  UCSR0C = (3 << UCSZ00);
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
                                         #define BAUD 9600
                                         #include <util/setbaud.h>
                                         UBRR0H = UBRRH_VALUE;
                                         UBRR0L = UBRRL_VALUE;
                                         UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
                                         UCSR0B = _BV(RXEN0) | _BV(TXEN0);

   I'm rather fond of stdio and formatted IO. I've been spoiled by big
   machines with multiple multi-GHz cpus, a few GB of memory and a few TB
   of storage. Programming an embedded device makes me realize how
   wonderful printf("sensor %d value %f\n", n, values[n]) is. Avr-libc has
   some glue to simulate stdio. Here's an example of how to use this:
   [27]lesson8.c. The manual warns that printf and scanf are quite
   resource-intensive. This is not even a little bit of an exaggeration.
   Have a look at the annotated assembly files and be afraid. Be very
   afraid. In my example implementation, I also use the program space
   macros to avoid keeping constant strings in RAM when they could simply
   be used straight from flash.
     __________________________________________________________________

Lesson 9: Analog Input

   In this example, I will be using a thermistor to provide analog input.
   Actually setting up the ADC is pretty straightforward; Because I'm
   using a thermistor as my analog device, I added some "fluff" to help
   visualize the input, but it's not strictly necessary.

   There are a few little things to do for a simple, polled analog sample.
   Ensure that the pin you're reading has pull-ups disabled, since these
   interfere with the sampling. Writing 0 to the pin register (PORTC = 0)
   or setting it to output mode (DDRC = 0) Set the ADC clock correctly -
   it needs to be clocked at 50kHz-200kHz. The Arduino's 16MHz clock
   divided by 128 is a very comfortable 165kHz. Set the reference voltage
   correctly. This determines the upper limit for measurement; using Vcc
   is probably the best bet. USB power will probably be very stable.
   Finally, set the ADC enable bit and force one conversion - the first
   conversion takes extra time due to some internal setup that is
   required. Here is [28]lesson9.c.
     __________________________________________________________________

Lesson 10: A blinking LED - Interrupts and timers

   Like the serial ports, it took a bit of studying of the datasheets to
   understand how to make timers work. A large part of this was due to one
   incorrectly set bit - it compiled and didn't do what I thought it
   should do. As it turns out, a simple blinky light was easier to figure
   out than PWM.

   In this example, I'm using the TIMER0_OVERFLOW interrupt to decide what
   to do when the timer overflows - whether to toggle the LED or not.
   Interrupt handlers must be written carefully if they are run often
   since they can cut into more useful processing time. Setting up an
   interrupt handler has some noticeable overhead - 14 instructions for
   the prologue and epilogue - before the main body of the handler is
   executed. Furthermore, a since line of C might be translated into 10
   instruction it is very easy to write an interrupt handler that takes a
   long time to run. Depending on the length of the handler and the
   frequency of interrupts, you may run out of stack space and crash. The
   example compiler invokation below includes flags to emit
   source-annotated-assembly language.

   Using the 8-bit timer0 is fairly straightforward. There are four things
   to be done:
    1. select the timer's clock source
    2. allow the timer to generate interrupts
    3. reset the timer
    4. enable interrupt processing

   Each of these items takes a just single line of code. First, the clock
   source must be configured. The timer system can accept external inputs
   or use the system clock - optionally divided down to a slower rate. The
   Clock Select (CS00-CS02) bits in the timer/counter control register B
   (TCCR0B) are used. Assuming a 16MHz clock, a prescale of 1024 will
   cause 15625 increment operations per second. Since the counter can hold
   256 unique values, it will overflow at 61Hz. If a higher interrupt rate
   is needed, the prescaler can be set lower, or a smaller Output Compare
   (OCR0A) can be used. The timer/counter interrupt mask register defaults
   to 0 - no interrupts will be generated by the timer/counter system. To
   enable timer overflow interrupts, set the TOIE0 bit of TIMSK0. Timers
   can be set and reset by storing into them, in this case TCNT0 = 0.
   Finally, the sei() function enables interrupt processing.

   One way to do this is [29]lesson10.c. My version can blink either the
   onboard LED on PORTB.5 or a set of LEDs on PORTB.1 - PORTB.3.

Lesson 11: Persistent Storage

   EEPROM provides developers with the ability to persistently store data
   on chip, inside the running program. It requires no peripherals, and
   doesn't place the program code at risk like writing the flash. Of
   course, EEPROM is much slower than RAM access. You should be prepared
   to deal with power failures while writing to EEPROM.

   EEPROM use is easy. Because EEPROM is slower than memory, you first
   need to check that the EEPROM is ready to accept operations. There is a
   function - eeprom_is_ready() - to do this, and this is the core of the
   eeprom_busy_wait() function. Once the EEPROM is ready, it can be
   written with eeprom_write_{byte,word,dword,block} and read back with
   eeprom_read_{byte,word,dword,block}. See [30]lesson11.c for an example.

   To safely use EEPROM storage in applications where power loss is a
   significant risk, consider storing a version counter and a validity
   flag in the structure. The structure should be marked invalid before
   any changes are made, and should only be marked valid once all other
   changes are made. Keep two (or more) copies of the structure and rotate
   through them; The version counter can be used to determine which is the
   most current copy. In the event power is lost during a write, older
   versions remain intact. You may want to have a look at "AVR101: High
   Endurance EEPROM Storage".

Lesson 12: Input Capture

   One of the useful features of the AVR is the input capture system. An
   application requiring low latency timestamping (a stopwatch or a pwm
   detector) could benefit from this. Input capture works by atomically
   copying the current value of the Timer/Counter Register into the Input
   Capture Register. The timer continues running and can optionally be
   reset to zero. Inside the input capture interrupt handler the user code
   can run without having to take heroic measures to minimize time spent
   reading the timer or resetting it - this is all done in hardware.

Lesson 13: Watchdog

   From time to time, it may be useful to have an interrupt or reset
   delivered independently of the main program code. This may be to wake
   the AVR from sleep mode or to reset when the program gets stuck in a
   loop; that's where the watchdog timer comes in. An onboard 128kHz
   oscillator is used to drive the watchdog with intervals from 16ms to
   8s. If the watchdog expires without being touched, the system can catch
   an interrupt, be reset or both. If you "kick the dog" before the time
   expires, no action is taken.

   While it can be slightly complicated to set up the watchdog by hand,
   avr-libc has some convenience functions. To enable the watchdog, call
   watchdog_enable() with the desired duration. Refreshing the watchdog is
   as simple as a call to watchdog_reset(). If the WDTON fuse is set the
   watchdog activates automatically, rather than requiring manual
   activation.

   A short example of using the watchdog timer to wake from a deep sleep
   mode is found in [31]lesson13.c.

Lesson 14: I2C/SPI peripherals

ATmega644P-specific

   Most of this document should be applicable to any AVR board. I wrote it
   using a Freeduino SB which has an ATmega168. I'm also running a
   Sanguino which is based on the ATmega644P. This section covers some of
   the differences when running on this hardware.

   I recently ported an application I built on the '168 to the '644P. The
   only changes I needed to make were to the pins with the LEDs and the
   input capture pin. Most Arduino compatible boards seem to use PORTD.5
   for the on-board blinkenlicht - the Sanguino I'm working with uses
   PORTB.0. Input capture moves from PORTB.0 to PORTD.6. After some
   thought I changed all of the program's references to specific ports and
   pins to preprocessor macros allowing me to use a different set of pins
   depending on the board I'm running.

ATtiny85-specific

   Most of this document should be applicable to any AVR board. I wrote it
   using a Freeduino SB which has an ATmega168. I'm also running some
   ATtiny85 parts. This section covers some of the differences when
   running on this hardware.

   While porting my firefly simulator to the tiny85, I found the smaller
   peripheral mix got to be very annoying. In particular, TIMER2 which can
   wake the AVR from sleep does not exist, nor does the
   SLEEP_MODE_PWR_SAVE. Power down mode does exist, but it may disable too
   many other parts of the processor. In this case, I was able to use the
   watchdog timer to wake from SLEEP_MODE_PWR_DOWN, but the big lesson was
   to carefully read the datasheets before committing to a particular
   part.

AT90USB162-specific

   Most of this document should be applicable to any AVR board. I wrote it
   using a Freeduino SB which has an ATmega168. I'm also running a Teensy
   which is based on the AT90USB162. This section covers some of the
   differences when running on this hardware.

References

     [1] [32]pin_map.html

     [2] http://www.atmel.com/dyn/resources/prod_documents/avr_3_04.pdf

     [3] http://www.atmel.com/dyn/resources/prod_documents/doc2545.pdf

     http://avrbasiccode.wikispaces.com/Atmega168

     http://javiervalcarce.es/wiki/Program_Arduino_with_AVR-GCC

     http://www.ladyada.net/learn/Arduino/

     http://ccrma.stanford.edu/courses/250a/docs/avrlib/html/

     http://www.nongnu.org/avr-libc/user-manual/

     http://www.engbedded.com/cgi-bin/fc.cgi

     http://piconomic.berlios.de/

     http://www.sanguino.cc/

     http://www.pjrc.com/teensy/

     ... and the rest of atmel's ginormous tech library

Appendix A: compiling and loading your code

avr-gcc -g -mmcu=atmega168 -c example.c -Wa,-alh,-L -o example.o > example.asm
avr-gcc -g -mmcu=atmega168 -Wl,-Map,example.map -o example.elf example.o
avr-objdump -h -S example.elf > example.lst
avr-objcopy -j .text -j .data -O ihex example.elf example.hex
avr-size example.elf
avrdude -b19200 -P /dev/cuaU0 -c avrisp -p m168 -U flash:w:example.hex

avr-gcc -g -mmcu=atmega644p -c blinkprint.c -Wa,-alh,-L -o blinkprint.o > blinkp
rint.asm
avr-gcc -g -mmcu=atmega644p -Wl,-Map,blinkprint.map -o blinkprint.elf blinkprint
.o
avr-objdump -h -S blinkprint.elf > blinkprint.lst
avr-objcopy -j .text -j .data -O ihex blinkprint.elf blinkprint.hex
avr-size blinkprint.elf
avrdude -c usbtiny -p atmega644p -U flash:w:blinkprint.hex

   NB: older versions of GCC may not support newer mcu types like
   -mmcu=atmega168 or -mmcu=atmega644p. For some of these examples you may
   be able to use -mmcu=avr5, but things like the serial port won't work
   correctly or at all without the correct mcu flag. Arduino-0011 uses GCC
   4.0.4, that should be considered the minimum required version.

   Copyright ? 2008,2009 Chris Kuethe <chris.kuethe@gmail.com>
   $CSK: index.html,v 1.27 2010/01/09 21:54:37 ckuethe Exp $

References

   1. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#blinky1
   2. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#blinky2
   3. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#digital-in
   4. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#usart-out
   5. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#usart-in
   6. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#lesson06
   7. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#pwm-out
   8. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#avr-libc-goodies
   9. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#adc-in
  10. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#blinky3
  11. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#eeprom
  12. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#input-capture
  13. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#watchdog
  14. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#i2c-spi
  15. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#atmega644p-only
  16. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#attiny85-only
  17. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/index.html#at90usb162-only
  18. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/pin_map.html
  19. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson1.c
  20. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson2.c
  21. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson3.c
  22. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson4.c
  23. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson5.c
  24. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson6.c
  25. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson7.c
  26. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson7b.c
  27. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson8.c
  28. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson9.c
  29. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson10.c
  30. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson11.c
  31. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/lesson13.c
  32. file:///export/backups/kaizo.org.old/htdocs/old/mainframe.cx_tutorial/pin_map.html
