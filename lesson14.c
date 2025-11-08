/* Lesson 14: I2C (TWI) Communication */
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
 * I2C/TWI (Two Wire Interface) Communication
 * This example demonstrates basic I2C operations
 * SDA: PC4 (Analog 4)
 * SCL: PC5 (Analog 5)
 *
 * Example: Reading from a DS1307 RTC or similar I2C device
 */

static int cput(char, FILE *);
static FILE O = FDEV_SETUP_STREAM(cput, NULL, _FDEV_SETUP_WRITE);

static int cput(char c, FILE *f)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

#define I2C_START		0x08
#define I2C_REP_START	0x10
#define I2C_MT_SLA_ACK	0x18
#define I2C_MT_DATA_ACK	0x28
#define I2C_MR_SLA_ACK	0x40
#define I2C_MR_DATA_ACK	0x50
#define I2C_MR_DATA_NACK 0x58

void i2c_init(void)
{
	/* Set SCL frequency to 100kHz
	 * SCL_freq = F_CPU / (16 + 2*TWBR*prescaler)
	 * For 100kHz with prescaler=1: TWBR = 72
	 */
	TWBR = 72;
	TWSR = 0; /* Prescaler = 1 */
	TWCR = _BV(TWEN); /* Enable TWI */
}

uint8_t i2c_start(void)
{
	/* Send START condition */
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

	/* Wait for TWINT flag */
	while (!(TWCR & _BV(TWINT)))
		;

	/* Return status */
	return TWSR & 0xF8;
}

void i2c_stop(void)
{
	/* Send STOP condition */
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

	/* Wait for STOP to complete */
	while (TWCR & _BV(TWSTO))
		;
}

uint8_t i2c_write(uint8_t data)
{
	/* Load data and start transmission */
	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN);

	/* Wait for transmission */
	while (!(TWCR & _BV(TWINT)))
		;

	/* Return status */
	return TWSR & 0xF8;
}

uint8_t i2c_read_ack(void)
{
	/* Enable ACK and start reception */
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);

	/* Wait for data */
	while (!(TWCR & _BV(TWINT)))
		;

	return TWDR;
}

uint8_t i2c_read_nack(void)
{
	/* Start reception without ACK */
	TWCR = _BV(TWINT) | _BV(TWEN);

	/* Wait for data */
	while (!(TWCR & _BV(TWINT)))
		;

	return TWDR;
}

/* Example: Scan I2C bus for devices */
void i2c_scan(void)
{
	uint8_t address, status;

	printf_P(PSTR("Scanning I2C bus...\r\n"));

	for (address = 1; address < 128; address++) {
		i2c_start();
		status = i2c_write((address << 1) | 0); /* Write mode */
		i2c_stop();

		if (status == I2C_MT_SLA_ACK) {
			printf_P(PSTR("Device found at address 0x%02X\r\n"), address);
		}
		_delay_ms(1);
	}

	printf_P(PSTR("Scan complete\r\n\r\n"));
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

	printf_P(PSTR("I2C/TWI Example\r\n"));

	/* Initialize I2C */
	i2c_init();

	/* Scan for devices */
	i2c_scan();

	while (1) {
		/* Example: continuous scanning every 5 seconds */
		_delay_ms(5000);
		i2c_scan();
	}

	return 0;
}
