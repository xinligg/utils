CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS =
SCREENOBJECT = screen.o
GIBLIBCFLAGS = `giblib-config --cflags`
GIBLIBLIBS = `giblib-config --libs`

all: screen
screen: $(SCREENOBJECT)
	$(CC) $(SCREENOBJECT) $(LDFLAGS) $(GIBLIBLIBS) -lX11 -o $@

screen.o: screen.c
	$(CC) $(CFLAGS) $(GIBLIBCFLAGS) -c $<

clean:
	rm -f screen *~ *.o a.out
clean-media:
	rm -f *~ *.jpeg
.PHONY: all clean
