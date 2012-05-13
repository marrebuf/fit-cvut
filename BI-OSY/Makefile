SHELL = /bin/sh
LIBS = -lm -pthread -lrt
CXXFLAGS = -Wall -pedantic -ggdb -Wno-long-long

TARGETS = $(basename $(wildcard ukol*.cpp))

all: $(TARGETS)

ukol%: ukol%.cpp test_%.cpp common_%.h
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $(filter-out %.h,$^)

# Possibly other variants
ukolssvc%: ukolssvc%.cpp test_ssvc.cpp common_ssvc.h
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $(filter-out %.h,$^)
ukolfs%: ukolfs%.cpp test_fs.cpp common_fs.h
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $(filter-out %.h,$^)
ukolraid%: ukolraid%.cpp test_raid.cpp common_raid.h
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $(filter-out %.h,$^)

clean:
	rm -f $(TARGETS)

.PHONY: all clean

