/* Lesson 18: Using Multiple Timers Together */
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
 * Using Multiple Timers Together
 * This example demonstrates coordinating all three timers:
 *
 * TIMER0 (8-bit): Fast PWM for LED brightness control
 * TIMER1 (16-bit): CTC mode for 1Hz heartbeat
 * TIMER2 (8-bit): Overflow for fast event timing (debouncing, etc)
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

volatile uint32_t seconds = 0;
volatile uint16_t milliseconds = 0;
volatile uint8_t pwm_direction = 1;

/* TIMER0 - Nothing to do, hardware PWM runs automatically */

/* TIMER1 Compare Match A - 1Hz heartbeat */
ISR(TIMER1_COMPA_vect)
{
	seconds++;

	/* Adjust PWM brightness every second */
	static uint8_t brightness = 0;

	if (pwm_direction) {
		brightness += 16;
		if (brightness >= 240) {
			pwm_direction = 0;
		}
	} else {
		brightness -= 16;
		if (brightness <= 16) {
			pwm_direction = 1;
		}
	}

	OCR0A = brightness; /* Update PWM duty cycle */
}

/* TIMER2 Overflow - Called ~1000 times per second */
ISR(TIMER2_OVF_vect)
{
	milliseconds++;
	if (milliseconds >= 1000) {
		milliseconds = 0;
	}
}

void timer0_init_pwm(void)
{
	/*
	 * TIMER0: Fast PWM on OC0A (PD6)
	 * This will fade an LED in and out
	 */

	/* Set OC0A (PD6) as output */
	DDRD |= _BV(PD6);

	/* Fast PWM mode, non-inverting */
	TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);

	/* Start timer with prescaler 64 */
	TCCR0B = _BV(CS01) | _BV(CS00);

	/* Set initial duty cycle to 50% */
	OCR0A = 128;
}

void timer1_init_heartbeat(void)
{
	/*
	 * TIMER1: CTC mode for 1Hz interrupt
	 * F_CPU / 1024 / 15625 = 1Hz
	 */

	/* CTC mode (Clear Timer on Compare) */
	TCCR1B = _BV(WGM12);

	/* Set compare value for 1Hz */
	OCR1A = 15625;

	/* Start timer with prescaler 1024 */
	TCCR1B |= _BV(CS12) | _BV(CS10);

	/* Enable compare match interrupt */
	TIMSK1 = _BV(OCIE1A);
}

void timer2_init_millisecond(void)
{
	/*
	 * TIMER2: Overflow interrupt for millisecond timing
	 * With prescaler 64: 16MHz / 64 / 250 = 1000Hz
	 * We'll use overflow mode and reset counter
	 */

	/* Normal mode */
	TCCR2A = 0;

	/* Prescaler 64 */
	TCCR2B = _BV(CS22);

	/* Set counter value for ~1ms overflow
	 * 16MHz / 64 = 250kHz
	 * 250kHz / 1000 = 250 ticks per millisecond
	 * So overflow at 256-250 = 6
	 */
	TCNT2 = 6;

	/* Enable overflow interrupt */
	TIMSK2 = _BV(TOIE2);
}

uint32_t millis(void)
{
	uint32_t m;
	uint8_t oldSREG = SREG;

	cli(); /* Disable interrupts */
	m = (seconds * 1000UL) + milliseconds;
	SREG = oldSREG; /* Restore interrupt state */

	return m;
}

int main(void)
{
	uint32_t last_print = 0;

	/* Set up serial output */
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	stdout = &O;

	printf_P(PSTR("Multiple Timers Example\r\n"));
	printf_P(PSTR("TIMER0: PWM on PD6 (pin 6)\r\n"));
	printf_P(PSTR("TIMER1: 1Hz heartbeat\r\n"));
	printf_P(PSTR("TIMER2: Millisecond counter\r\n\r\n"));

	/* Initialize all three timers */
	timer0_init_pwm();
	timer1_init_heartbeat();
	timer2_init_millisecond();

	/* Enable global interrupts */
	sei();

	while (1) {
		uint32_t current_millis = millis();

		/* Print status every 500ms */
		if (current_millis - last_print >= 500) {
			last_print = current_millis;
			printf_P(PSTR("Time: %lu.%03u s, PWM: %u\r\n"),
				seconds, (uint16_t)milliseconds, OCR0A);
		}
	}

	return 0;
}
