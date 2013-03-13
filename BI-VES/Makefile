SHELL = /bin/sh
CFLAGS = -Wall -std=gnu99 -I. $(shell pkg-config --cflags gtk+-3.0)
LDLIBS = $(shell pkg-config --libs gtk+-3.0 gthread-2.0 gmodule-2.0) -lm

all: graphics touch-app ticket-machine pwm-ui

graphics: graphics.o libves.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

graphics.o: graphics.c p24fxxxx.h
	$(CC) $(CFLAGS) -c -o $@ $<


touch-app: touch-app.o libves.o touch.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

touch-app.o: touch-app.c touch.h p24fxxxx.h
	$(CC) $(CFLAGS) -c -o $@ $<


pwm-ui: pwm-ui.o libves.o touch.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

pwm-ui.o: pwm-ui.c touch.h p24fxxxx.h
	$(CC) $(CFLAGS) -c -o $@ $<


ticket-machine: ticket-machine.o libves.o touch.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ticket-machine.o: ticket-machine.c touch.h p24fxxxx.h
	$(CC) $(CFLAGS) -c -o $@ $<


libves.o: libves.c libves.h
	$(CC) $(CFLAGS) -c -o $@ $<

touch.o: touch.c touch.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f graphics graphics.o \
	      ticket-machine ticket-machine.o \
		  pwm-ui pwm-ui.o \
		  touch-app touch-app.o \
		  touch.o libves.o

.PHONY: all clean
