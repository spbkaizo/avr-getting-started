/* Lesson 19: Interrupt-Driven UART (Non-Blocking Serial) */
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
#include <string.h>

/*
 * Interrupt-Driven UART
 * This demonstrates non-blocking serial communication using interrupts
 * Allows the CPU to do other work while serial data is transmitted/received
 */

#define TX_BUFFER_SIZE 128
#define RX_BUFFER_SIZE 128

/* Circular buffers for TX and RX */
volatile char tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_read_pos = 0;
volatile uint8_t tx_write_pos = 0;

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_read_pos = 0;
volatile uint8_t rx_write_pos = 0;

/* UART Data Register Empty Interrupt - ready to send next byte */
ISR(USART_UDRE_vect)
{
	if (tx_read_pos != tx_write_pos) {
		/* Send next byte from buffer */
		UDR0 = tx_buffer[tx_read_pos];
		tx_read_pos = (tx_read_pos + 1) % TX_BUFFER_SIZE;
	} else {
		/* Buffer empty, disable UDRE interrupt */
		UCSR0B &= ~_BV(UDRIE0);
	}
}

/* UART Receive Complete Interrupt */
ISR(USART_RX_vect)
{
	uint8_t next_write_pos = (rx_write_pos + 1) % RX_BUFFER_SIZE;

	/* Check if buffer would overflow */
	if (next_write_pos != rx_read_pos) {
		/* Store received byte */
		rx_buffer[rx_write_pos] = UDR0;
		rx_write_pos = next_write_pos;
	} else {
		/* Buffer full, discard byte (or handle error) */
		volatile uint8_t dummy = UDR0;
		(void)dummy; /* Prevent unused variable warning */
	}
}

void uart_init(void)
{
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8N1 */

	/* Enable RX, TX, and RX Complete Interrupt */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
}

void uart_putchar(char c)
{
	uint8_t next_write_pos = (tx_write_pos + 1) % TX_BUFFER_SIZE;

	/* Wait if buffer is full */
	while (next_write_pos == tx_read_pos)
		;

	/* Add character to buffer */
	tx_buffer[tx_write_pos] = c;
	tx_write_pos = next_write_pos;

	/* Enable UDRE interrupt to start transmission */
	UCSR0B |= _BV(UDRIE0);
}

void uart_puts(const char *str)
{
	while (*str) {
		if (*str == '\n') {
			uart_putchar('\r');
		}
		uart_putchar(*str++);
	}
}

void uart_puts_P(PGM_P str)
{
	char c;
	while ((c = pgm_read_byte(str++))) {
		if (c == '\n') {
			uart_putchar('\r');
		}
		uart_putchar(c);
	}
}

uint8_t uart_available(void)
{
	return (rx_write_pos - rx_read_pos + RX_BUFFER_SIZE) % RX_BUFFER_SIZE;
}

char uart_getchar(void)
{
	/* Wait if buffer is empty */
	while (rx_read_pos == rx_write_pos)
		;

	char c = rx_buffer[rx_read_pos];
	rx_read_pos = (rx_read_pos + 1) % RX_BUFFER_SIZE;

	return c;
}

uint8_t uart_getline(char *buf, uint8_t maxlen)
{
	uint8_t i = 0;
	char c;

	while (i < maxlen - 1) {
		c = uart_getchar();

		/* Echo character */
		uart_putchar(c);

		if (c == '\r' || c == '\n') {
			uart_putchar('\n');
			buf[i] = '\0';
			return i;
		} else if (c == '\b' || c == 127) { /* Backspace */
			if (i > 0) {
				i--;
				uart_puts_P(PSTR(" \b")); /* Erase character */
			}
		} else {
			buf[i++] = c;
		}
	}

	buf[i] = '\0';
	return i;
}

int main(void)
{
	char input[64];
	uint16_t counter = 0;

	/* Initialize UART */
	uart_init();

	/* Enable global interrupts */
	sei();

	uart_puts_P(PSTR("Interrupt-Driven UART Example\n"));
	uart_puts_P(PSTR("Type something and press Enter\n\n"));

	while (1) {
		/* CPU can do other work here while serial operates in background */

		/* Check if data is available */
		if (uart_available() > 0) {
			uart_puts_P(PSTR("> "));
			uart_getline(input, sizeof(input));

			uart_puts_P(PSTR("You typed: "));
			uart_puts(input);
			uart_putchar('\n');

			/* Echo count */
			char buf[32];
			sprintf(buf, "Message count: %u\n\n", ++counter);
			uart_puts(buf);
		}

		/* Do other tasks here - serial runs in background! */
	}

	return 0;
}
