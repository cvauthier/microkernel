CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

USER_OBJS=main.o

include ../compil/make.config

all: moreless

moreless: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f moreless
	rm -f $(OBJS) *.o */*.o */*/*.o

install: moreless
	mkdir -p $(DESTDIR)$(BINDIR)/games/
	cp $< $(DESTDIR)$(BINDIR)/games/

