CC = gcc

CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/

traceroute.o: traceroute.c traceroute.h
					gcc -c $< $(CFLAGS) $(LFLAGS)

clean:
					rm -rf *.o

