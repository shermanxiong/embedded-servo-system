#include <avr/io.h>
#include "pinFunc.h"

void init_leds(void) {
	DDRB |= _BV(PB0) | _BV(PB6) | _BV(PB7);
	DDRD |= _BV(PD5) | _BV(PD6) | _BV(PD7);
}

void toggle_status_led1(void) {
	PORTB ^= _BV(PB6);	
}

void toggle_status_led2(void) {
	PORTB ^= _BV(PB7); 
}

void toggle_status_led3(void) {
	PORTD ^= _BV(PD5);
}

void toggle_status_led4(void) {
	PORTD ^= _BV(PD6);
}

void toggle_status_led5(void) {
	PORTD ^= _BV(PD7);
}
void toggle_error_led(void) {
	PORTB ^= _BV(PB0);
}