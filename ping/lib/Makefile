CC = gcc

CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/

raw_ip.o: ping.c ping.h
					gcc -c $< $(CFLAGS) $(LFLAGS)

clean:
					rm -rf *.o

