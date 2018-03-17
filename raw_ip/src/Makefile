CC = gcc
CFLAGS = -Wall
LFLAGS =

vpath %.h ../head/
vpath %.c ../lib/
vpath %.o ../lib/


all:	raw_ip_client raw_ip_server 

raw_ip_client: raw_ip_client.o ../lib/raw_ip.o
						$(CC) -o $@ $(CFLAGS) raw_ip_client.c ../lib/raw_ip.c

raw_ip_server: raw_ip_server.o ../lib/raw_ip.o
						$(CC) -o $@ $(CFLAGS) raw_ip_server.c ../lib/raw_ip.c

clean:
						clear
						cd ../lib && $(MAKE) clean
						rm -rf raw_ip_client raw_ip_server *.o
