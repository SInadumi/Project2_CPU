# Makefile cpu

PROGRAM = cpu
OBJS = main.o cpuboard.o
CC = gcc
CFLAGS = -Wall -O2

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $^

main.o: main.c
	$(CC) $(CFLAGS) -c $<

cpuboard.o: cpuboard.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)