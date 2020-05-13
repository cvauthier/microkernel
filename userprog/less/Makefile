CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

.PHONY: all clean install
.SUFFIXES: .o .c .S

USER_OBJS=less.o

include ../compil/make.config

all: less

less: $(OBJS)
	$(CC_LINK) -o $@ $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

clean:
	rm -f less
	rm -f $(OBJS) *.o */*.o */*/*.o

install: less
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $< $(DESTDIR)$(BINDIR)
	

