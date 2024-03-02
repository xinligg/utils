CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS = -lX11 -lImlib2
OBJECT = xrootbg.o

all: xrootbg
xrootbg: $(OBJECT)
	$(CC) $(OBJECT) $(LDFLAGS) -lX11 -o $@

xrootbg.o: xrootbg.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f xrootbg *~ *.o a.out
clean-media:
	rm -f *~ *.jpeg
.PHONY: all clean
