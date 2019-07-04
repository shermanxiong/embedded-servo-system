/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "utils/pinFunc.h"
#include <stdlib.h>
#include <limits.h>

#define POLLING
//#define INTERRUPT

#define BAUDRATE 2400
#define UBRR F_CPU / BAUDRATE / 16 - 1

#define TICKS_PER_REV 96

#define AVG 8

/ *
	Current LED indicators
	LED 1: Receive command from computer
	LED 2: Transmit data to computer
	LED 3: Encoder interrupt
	LED 4: Received 'GET' command from computer
	LED 5: Received 'SET' command from computer
* /

static unsigned int y = 0;

static volatile int y_avg[AVG];
static volatile int y_counter;

static volatile int ticks[AVG];
static volatile int tick_counter = 0;

static volatile int counter = 0;

static volatile char received;

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

static void init_pci(void) {
	// When PCIE1 bit set = PCINT14:8 interrupts enabled
	PCICR |= _BV(PCIE1);
	// Enable PCINT8 and PCINT9 pins (Encoder pins B and A respectively)
	PCMSK1 |= _BV(PCINT8) | _BV(PCINT9);
}

void init_pwm(void) {
	DDRD |= _BV(PD3);
	TCCR2A |= _BV(COM2B1) | _BV(COM2B0) | _BV(WGM21) | _BV(WGM20);
	TCCR2B |= _BV(CS21);
}

void init_encoder(void) {
	// set PC0 and PC1 to inputs (Encoder pins)
	PORTC |= _BV(PC0) | _BV(PC1);
}

void init_timers(void) {
	// set clock pre-scaler = 256
	TCCR1B = _BV(CS12);
}

static void set_duty_cycle(int val) {
	OCR2B = 255 - val;
}

static void USART_transmit(char data) {
	// wait for empty transmit buffer	
	while (!(UCSR0A & (_BV(UDRE0))));

	// put data into buffer, sends the data
	UDR0 = data;
	toggle_status_led2();
}

static char USART_receive(void) {
	// wait for data to be received
	while(!(UCSR0A & _BV(RXCIE0)));

	// get and return received data from buffer
	toggle_status_led1();
	return UDR0;
}

static void transmit(int val) {
	char snum[8];
	// convert integer to character array (ie. string)
	itoa(val, snum, 10);
	for (int i = 0; i < 8; i++) {
		USART_transmit(snum[i]);
	}
	USART_transmit('\n');
	USART_transmit('\r');
}

static int average_of_8_ticks() {
	int sum = 0;
	for(int i = 0; i < AVG; i++) {
		sum += ticks[i];
	}
	
	return (sum >> 3); // divide by 8 by bit-shifting
}

static int average_of_8_rpms() {
	int sum = 0;
	for(int i = 0; i < AVG; i++) {
		sum += y_avg[i];
	}
	
	return (sum >> 3); // divide by 8 by bit-shifting
}
static int calculate_rpm(void) {
	// rpm = 60 seconds / ( number of ticks * (prescalar/F_CPU) * ticks per revolution )
	// 60 seconds * F_CPU [1mil] / interrupts per rev [96] / prescalar[256] = 2441
	// double rpm = 2441 / average_of_8_ticks(ticks);
	// return (int) rpm;
	return 2441 / average_of_8_ticks(ticks);
}

int main(void) {
	init_USART(UBRR);
	init_leds();
	init_pwm();
	init_pci();
	init_timers();
	sei();
	
	set_duty_cycle(255);
	while(1) {
		if (TIFR2 & _BV(TOV2)) {
			ticks[tick_counter] = INT_MAX;
			if(tick_counter == 7) {
				tick_counter = 0;
				} else {
				tick_counter++;
			}
		}
		// continuously calculate rpm ever 10 PWM cycles
		
		while(!(TIFR2 & _BV(TOV2)));
		if (counter < 10) {
			counter++;
		} else {
			/ *y_avg[y_counter] = calculate_rpm();
			if (y_counter == 7) {
				y_counter = 0;
			} else {
				y_counter++;
			}* /
			y = calculate_rpm();
			counter = 0;
		}
		
		if (UCSR0A & _BV(RXC0)) {
			received = USART_receive();
			// 'GET' command received
			if ((int) received > 120) {
				//USART_transmit((char) y);
				//transmit(average_of_8_ticks());
				transmit(y);
				toggle_status_led4();
			} else { // 'SET' command received
				y = (int) received;
				USART_transmit(y);
				toggle_status_led5();
			}
		}
	}
	return 0;
}

ISR(PCINT1_vect) {
	toggle_status_led3();
	// store tick values to keep past 8 values
	if (TCNT1 > 10) {
		ticks[tick_counter] = TCNT1;
		if(tick_counter == 7) {
			tick_counter = 0;
		} else {
			tick_counter++;
		}
	}
	TCNT1 = 0;
}*/