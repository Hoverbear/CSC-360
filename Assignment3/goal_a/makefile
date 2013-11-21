#
# "makefile" for the CPU scheduler simulation.
#

CC=gcc
CFLAGS=-c -Wall -g

all: gentasks simsched

gentasks.o: gentasks.c
	$(CC) $(CFLAGS) gentasks.c

simsched.o: simsched.c
	$(CC) $(CFLAGS) simsched.c

gentasks: gentasks.o
	$(CC) gentasks.o -o gentasks

simsched: simsched.o
	$(CC) simsched.o -o simsched

clean:
	rm -rf *.o gentasks simsched
