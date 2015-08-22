#include <isr_compat.h>
#include <msp430.h>

#include "xx22x2.h"

/* f0 10 f0 11 00 ff */
#define MYCODE	((TRITF << 22) | (TRIT0 << 20) | (TRIT1 << 18) | (TRIT0 << 16) \
				| (TRITF << 14) | (TRIT0 << 12) | (TRIT1 << 10) | (TRIT1 << 8) \
				| (TRIT0 << 6) | (TRIT0 << 4) | (TRITF << 2) | (TRITF << 0))

#define LED1		BIT0
#define LED2		BIT6
#define TXD		BIT4
#define RXD		BIT5
#define BUTTON	BIT3

#define led_on()	(P1OUT |= LED1)
#define led_off()	(P1OUT &= ~LED1)

#define F_CPU		8000000ul
#define F_OSC		27000ul
#define SUBBIT		(((F_CPU) + (F_OSC) / 8) * 4 / (F_OSC))

enum{
	EV_TIMER = BIT0
};

unsigned short events;
unsigned char tx;

int
main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	/* stop watchdog */
	
	DCOCTL = 0;
	BCSCTL1 = 13;
	DCOCTL = (3 << 5) | 17;			/* set DCO to 8 MHz */
	
	P1DIR = LED1 | LED2 | TXD;
	P1OUT = BUTTON;				/* select pullups/pulldowns */
	P1REN = ~(LED1 | LED2 | TXD);		/* enable pullups/pulldowns */
	P1IE = BUTTON;				/* enable interrupt */
	
	P2SEL = 0;						/* no alternative functions */
	P2OUT = 0;					/* select pullups/pulldowns */
	P2REN = 0xff;					/* enable pullups/pulldowns */
	
	TACTL = TASSEL_2 | MC_1;		/* Timer_A setup */
	TACCTL0 = CCIE;				/* enable interrupt */
	TACCR0 = SUBBIT - 1;			/* set to subbit (1/8 bit) period */
	
	xx22x2_txcode = MYCODE;
	
	_EINT();						/* global interrupt enable */
	while(1){
		LPM0;
		if(events & EV_TIMER){
			events &= ~EV_TIMER;
			if((P1IN & BUTTON) == 0)
				tx = xx22x2_tx();
			else
				LPM4;
		}
	}
}

ISR(PORT1, port1_isr)
{
	if(P1IFG & BUTTON){				/* signal front 1->0 */
		P1IFG &= ~BUTTON;			/* clear flag */
		LPM4_EXIT;				/* wake up */
	}
}

ISR(TIMERA0, timera0_isr)
{
	if(tx)
		P1OUT |= TXD;
	else
		P1OUT &= ~TXD;
	events |= EV_TIMER;
	LPM0_EXIT;
}

