#include <isr_compat.h>
#include <msp430.h>

#include "xx22x2.h"

/* f0 10 f0 11 00 ff sz zz */
#define MYCODE	((TRITF << 30) | (TRIT0 << 28) | (TRIT1 << 26) | (TRIT0 << 24) | (TRITF << 22) | (TRIT0 << 20) \
				| (TRIT1 << 18) | (TRIT1 << 16) | (TRIT0 << 14) | (TRIT0 << 12) | (TRITF << 10) | (TRITF << 8))

#define LED1		BIT0
#define LED2		BIT6
#define TXD		BIT4
#define RXD		BIT5
#define BUTTON	BIT3

#define led_on()	(P1OUT |= LED1)
#define led_off()	(P1OUT &= ~LED1)

#define F_CPU		8000000ul
#define F_OSC		27000ul
#define SUBBIT		(((F_CPU) / 8 + (F_OSC) / 8) *4 / (F_OSC))

enum{
	EV_TIMER = BIT0,
	EV_RISING = BIT1
};

unsigned short events, subbit, tcnt;
unsigned char rx;

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
	WDTCTL = WDTPW + WDTHOLD;	/* stop watchdog */
	
	DCOCTL = 0;
	BCSCTL1 = 13;
	DCOCTL = (3 << 5) | 17;			/* set DCO to 8 MHz */
	
	P1DIR = LED1 | LED2 | TXD;
	P1OUT = BUTTON | RXD;			/* select pullups/pulldowns */
	P1REN = ~(LED1 | LED2 | TXD);		/* enable pullups/pulldowns */
	P1IE = RXD;					/* enable interrupt */
	
	P2SEL = 0;						/* no alternative functions */
	P2OUT = 0;					/* select pullups/pulldowns */
	P2REN = 0xff;					/* enable pullups/pulldowns */
	
	TACTL = TASSEL_2 | ID_3 | MC_2;	/* Timer_A continuous mode with /8 divider */
	TACCTL0 = CCIE;				/* enable interrupt */
	subbit = SUBBIT;
	TACCR0 = SUBBIT - 1;			/* set to subbit (1/8 bit) period */
	
	xx22x2_setcallback(callback);
	
	_EINT();						/* global interrupt enable */
	while(1){
		LPM0;
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

ISR(PORT1, port1_isr)
{
	if(P1IFG & RXD){				/* signal front 0->1 */
		P1IFG &= ~RXD;			/* clear flag */
		tcnt = TAR;
		TACCR0 = tcnt + subbit / 2;	/* calibrate timer */
		events |= EV_RISING;
		LPM0_EXIT;
	}
}

ISR(TIMERA0, timera0_isr)
{
	rx = P1IN & RXD;
	TACCR0 += subbit;
	events |= EV_TIMER;
	LPM0_EXIT;
}

