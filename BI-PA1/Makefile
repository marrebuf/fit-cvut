SHELL = /bin/sh
LIBS = -lm

TARGETS = $(basename $(wildcard ukol*.c))

all: $(TARGETS)

ukol%: ukol%.c
	$(CXX) -o $@ -Wall -pedantic $< $(LIBS)

clean:
	@rm -f $(TARGETS)

.PHONY: all clean

