CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

USER_OBJS=main.o

include ../compil/make.config

all: hangman

hangman: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f hangman
	rm -f $(OBJS) *.o */*.o */*/*.o

install: hangman
	mkdir -p $(DESTDIR)$(BINDIR)/games/
	cp $< $(DESTDIR)$(BINDIR)/games/
	cp dict.txt $(DESTDIR)$(BINDIR)/games/

