NAME=avr_recv
OBJS=avr_recv.o xx22x2.o
CPU=atmega8
CC=avr-gcc
CFLAGS=-ansi -Wall -pedantic -Os -g -mmcu=$(CPU) -I../../src
LDFLAGS=-mmcu=$(CPU)

.PHONY:	all prog clean

all:	$(NAME).elf

$(NAME).elf:	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

xx22x2.o:	../../src/xx22x2.c
	$(CC) $(CFLAGS) -c -o $@ $<

prog:	$(NAME).hex
	avrdude -p $(CPU) -c arduino -P /dev/serial/by-id/usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0 -U flash:w:$(NAME).hex

$(NAME).hex:	$(NAME).elf
	avr-objcopy -j .text -j .data -O ihex $(NAME).elf $(NAME).hex

$(NAME).lst:	$(NAME).elf
	avr-objdump -S $(NAME).elf >$@

clean:
	rm -f $(NAME).elf $(NAME).hex $(NAME).lst $(OBJS)
