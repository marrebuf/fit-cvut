SHELL = /bin/sh
LIBS = -lm
CXXFLAGS = -Wall -pedantic -ggdb -Wno-long-long

TARGETS = $(basename $(wildcard ukol*.cpp cv*.cpp))

all: $(TARGETS)

cv%: cv%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)
	
ukol%: ukol%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean

