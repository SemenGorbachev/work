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
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>

#define ETH_HEAD_LEN 14
#define UDP_HEAD_LEN 8
#define IP_HEAD_LEN 20
#define INT_DEF "enp2s0f0"
#define IP_SOURCE "192.168.1.3"

void print_ip(int ip);
unsigned short csum(unsigned short *ptr, int nbytes);
int sendETHIPUDP(int s, struct udphdr udph, struct iphdr iph, struct ether_header ethh, char * data, int data_lenght, struct sockaddr_ll serv, socklen_t serv_lenght);

