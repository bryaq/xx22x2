#define TRIT0	0x0ul
#define TRIT1	0x3ul
#define TRITF	0x1ul

extern void xx22x2_rx(unsigned char subbit);
extern unsigned char xx22x2_tx(void);
extern void xx22x2_detectosc(unsigned short *subbitp, unsigned short tcnt);

extern void (*xx22x2_callback)(unsigned long code);
extern unsigned long xx22x2_txcode;
