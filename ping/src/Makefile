CC = gcc
CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/
vpath %.c ../lib/
vpath %.o ../lib/ 

ping_to: ping_to.o ../lib/ping.o
						$(CC) -o $@ $(CFLAGS) ping_to.c ../lib/ping.c

clean:
						clear
						cd ../lib && $(MAKE) clean
						rm -rf ping_to *.o
