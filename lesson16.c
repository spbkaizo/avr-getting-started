/* Lesson 16: Pin Change and External Interrupts */
/*
 * Copyright (c) 2025 Chris Kuethe <chris.kuethe@gmail.com>
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

#define F_CPU	16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/*
 * Advanced Interrupts Example
 * Demonstrates External Interrupts (INT0, INT1) and Pin Change Interrupts (PCINT)
 *
 * External Interrupts:
 * INT0: PD2 (Pin 2) - can trigger on rising, falling, or any edge
 * INT1: PD3 (Pin 3) - can trigger on rising, falling, or any edge
 *
 * Pin Change Interrupts: Any pin can trigger on state change
 * This example uses PCINT on PORTB
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

volatile uint16_t int0_count = 0;
volatile uint16_t int1_count = 0;
volatile uint16_t pcint_count = 0;
volatile uint8_t last_portb = 0;

/* External Interrupt 0 - Rising Edge */
ISR(INT0_vect)
{
	int0_count++;
	/* Toggle LED on PB5 */
	PORTB ^= _BV(PB5);
}

/* External Interrupt 1 - Falling Edge */
ISR(INT1_vect)
{
	int1_count++;
	/* Toggle LED on PB4 */
	PORTB ^= _BV(PB4);
}

/* Pin Change Interrupt on PORTB */
ISR(PCINT0_vect)
{
	uint8_t current_portb = PINB;
	uint8_t changed_bits = current_portb ^ last_portb;

	pcint_count++;

	/* You can check which pin(s) changed */
	if (changed_bits & _BV(PB0)) {
		printf_P(PSTR("PB0 changed\r\n"));
	}
	if (changed_bits & _BV(PB1)) {
		printf_P(PSTR("PB1 changed\r\n"));
	}

	last_portb = current_portb;
}

void external_interrupts_init(void)
{
	/* Set INT0 and INT1 pins as inputs with pull-ups */
	DDRD &= ~(_BV(PD2) | _BV(PD3));
	PORTD |= _BV(PD2) | _BV(PD3);

	/* Configure INT0 for rising edge trigger */
	EICRA |= _BV(ISC01) | _BV(ISC00);

	/* Configure INT1 for falling edge trigger */
	EICRA |= _BV(ISC11);
	EICRA &= ~_BV(ISC10);

	/* Enable INT0 and INT1 */
	EIMSK |= _BV(INT0) | _BV(INT1);
}

void pin_change_interrupts_init(void)
{
	/* Set PB0 and PB1 as inputs with pull-ups */
	DDRB &= ~(_BV(PB0) | _BV(PB1));
	PORTB |= _BV(PB0) | _BV(PB1);

	/* Enable Pin Change Interrupt on PORTB */
	PCICR |= _BV(PCIE0);

	/* Enable PCINT0 and PCINT1 */
	PCMSK0 |= _BV(PCINT0) | _BV(PCINT1);

	/* Store initial state */
	last_portb = PINB;
}

int main(void)
{
	/* Set up serial output */
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	stdout = &O;

	/* Set PB4 and PB5 as outputs for LEDs */
	DDRB |= _BV(PB4) | _BV(PB5);

	printf_P(PSTR("Advanced Interrupts Example\r\n"));
	printf_P(PSTR("INT0 (PD2): Rising edge\r\n"));
	printf_P(PSTR("INT1 (PD3): Falling edge\r\n"));
	printf_P(PSTR("PCINT (PB0,PB1): Any change\r\n\r\n"));

	/* Initialize interrupts */
	external_interrupts_init();
	pin_change_interrupts_init();

	/* Enable global interrupts */
	sei();

	while (1) {
		/* Print interrupt counts every 2 seconds */
		_delay_ms(2000);
		printf_P(PSTR("INT0: %u, INT1: %u, PCINT: %u\r\n"),
			int0_count, int1_count, pcint_count);
	}

	return 0;
}
