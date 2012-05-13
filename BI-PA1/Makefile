SHELL = /bin/sh
LIBS = -lm
CXXFLAGS = -Wall -pedantic -ggdb -Wno-long-long

TARGETS = $(basename $(wildcard ukol*.c))

all: $(TARGETS)

ukol%: ukol%.c
	$(CXX) $(CXXFLAGS) -o $@ -Wall -pedantic $< $(LIBS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean

