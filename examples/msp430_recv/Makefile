NAME=msp430_recv
OBJS=msp430_recv.o xx22x2.o
CPU=msp430g2211
CC=msp430-gcc
CFLAGS=-ansi -Wall -pedantic -Os -g -mmcu=$(CPU) -I../..
LDFLAGS=-mmcu=$(CPU)

.PHONY:	all prog clean

all:	$(NAME).elf

$(NAME).elf:	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

xx22x2.o:	../../xx22x2.c
	$(CC) $(CFLAGS) -c -o $@ $<

prog:	$(NAME).elf
	mspdebug rf2500 'prog $<'

clean:
	rm -f $(NAME).elf $(OBJS)
