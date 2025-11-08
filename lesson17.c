/* Lesson 17: Sleep Modes and Power Management */
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
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

/*
 * Sleep Modes and Power Management
 *
 * AVR Sleep Modes (from least to most power savings):
 * - IDLE: CPU stopped, peripherals running
 * - ADC_NOISE_REDUCTION: CPU and ADC noise reduced
 * - POWER_DOWN: Most systems off, wake via external interrupt or watchdog
 * - POWER_SAVE: Like power down but timer2 can run
 * - STANDBY: Like power down but oscillator running
 * - EXTENDED_STANDBY: Like power save but oscillator running
 *
 * This example uses IDLE and POWER_DOWN modes
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

volatile uint8_t wakeup_count = 0;

/* External interrupt to wake from sleep */
ISR(INT0_vect)
{
	wakeup_count++;
	/* Toggle LED */
	PORTB ^= _BV(PB5);
}

/* Timer interrupt for periodic wake from IDLE */
ISR(TIMER1_COMPA_vect)
{
	/* LED blink in idle mode */
	PORTB ^= _BV(PB4);
}

void setup_external_interrupt(void)
{
	/* Set INT0 (PD2) as input with pull-up */
	DDRD &= ~_BV(PD2);
	PORTD |= _BV(PD2);

	/* Configure INT0 for falling edge (button press) */
	EICRA |= _BV(ISC01);
	EICRA &= ~_BV(ISC00);

	/* Enable INT0 */
	EIMSK |= _BV(INT0);
}

void setup_timer(void)
{
	/* Configure Timer1 for 1Hz interrupt
	 * F_CPU / 1024 / 15625 = 1Hz
	 */
	OCR1A = 15625;
	TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); /* CTC mode, prescaler 1024 */
	TIMSK1 = _BV(OCIE1A); /* Enable compare match interrupt */
}

void enter_idle_mode(void)
{
	printf_P(PSTR("Entering IDLE mode...\r\n"));
	_delay_ms(100); /* Let serial finish */

	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	sleep_cpu();
	sleep_disable();

	printf_P(PSTR("Woke from IDLE\r\n"));
}

void enter_power_down_mode(void)
{
	printf_P(PSTR("Entering POWER DOWN mode...\r\n"));
	printf_P(PSTR("Press button on PD2 to wake\r\n"));
	_delay_ms(100); /* Let serial finish */

	/* Disable peripherals to save more power */
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();

	set_sleep_mode(SLEEP_MODE_POWER_DOWN);
	sleep_enable();
	sleep_cpu();
	sleep_disable();

	/* Re-enable peripherals */
	power_all_enable();

	printf_P(PSTR("Woke from POWER DOWN! Count: %u\r\n"), wakeup_count);
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

	/* Set LEDs as outputs */
	DDRB |= _BV(PB4) | _BV(PB5);

	printf_P(PSTR("Sleep Modes and Power Management\r\n\r\n"));

	/* Setup interrupts */
	setup_external_interrupt();
	setup_timer();

	/* Enable global interrupts */
	sei();

	/* Demonstrate IDLE mode (wakes every 1 second via timer) */
	printf_P(PSTR("Demonstrating IDLE mode for 5 seconds\r\n"));
	for (uint8_t i = 0; i < 5; i++) {
		enter_idle_mode();
	}

	/* Switch to power down mode (wakes only on button press) */
	while (1) {
		enter_power_down_mode();
		_delay_ms(1000); /* Stay awake briefly */
	}

	return 0;
}
