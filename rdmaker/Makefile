CC = gcc
CFLAGS = -Wall -O2 -c
OBJS = main.o filenode.o ramdisk.o
SRCS = main.c filenode.c ramdisk.c

all: rdmaker
install : rdmaker

rdmaker: $(OBJS)
	$(CC) -o $@ $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -f $(OBJS)
	rm -f rdmaker

depend:
	makedepend -I. $(SRCS)

install-headers:

