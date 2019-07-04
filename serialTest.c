/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "utils/pinFunc.h"
#include <stdlib.h>

#define POLLING
//#define INTERRUPT

#define BAUDRATE 2400
#define UBRR F_CPU / BAUDRATE / 16 - 1

volatile char received;

static void init_USART(unsigned int ubrr) {
	// set UBRR (USART baud rate register) aka set baud rate
	UBRR0H = (unsigned char) (ubrr >> 8);
	UBRR0L = (unsigned char) (ubrr);
	
	// enable receiver and transmitter
	// RXCIE0 = receiver interrupt
	// RXEN0 = receiver enable
	// TXEN0 = transmitter enable
#ifdef POLLING
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
#endif

#ifdef INTERRUPT
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
#endif
	
	// set frame - 8 bits, 1 stop bit, no parity
	UCSR0C = 3 << UCSZ00;
}

static void USART_transmit(char data) {
	// wait for empty transmit buffer
	while (!(UCSR0A & (_BV(UDRE0))));

	// put data into buffer, sends the data
	UDR0 = data;
	if (data == 'd') toggle_status_led2();
}

static char USART_receive(void) {
	// wait for data to be received
	while(!(UCSR0A & _BV(RXCIE0)));

	// get and return received data from buffer
	toggle_status_led1();
	return UDR0;
}

#ifdef INTERRUPT
ISR(USART_RX_vect) {
	toggle_status_led3();
	received = USART_receive();
	// a 'get' signal from the computer
	if ((int) received > 120) {
		USART_transmit((char) 100);
		toggle_status_led4();
	} else {
		USART_transmit(received);
		toggle_status_led5();
	}
}
#endif

int main(void) {
	
	init_leds();
	init_USART(UBRR);
	sei();

	while(1) {
#ifdef POLLING
		if (UCSR0A & _BV(RXC0)) {
			toggle_status_led3();
			received = USART_receive();
			// a 'get' signal from the computer
			if ((int) received > 120) {
				USART_transmit((char)100);
				toggle_status_led4();				
			} else {
				USART_transmit(received);
				toggle_status_led5();
			}
		}
#endif
	};
	return 0;
}
*/
