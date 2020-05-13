CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

USER_OBJS=main.o

include ../compil/make.config

all: shell

shell: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f shell
	rm -f $(OBJS) *.o */*.o */*/*.o

install: shell
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $(DESTDIR)$(BINDIR)
	

