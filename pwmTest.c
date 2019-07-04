#define F_CPU 4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "utils/pinFunc.h"

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

void incr_count(void) {
	if(count < 255) {
		count++;
		OCR2B = 255 - count;
	} else {
		toggle_error_led();
	}
}

void decr_count(void) {
	if(count > 0) {
		count--;
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

int main(void)
{
	init_pwm();
	init_pci();
	init_encoder();
	init_leds();
	
	// PWM inverted in code => 0 = high, 255 = low
	//OCR2B = 100;
	while(1);
}
