SHELL = /bin/sh
RM = rm -f
CC = gcc
CFLAGS = -Wall -lpthread -ggdb

.PHONY: all clean

all: robot

robot: robot.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) robot

