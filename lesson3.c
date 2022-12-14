/* $CSK: lesson3.c,v 1.3 2008/09/29 06:17:10 ckuethe Exp $ */
/*
 * Copyright (c) 2008 Chris Kuethe <chris.kuethe@gmail.com>
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

#include <avr/io.h>
#include <util/delay.h>

/*
 * Assumptions:
 * 	- LED connected to PORTB.2
 * 	- Switch connected to PORTD.2
 */

int main (void)
{
	/* set PORTB for output*/
	DDRB = 0xFF;
	/* set PORTD for input*/
	DDRD &= 0xFB;
	PORTD |= 0x04;

	while (1) {
		if (PIND & 0x04)
			PORTB &= ~0x20;
		else
			PORTB |= 0x20;
	}
	return 0;
}
