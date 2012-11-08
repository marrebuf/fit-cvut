SHELL = /bin/sh
LIBS = -lm
CXXFLAGS = -Wall -pedantic -ggdb -Wno-long-long

TARGETS = btreeplus sorttest
OUTPUTS = sorttest.dat btree_*.png

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

sorttest: sorttest.cpp
	$(CXX) -Wall -std=c++0x -O3 -o $@ $<
	
clean:
	rm -f $(TARGETS) $(OUTPUTS)

.PHONY: all clean

