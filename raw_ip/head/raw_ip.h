#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/udp.h> 
#include <netinet/ip.h>

#define UDP_HEAD_LEN 8
#define IP_HEAD_LEN 20

void print_ip(int ip);
unsigned short csum(unsigned short *ptr, int nbytes);
int sendIPUDP(int s, struct udphdr udph, struct iphdr iph, char * data, int data_lenght, struct sockaddr_in serv, socklen_t serv_lenght);

