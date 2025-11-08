/* Lesson 15: SPI Communication */
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
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

/*
 * SPI Communication Example
 * This demonstrates SPI master mode
 *
 * ATmega168 SPI Pins:
 * MOSI: PB3 (Pin 11)
 * MISO: PB4 (Pin 12)
 * SCK:  PB5 (Pin 13)
 * SS:   PB2 (Pin 10) - Slave Select (user controlled)
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

void spi_init(void)
{
	/* Set MOSI, SCK, and SS as outputs */
	DDRB |= _BV(PB3) | _BV(PB5) | _BV(PB2);

	/* Set MISO as input */
	DDRB &= ~_BV(PB4);

	/* Enable pull-up on MISO */
	PORTB |= _BV(PB4);

	/* Set SS high (inactive) */
	PORTB |= _BV(PB2);

	/* Enable SPI, Master mode, Clock = F_CPU/16
	 * SPR1=0, SPR0=1 gives F_CPU/16 (1MHz @ 16MHz)
	 * CPOL=0, CPHA=0 (SPI Mode 0)
	 */
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
}

uint8_t spi_transfer(uint8_t data)
{
	/* Load data into the buffer */
	SPDR = data;

	/* Wait for transmission complete */
	while (!(SPSR & _BV(SPIF)))
		;

	/* Return received data */
	return SPDR;
}

void spi_select(void)
{
	/* Pull SS low to select slave */
	PORTB &= ~_BV(PB2);
}

void spi_deselect(void)
{
	/* Pull SS high to deselect slave */
	PORTB |= _BV(PB2);
}

/* Example: Read/Write to a simple SPI device */
void spi_write_byte(uint8_t address, uint8_t data)
{
	spi_select();
	spi_transfer(0x02); /* Write command */
	spi_transfer(address);
	spi_transfer(data);
	spi_deselect();
}

uint8_t spi_read_byte(uint8_t address)
{
	uint8_t data;

	spi_select();
	spi_transfer(0x03); /* Read command */
	spi_transfer(address);
	data = spi_transfer(0x00); /* Clock out dummy byte to read */
	spi_deselect();

	return data;
}

int main(void)
{
	uint8_t counter = 0;
	uint8_t received;

	/* Set up serial output */
#define BAUD 9600
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	stdout = &O;

	printf_P(PSTR("SPI Communication Example\r\n"));

	/* Initialize SPI */
	spi_init();

	while (1) {
		/* Example: Loopback test - connect MOSI to MISO */
		printf_P(PSTR("Sending: 0x%02X\r\n"), counter);

		spi_select();
		received = spi_transfer(counter);
		spi_deselect();

		printf_P(PSTR("Received: 0x%02X\r\n\r\n"), received);

		counter++;
		_delay_ms(1000);
	}

	return 0;
}
