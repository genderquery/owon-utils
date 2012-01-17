CC = gcc
CFLAGS = -Wall -g# -O2
LDFLAGS = -L.
BINARIES = owondump owonparse
AR = ar
ARFLAGS = rcs

all: $(BINARIES)

owondump: owondump.o usb.o
	$(CC) $(CFLAGS) -o owondump owondump.o usb.o -lusb

owonparse: owonparse.o parse.o
	$(CC) $(CFLAGS) -o owonparse owonparse.o parse.o

owondump.o: owon.h usb.h usb.c owondump.c
	$(CC) $(CFLAGS) -c owondump.c

owonparse.o: owon.h parse.h parse.o owonparse.c
	$(CC) $(CFLAGS) -c owonparse.c

#libowon.a: usb.o parse.o
#	$(AR) $(ARFLAGS) libowon.a usb.o parse.o

#libowon.so: usb.o parse.o
#	$(CC) $(CFLAGS) -shared -o libowon.so usb.o parse.o -lusb
## add -fPIC to *.o below for shared lib

usb.o: owon.h usb.h usb.c
	$(CC) $(CFLAGS) -c usb.c

parse.o: owon.h parse.h parse.c
	$(CC) $(CFLAGS) -c parse.c

clean:
	rm -f *.o *.exe *.a *.so $(BINARIES)
