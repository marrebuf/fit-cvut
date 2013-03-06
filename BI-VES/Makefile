SHELL = /bin/sh
CFLAGS = -Wall -std=gnu99 -I. $(shell pkg-config --cflags gtk+-3.0)
LDLIBS = $(shell pkg-config --libs gtk+-3.0 gthread-2.0 gmodule-2.0)

all: test

test: main.o libves.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c p24fxxxx.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f test main.o libves.o

.PHONY: all clean
