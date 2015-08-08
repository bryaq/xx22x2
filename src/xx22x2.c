#include "xx22x2.h"

#define BIT0	0x88
#define BIT1	0xee
#define BITF	0x8e
#define BITS	0x80
#define BITZ	0x00

static void (*callback)(unsigned long code);
static unsigned char txcode[16];

void
xx22x2_setcallback(void (*f)(unsigned long code))
{
	callback = f;
}

void
xx22x2_setcode(unsigned long code)
{
	unsigned char i;
	
	code >>= 8;
	for(i = 12; i > 0; i--){
		switch((unsigned char)code & 0x03){
		case TRIT0:
			txcode[i - 1] = BIT0;
			break;
		case TRIT1:
			txcode[i - 1] = BIT1;
			break;
		case TRITF:
			txcode[i - 1] = BITF;
			break;
		}
		code >>= 2;
	}
	txcode[12] = BITS;
	txcode[13] = BITZ;
	txcode[14] = BITZ;
	txcode[15] = BITZ;
}

void
xx22x2_rx(unsigned char subbit)
{
	static unsigned long code;
	static unsigned char bit, cnt;
	
	bit <<= 1;
	bit |= subbit ? 1 : 0;
	cnt++;
	
	if((cnt & 0x07) == 0){			/* every 8 subbits */
		switch(bit){
		case BITZ:
			if(cnt < 14 * 8){			/* unexpected zero */
				cnt = 0;
				return;
			}
			break;
		case BITS:
			if(cnt != 13 * 8){		/* unexpected sync */
				cnt = 0;
				return;
			}
			break;
		case BIT0:
		case BIT1:
		case BITF:
			if(cnt > 12 * 8){			/* unexpected AD bit */
				if(cnt == 13 * 8)	/* instead of sync bit */
					cnt -= 8;		/* treat as 12th bit */
				else				/* instead of zero bit */
					cnt =8;		/* treat as 1st bit */
			}
			break;
		default:					/* incorrect bit */
			cnt = 7;				/* restart from the very beginning */
			return;
		}
		code <<= 2;
		switch(bit){
		case BIT0:
			code |= TRIT0;
			break;
		case BIT1:
			code |= TRIT1;
			break;
		case BITF:
			code |= TRITF;
			break;
		}
		if(cnt == 16 * 8){			/* complete data word */
			cnt = 0;
			if(callback)
				callback(code);
		}
	}
}

unsigned char
xx22x2_tx(void)
{
	static unsigned char bit, cnt;
	
	bit <<= 1;
	if((cnt & 0x07) == 0)				/* every 8 subbits */
		bit = txcode[(cnt >> 3) & 0x0f];
	cnt++;
	return bit & 0x80;				/* decode into subbits */
}
