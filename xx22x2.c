#include "xx22x2.h"

#define BIT0	0x8
#define BIT1	0xe
#define BITS	0x8
#define BITZ	0x0

static void (*callback)(unsigned long code);
static unsigned long txcode;

void
xx22x2_setcallback(void (*f)(unsigned long code))
{
	callback = f;
}

void
xx22x2_setcode(unsigned long code)
{
	unsigned char i;
	
	for(i = 0; i < 24; i++){
		txcode <<= 1;
		txcode |= code & 0x1;
		code >>= 1;
	}
}

void
xx22x2_rx(unsigned char subbit)
{
	static unsigned long code;
	static unsigned char bit, cnt;
	
	bit <<= 1;
	bit |= subbit ? 1 : 0;
	cnt++;
	
	if((cnt & 0x03) == 0){			/* every 4 subbits */
		if(cnt < 25 *4){
			if(((bit & 0xf) != BIT0) && ((bit & 0xf) != BIT1)){	/* incorrect bit */
				cnt = 3;			/* restart from the very beginning */
				return;
			}
			code <<= 1;
			if((bit & 0xf) == BIT1)
				code |= 0x1;
		}else if(cnt == 25 *4){
			if((bit & 0xf) != BITS){	/* incorrect bit */
				cnt = 3;			/* restart from the very beginning */
				return;
			}
		}else if(cnt > 25 *4){
			if((bit & 0xf) != BITZ){	/* incorrect bit */
				cnt = 3;			/* restart from the very beginning */
				return;
			}
		}
		if(cnt == 32 * 4){			/* complete data word */
			cnt = 0;
			if(callback)
				callback(code & 0x00ffffff);
		}
	}
}

unsigned char
xx22x2_tx(void)
{
	static unsigned long code;
	static unsigned char bit, cnt;
	
	bit <<= 1;
	if((cnt & 0x03) == 0){			/* every 4 subbits */
		if(cnt < 24 * 4){
			if(cnt == 0)
				code = txcode;
			bit = (code & 0x1) ? BIT1 : BIT0;
			code >>= 1;
		}else if(cnt == 24 * 4)
			bit = BIT0;
	}
	cnt++;
	if(cnt == 32 * 4)
		cnt = 0;
	return bit & 0x08;				/* decode into subbits */
}

/* XXX: detects correctly only if real subbit < 2048 */
void
xx22x2_detectosc(unsigned short *subbitp, unsigned short tcnt)
{
	static unsigned long sum;
	static unsigned short start;
	static unsigned char cnt;
	
	sum += (tcnt - (unsigned short)sum);
	if(cnt == 25){
		*subbitp = (sum - start) / 128;
		start = tcnt;
		sum = tcnt;
		cnt = 0;
	}
	cnt++;
}

