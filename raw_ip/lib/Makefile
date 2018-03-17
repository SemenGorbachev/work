CC = gcc

CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/

raw_ip.o: raw_ip.c raw_ip.h
					gcc -c $< $(CFLAGS) $(LFLAGS)

clean:
					rm -rf *.o

