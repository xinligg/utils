CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS =
OBJECT = showjpg.o

all: showjpg
showjpg: $(OBJECT)
	$(CC) $(OBJECT) $(LDFLAGS) -lX11 -lXext -ljpeg -o $@

showjpg.o: showjpg.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f showjpg *~ *.o a.out
clean-media:
	rm -f *~ *.jpeg
.PHONY: all clean
