CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS = -lX11 -lpng -lImlib2
OBJECT = delaykill.o

all: delaykill
delaykill: $(OBJECT)
	$(CC) $(OBJECT) $(LDFLAGS) -o $@

delaykill.o: delaykill.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f delaykill *~ *.o a.out
clean-media:
	rm -f *~ *.png
.PHONY: all clean
