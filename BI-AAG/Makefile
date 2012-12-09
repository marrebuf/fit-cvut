SHELL = /bin/sh
FLEX = flex
CFLAGS = -std=gnu99 -Wall
CFLAGSFLEX = -lfl
CXXFLAGS = -std=c++0x -Wall

TARGETS = janoupre csolver cxxsolver
CLEAN = janoupre.yy.c

all: $(TARGETS)

janoupre.yy.c: janoupre.lex
	$(FLEX) -o $@ $<
janoupre: janoupre.yy.c
	$(CC) -o $@ $< $(CFLAGSFLEX)

csolver: csolver.c
	$(CC) $(CFLAGS) -o $@ $<
cxxsolver: cxxsolver.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) $(CLEAN)

.PHONY: all clean

