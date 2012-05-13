SHELL = /bin/sh
CXXFLAGS = -Wall -pedantic -ggdb

all: auta
main: auta.cpp auta.h
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f auta

.PHONY: clean all

