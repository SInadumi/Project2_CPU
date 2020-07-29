# Makefile cpu

# CC := gcc
# CFLAGS := -g -Wall -Wextra

# main:  cpuboard.o main.o

# cpuboard.o main.o: cpuboard.h

PROGRAM = cpu
OBJS = main.o cpuboard.o
CC = gcc
CFLAGS = -g -Wall -Wextra

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $^

main.o: main.c
	$(CC) $(CFLAGS) -c $<

cpuboard.o: cpuboard.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)