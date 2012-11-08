SHELL = /bin/sh
CFLAGS = -std=gnu99 -Wall
CXXFLAGS = -std=c++0x -Wall

TARGETS = csolver cxxsolver

all: $(TARGETS)

csolver: csolver.c
	$(CC) $(CFLAGS) -o $@ $<
cxxsolver: cxxsolver.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean

