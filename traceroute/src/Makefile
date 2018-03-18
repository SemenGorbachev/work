CC = gcc
CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/
vpath %.c ../lib/
vpath %.o ../lib/ 

traceroute_to: traceroute_to.o ../lib/traceroute.o
						$(CC) -o $@ $(CFLAGS) traceroute_to.c ../lib/traceroute.c

clean:
						clear
						cd ../lib && $(MAKE) clean
						rm -rf traceroute_to *.o
