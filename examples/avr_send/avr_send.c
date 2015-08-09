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
#define SUBBIT		(((F_CPU) + (F_OSC) / 8) *4 / (F_OSC))

enum{
	EV_TIMER = _BV(0)
};

volatile unsigned char events, tx;

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
	
	MCUCR |= 0;					/* interrupt on low level */
	GICR |= _BV(INT1);				/* enable interrupt */
	
	TCCR1B = _BV(WGM12) | _BV(CS10);	/* Timer1 setup */
	TIMSK = _BV(OCIE1A);			/* enable interrupt */
	OCR1A = SUBBIT - 1;			/* set to subbit (1/8 bit) period */
	
	xx22x2_setcode(MYCODE);
	
	sei();							/* global interrupt enable */
	while(1){
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
		if(events & EV_TIMER){
			events &= ~EV_TIMER;
			if((PIND & BUTTON) == 0)
				tx = xx22x2_tx();
			else{
				set_sleep_mode(SLEEP_MODE_PWR_DOWN);
				cli();
				GICR |= _BV(INT1);	/* enable interrupt */
				sleep_enable();
				sei();
				sleep_cpu();
				sleep_disable();
			}
		}
	}
}

ISR(INT1_vect)
{
	GICR &= ~_BV(INT1);			/* disable interrupt */
}

ISR(TIMER1_COMPA_vect)
{
	if(tx)
		PORTD |= TXD;
	else
		PORTD &= ~TXD;
	events |= EV_TIMER;
}

