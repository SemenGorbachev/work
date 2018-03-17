CC = gcc

CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/

raw_ip.o: raw_eth.c raw_eth.h
					gcc -c $< $(CFLAGS) $(LFLAGS)

clean:
					rm -rf *.o

