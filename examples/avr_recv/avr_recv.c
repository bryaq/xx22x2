#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "xx22x2.h"

/* f0 10 f0 11 00 ff sz zz */
#define MYCODE	((TRITF << 30) | (TRIT0 << 28) | (TRIT1 << 26) | (TRIT0 << 24) | (TRITF << 22) | (TRIT0 << 20) \
				| (TRIT1 << 18) | (TRIT1 << 16) | (TRIT0 << 14) | (TRIT0 << 12) | (TRITF << 10) | (TRITF << 8))

#define LED1		_BV(PB5)
#define TXD		_BV(PD7)
#define RXD		_BV(PD2)
#define BUTTON	_BV(PD3)

#define led_on()	(PORTB |= LED1)
#define led_off()	(PORTB &= ~LED1)

#define F_CPU		8000000ul
#define F_OSC		27000ul
#define SUBBIT		(((F_CPU) / 8 + (F_OSC) / 8) *4 / (F_OSC))

enum{
	EV_TIMER = _BV(0),
	EV_RISING = _BV(1)
};

volatile unsigned short subbit, tcnt;
volatile unsigned char events, rx;

void
callback(unsigned long code)
{
	if(code == MYCODE){
		led_on();
	}
}

int
main(void)
{
	OSCCAL = 0xa7;				/* 8 MHz */
	
	ACSR |= _BV(ACD);
	
	PORTB = 0xff & ~LED1;			/* select pullups */
	DDRB = LED1;
	
	PORTC = 0xff;					/* select pullups */
	
	PORTD = 0xff & ~TXD;			/* select pullups */
	DDRD = TXD;
	
	MCUCR |= _BV(ISC01) | _BV(ISC00);	/* interrupt on rising edge */
	GICR |= _BV(INT0);				/* enable interrupt */
	
	TCCR1B = _BV(CS11);			/* Timer1 normal mode with /8 prescaler*/
	TIMSK = _BV(OCIE1A);			/* enable interrupt */
	subbit = SUBBIT;
	OCR1A = SUBBIT - 1;			/* set to subbit (1/8 bit) period */
	
	xx22x2_setcallback(callback);
	
	sei();							/* global interrupt enable */
	set_sleep_mode(SLEEP_MODE_IDLE);
	while(1){
		sleep_mode();
		if(events & EV_RISING){
			events &= ~EV_RISING;
			xx22x2_detectosc((unsigned short *)&subbit, tcnt);
		}
		if(events & EV_TIMER){
			events &= ~EV_TIMER;
			led_off();
			xx22x2_rx(rx);
		}
	}
}

ISR(INT0_vect)
{
	tcnt = TCNT1;
	OCR1A = tcnt + subbit / 2;		/* calibrate timer */
	events |= EV_RISING;
}

ISR(TIMER1_COMPA_vect)
{
	rx = PIND & RXD;
	OCR1A += subbit;
	events |= EV_TIMER;
}

