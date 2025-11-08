/* Lesson 20: Complete Application - Environmental Monitor */
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
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

/*
 * Complete Application: Environmental Monitor
 *
 * This brings together many concepts from previous lessons:
 * - ADC for reading temperature sensor
 * - Timers for periodic sampling
 * - EEPROM for storing min/max values
 * - UART for reporting data
 * - PWM for visual feedback (LED brightness)
 * - Interrupts for responsive operation
 *
 * Features:
 * - Reads temperature from ADC every 5 seconds
 * - Stores min/max values in EEPROM
 * - Reports via serial
 * - LED brightness indicates temperature
 * - Button to reset min/max values
 */

/* EEPROM storage layout */
#define EEPROM_MAGIC    ((uint16_t *)0)
#define EEPROM_MIN_TEMP ((int16_t *)2)
#define EEPROM_MAX_TEMP ((int16_t *)4)
#define EEPROM_SAMPLES  ((uint32_t *)6)
#define MAGIC_VALUE     0xABCD

/* Global variables */
volatile uint8_t sample_flag = 0;
volatile uint8_t button_pressed = 0;
int16_t min_temp = 32767;
int16_t max_temp = -32768;
uint32_t sample_count = 0;

/* UART functions */
static int uart_putchar(char c, FILE *stream)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Timer1 Compare Match - 5 second sampling interval */
ISR(TIMER1_COMPA_vect)
{
	sample_flag = 1;
}

/* External Interrupt - Button press */
ISR(INT0_vect)
{
	/* Simple debounce - set flag, handle in main loop */
	_delay_ms(50);
	if (!(PIND & _BV(PD2))) {
		button_pressed = 1;
	}
}

void system_init(void)
{
	/* UART at 9600 baud */
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	stdout = &uart_output;

	/* ADC setup */
	ADMUX = _BV(REFS0); /* AVCC reference, ADC0 */
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* Enable, prescaler 128 */

	/* Timer1 for 5-second interval (CTC mode) */
	OCR1A = 15625; /* With prescaler 1024: 16MHz/1024/15625 = ~1Hz */
	TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); /* CTC, prescaler 1024 */
	TIMSK1 = _BV(OCIE1A);

	/* Timer0 for PWM on LED (PD6) */
	DDRD |= _BV(PD6);
	TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); /* Fast PWM, non-inverting */
	TCCR0B = _BV(CS01); /* Prescaler 8 */
	OCR0A = 128; /* 50% initial brightness */

	/* External interrupt on PD2 (button) */
	DDRD &= ~_BV(PD2);
	PORTD |= _BV(PD2); /* Pull-up */
	EICRA |= _BV(ISC01); /* Falling edge */
	EIMSK |= _BV(INT0);

	/* Status LED on PB5 */
	DDRB |= _BV(PB5);
	PORTB |= _BV(PB5);
}

void load_eeprom_data(void)
{
	uint16_t magic = eeprom_read_word(EEPROM_MAGIC);

	if (magic == MAGIC_VALUE) {
		min_temp = eeprom_read_word((uint16_t *)EEPROM_MIN_TEMP);
		max_temp = eeprom_read_word((uint16_t *)EEPROM_MAX_TEMP);
		sample_count = eeprom_read_dword(EEPROM_SAMPLES);
		printf_P(PSTR("Loaded from EEPROM: min=%d max=%d samples=%lu\n"),
			min_temp, max_temp, sample_count);
	} else {
		printf_P(PSTR("EEPROM not initialized\n"));
		eeprom_write_word(EEPROM_MAGIC, MAGIC_VALUE);
	}
}

void save_eeprom_data(void)
{
	eeprom_update_word((uint16_t *)EEPROM_MIN_TEMP, min_temp);
	eeprom_update_word((uint16_t *)EEPROM_MAX_TEMP, max_temp);
	eeprom_update_dword(EEPROM_SAMPLES, sample_count);
}

int16_t read_temperature(void)
{
	/* Start conversion */
	ADCSRA |= _BV(ADSC);

	/* Wait for completion */
	loop_until_bit_is_clear(ADCSRA, ADSC);

	/* Read result */
	uint16_t adc = ADC;

	/* Convert to temperature (example formula for LM35 in 10ths of degree C)
	 * Adjust this formula based on your actual sensor
	 * LM35: 10mV per degree C, with 5V reference and 10-bit ADC:
	 * temp = (adc * 5000) / 1024 / 10
	 */
	int16_t temp = (adc * 500L) / 1024;

	return temp;
}

void update_led_brightness(int16_t temp)
{
	/* Map temperature to LED brightness (example: 0-50°C -> 0-255) */
	int16_t brightness = (temp * 255L) / 50;
	if (brightness < 0) brightness = 0;
	if (brightness > 255) brightness = 255;

	OCR0A = brightness;
}

void reset_min_max(void)
{
	min_temp = 32767;
	max_temp = -32768;
	sample_count = 0;
	save_eeprom_data();
	printf_P(PSTR("Min/Max reset!\n"));
}

int main(void)
{
	int16_t temperature;
	uint8_t countdown = 5;

	system_init();
	sei(); /* Enable interrupts */

	printf_P(PSTR("\n=== Environmental Monitor ===\n"));
	printf_P(PSTR("Temperature sensor on ADC0\n"));
	printf_P(PSTR("Button on PD2 to reset min/max\n"));
	printf_P(PSTR("LED brightness indicates temperature\n\n"));

	load_eeprom_data();

	/* Take initial reading */
	temperature = read_temperature();
	printf_P(PSTR("Current: %d.%d°C\n"), temperature/10, temperature%10);

	while (1) {
		/* Handle button press */
		if (button_pressed) {
			button_pressed = 0;
			reset_min_max();
		}

		/* Handle periodic sampling */
		if (sample_flag) {
			sample_flag = 0;

			/* Read temperature */
			temperature = read_temperature();
			sample_count++;

			/* Update min/max */
			if (temperature < min_temp) {
				min_temp = temperature;
			}
			if (temperature > max_temp) {
				max_temp = temperature;
			}

			/* Update LED brightness */
			update_led_brightness(temperature);

			/* Report */
			printf_P(PSTR("T=%d.%d°C  Min=%d.%d  Max=%d.%d  Samples=%lu\n"),
				temperature/10, temperature%10,
				min_temp/10, min_temp%10,
				max_temp/10, max_temp%10,
				sample_count);

			/* Blink status LED */
			PORTB ^= _BV(PB5);

			/* Save to EEPROM every 10 samples */
			if (sample_count % 10 == 0) {
				save_eeprom_data();
			}

			countdown = 5;
		}

		/* Countdown display every second */
		if (TCNT1 > 15000 && countdown > 0) {
			printf_P(PSTR("."));
			countdown--;
		}
	}

	return 0;
}
