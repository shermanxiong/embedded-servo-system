//#define POLLING
#define INTERRUPT

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "utils/pinFunc.h"
#include "dimmer.h"
/*
static volatile unsigned char AB;
static volatile unsigned char count;

void init_pci(void) {
	// When PCIE1 bit set = PCINT14:8 interrupts enabled 
	PCICR |= _BV(PCIE1);
	// Enable PCINT8 and PCINT9 pins (B and A respectively)
	PCMSK1 |= _BV(PCINT8) | _BV(PCINT9);
}

void init_encoder(void) {
	count = 0;
	PORTC |= _BV(PC0) | _BV(PC1);
}

void init_pwm(void) {
	DDRD |= _BV(PD3);
	TCCR2A |= _BV(COM2B0) | _BV(COM2B1) | _BV(WGM20) | _BV(WGM21);
	TCCR2B |= _BV(CS21);
}

void display_count(unsigned char num) {
	//PB6 PB7 PD5 PD6 PD7
	PORTD = num << PD7 | ((num >> 1) & 1) << PD6 | ((num >> 2) & 1) << PD5;
	PORTB = ((num >> 3) & 1) << PB7 | ((num >> 4) & 1) << PB6;
}

void incr_count(void) {
	if(count < 255) {
		count++;
		display_count(count);
		OCR2B = 255 - count;
	} else {
		toggle_error_led();
	}
}

void decr_count(void) {
	if(count > 0) {
		count--;
		display_count(count);
		OCR2B = 255 - count;
	} else {
		toggle_error_led();
	}
}

ISR(PCINT1_vect) {
	// get value of encoder
	unsigned char BA_new = PINC;
	// use Rene Sommer's algorithm to get XOR sum to determine forward or backward
	unsigned char sum = AB^BA_new;
	
	// forward = 1, backward = 2
	if(sum == 1) {
		incr_count();
	} else if (sum == 2) {
		decr_count();
	}
	// swap A and B bits
	AB = (BA_new >> 1 & 1) | (BA_new << 1 & 2);
}


int main(void) {
	init_encoder();
	init_leds();
	init_pwm();
	
#ifdef POLLING	
	while(1) {
		// get value of encoder
		unsigned char BA_new = PINC;
		// use Rene Sommer's algorithm to get XOR sum to determine forward or backward
		unsigned char sum = AB^BA_new;
		
		// forward = 1, backward = 2
		if(sum == 1) {
			incr_count();
		} else if (sum == 2) {
			decr_count();
		}
		// swap A and B bits
		AB = (BA_new >> 1 & 1) | (BA_new << 1 & 2);
	}
#endif

#ifdef INTERRUPT
	init_pci();
	// set interrupts
	sei();
	//OCR2B = 100;
	while(1) {};
#endif
	return 0;
}*/
