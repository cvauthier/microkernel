.PHONY: all clean install
.SUFFIXES: .S .o

OBJS=crt0.o crti.o crtn.o crtbegin.o crtend.o

all: $(OBJS)

crtbegin.o crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.S.o:
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

install: $(OBJS)

clean:
	rm -f $(OBJS) *.o */*.o */*/*.o

