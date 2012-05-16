SHELL = /bin/sh
CXXFLAGS = -Wall -pedantic -ggdb -Wno-long-long

TARGETS = $(basename $(wildcard ukol*.cpp))

all: $(TARGETS)

ukol%: ukol%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean


