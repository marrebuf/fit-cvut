SHELL = /bin/sh
TARGETS = $(patsubst %.gpi,%.png,$(wildcard *.gpi))

all: $(TARGETS)

%.png: %.gpi data.txt
	gnuplot $<
clean:
	rm -f $(TARGETS)

.PHONY: all clean

