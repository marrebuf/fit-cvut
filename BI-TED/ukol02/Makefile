SHELL = /bin/sh

BASENAMES = $(basename $(wildcard ukol*.tex))
TARGETS = $(addsuffix .pdf,$(BASENAMES))
CLEAN = $(addsuffix .log,$(BASENAMES)) $(addsuffix .aux,$(BASENAMES))

all: $(TARGETS)

ukol%.pdf ukol%.aux ukol%.log: ukol%.tex
	pdflatex $<
	pdflatex $<

clean:
	rm -f $(TARGETS) $(CLEAN)

.PHONY: all clean


