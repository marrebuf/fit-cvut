SHELL = /bin/sh
CXXFLAGS = -Wall -pedantic -ggdb

all: main
main: main.cpp array.h queue-list.h queue-array.h
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f main

.PHONY: clean all

