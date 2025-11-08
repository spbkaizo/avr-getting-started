/* Lesson 12: Input Capture */
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

/*
 * Input Capture - measuring pulse width or frequency
 * Uses Timer1's Input Capture feature on ICP1 pin (PB0 on ATmega168)
 * This example measures the time between rising edges (frequency measurement)
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

volatile uint16_t capture_value = 0;
volatile uint16_t previous_capture = 0;
volatile uint16_t pulse_width = 0;
volatile uint8_t new_capture = 0;

ISR(TIMER1_CAPT_vect)
{
	/* Read the captured timer value */
	capture_value = ICR1;

	/* Calculate the time since last capture */
	pulse_width = capture_value - previous_capture;
	previous_capture = capture_value;

	new_capture = 1;
}

int main(void)
{
	uint32_t frequency;

	/* Set up serial output */
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	stdout = &O;

	/* Set ICP1 (PB0) as input */
	DDRB &= ~_BV(PB0);

	/* Configure Timer1 for Input Capture
	 * - Normal mode (count up to 0xFFFF)
	 * - No prescaler (CLK/1)
	 * - Capture on rising edge
	 */
	TCCR1A = 0;
	TCCR1B = _BV(CS10) | _BV(ICES1); /* No prescaler, rising edge */

	/* Enable Input Capture Interrupt */
	TIMSK1 = _BV(ICIE1);

	/* Enable global interrupts */
	sei();

	printf_P(PSTR("Input Capture Example\r\n"));
	printf_P(PSTR("Connect signal to PB0 (ICP1)\r\n\r\n"));

	while (1) {
		if (new_capture) {
			new_capture = 0;

			/* Calculate frequency from pulse width
			 * frequency = F_CPU / pulse_width
			 * Using no prescaler, each tick is 1/16MHz = 62.5ns
			 */
			if (pulse_width > 0) {
				frequency = F_CPU / pulse_width;
				printf_P(PSTR("Pulse width: %u ticks, Frequency: %lu Hz\r\n"),
					pulse_width, frequency);
			}
		}
	}

	return 0;
}
