CC = gcc
CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/
vpath %.c ../lib/
vpath %.o ../lib/


all:	raw_eth_client raw_eth_server 

raw_eth_client: raw_eth_client.o ../lib/raw_eth.o
						$(CC) -o $@ $(CFLAGS) raw_eth_client.c ../lib/raw_eth.c

raw_eth_server: raw_eth_server.o ../lib/raw_eth.o
						$(CC) -o $@ $(CFLAGS) raw_eth_server.c ../lib/raw_eth.c

clean:
						clear
						cd ../lib && $(MAKE) clean
						rm -rf raw_eth_client raw_eth_server *.o
