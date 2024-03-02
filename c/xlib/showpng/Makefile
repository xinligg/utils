CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS =
OBJECT = showpng.o

all: showpng
showpng: $(OBJECT)
	$(CC) $(OBJECT) $(LDFLAGS) -lX11 -lXext -lpng -o $@

showpng.o: showpng.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f showpng *~ *.o a.out
clean-media:
	rm -f *~ *.jpeg
.PHONY: all clean
