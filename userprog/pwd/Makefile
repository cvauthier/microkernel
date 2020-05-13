CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

USER_OBJS=pwd.o

include ../compil/make.config

all: pwd

pwd: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f pwd
	rm -f $(OBJS) *.o */*.o */*/*.o

install: pwd
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $(DESTDIR)$(BINDIR)
	

