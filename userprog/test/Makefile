CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

include ../compil/make.config

all: test

test: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f a.out
	rm -f $(OBJS) *.o */*.o */*/*.o

install: test
	mkdir -p $(DESTDIR)$(BINDIR)
	cp test $(DESTDIR)$(BINDIR)
	

