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

#define AVG 8

/*
	Current LED indicators
	LED 1: Receive command from computer
	LED 2: Transmit data to computer
	LED 3: Encoder interrupt
	LED 4: Received 'GET' command from computer
	LED 5: Received 'SET' command from computer
*/

static volatile int u;
static volatile int y;
static volatile double I = 0.0;

static volatile int v;
static volatile int e;

static const int umin = 0;
static const int umax = 255;

static volatile unsigned int y_ref = 0;

static unsigned int K = 1;
static unsigned int Ti = 1;
static unsigned int Tr = 1;
static const unsigned int beta = 1;

static int ticks[AVG];
static int tick_counter = 0;

static int counter = 0;

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

// send single char data through USART
static void USART_transmit(unsigned char data) {
	// wait for empty transmit buffer	
	while (!(UCSR0A & (_BV(UDRE0))));

	// put data into buffer, sends the data
	UDR0 = data;
	toggle_status_led2();
}


// receive single char data through USART
static char USART_receive(void) {
	// wait for data to be received
	while(!(UCSR0A & _BV(RXCIE0)));

	// get and return received data from buffer
	toggle_status_led1();
	return UDR0;
}

// used to print values in Hyperterminal
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
	
	return (sum >> 3); // divide by 8 by bit-shifting 3 to the right (ie. divide by 2^3)
}

// returns 0 if RPM less than 5 because it's below operational range for the project
static int calculate_rpm(void) {
	// rpm = 60 seconds / ( number of ticks * (prescalar/F_CPU) * ticks per revolution )
	// 60 seconds * F_CPU [1mil] / interrupts per rev [96] / prescalar[256] = 2441
	int rpm = 2441 / average_of_8_ticks(ticks);
	return (rpm < 5) ? 0 : rpm;
}

static void set_duty_cycle(int val) {
	OCR2B = val;
}

static int sat(int val, int max, int min) {
	if (val > max) {
		return max;
	}
	if (val < min) {
		return min;
	}
	return val;
}

static void PI_control(void) {
	y = calculate_rpm();
	// calculate output
	e = y_ref - y;
	v = K * (beta * y_ref - y) + I;
	u = sat(v, umax, umin);
	set_duty_cycle(u);
	
	// update states
	I = I + ((K / (double) Ti) * e + (u - v) / Tr);

	// print values onto hyperterminal - sometimes gives garbage if you print all at once
	// best to print one variable only at a time
	//transmit(y);
	//transmit(e);
	//transmit(v);
	//transmit(u);
	//transmit((int) I);
	toggle_error_led();
}

int main(void) {
	init_USART(UBRR);
	init_leds();
	init_pwm();
	init_pci();
	init_timers();
	sei();

	int counter_limit;
	
	// wait for a cmd to start PI loop
	while (! (UCSR0A & _BV(RXC0)));
	
	//y_ref = 20;
	
	while(1) {
		// no idea why my counter_limit is so large but it works for me
		// most people have theirs around 10-20
		if (y_ref < 50) {
			counter_limit = INT_MAX >> 2;
		} else {
			counter_limit = INT_MAX >> 1;
		}
		
		// run control code every counter_limit PWM cycles
		while(!(TIFR2 & _BV(TOV2)));
		
		if (counter < counter_limit) {
			counter++;
		} else {
			PI_control();
			counter = 0;
		}
		
		// check receiver registers if data is in receive buffer
		if (UCSR0A & _BV(RXC0)) {
			received = USART_receive();
			// 'GET' command received
			if ((int) received > 120) {
				USART_transmit((unsigned char) calculate_rpm());
				toggle_status_led4();
			} else { // 'SET' command received
				y_ref = (int) received;
				// adjust controller parameters depending on target speed
				// Motor 1,7 - case 1: K = 1 fine for 1a, 1b, Ti = 1 ; case 2: K = 1, Ti = 3
				// Motor 2,3,4,6,10 - case 1a: K = 2 ; case 1b: K = 1 ; case 2: K = 1, Ti = 2
				// Motor 9 is shit, don't use it
				
				if (y_ref >= y) { // case 1
					// adjust K depending on motor
					if (y_ref >= 100) { // case 1a
						K = 2;
					} else { // case 1b
						K = 1;
					}
					Ti = 1;
				} else { // case 2
					K = 1;
					// adjust Ti depending on motor
					Ti = 2;
				}
				toggle_status_led5();
			}
		}
	}
	return 0;
}

// encoder signal interrupt
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
}