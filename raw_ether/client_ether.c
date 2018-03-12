#include <pcap.h>  
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

void print_ip(int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    printf("ip: %d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);        
}


unsigned short csum(unsigned short *ptr, int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    unsigned short answer;
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
    return(answer);
}

int sendETHIPUDP(int s, struct udphdr udph, struct iphdr iph, struct ether_header ethh, char * data, int data_lenght, struct sockaddr_ll serv, socklen_t serv_lenght){
	unsigned char * buffer;
	char * ip_h_check;
	int res;
	unsigned char h_lenght_udp;
	unsigned char h_lenght_ip;
	unsigned char h_lenght_eth;
	unsigned int pac_lenght;

	h_lenght_eth = sizeof(ethh);
	h_lenght_udp = sizeof(udph);
	h_lenght_ip = sizeof(iph);
	pac_lenght = h_lenght_udp + data_lenght;
	udph.check = 0;
	if(!udph.len) udph.len = htons(pac_lenght);

	pac_lenght = h_lenght_ip + h_lenght_udp + data_lenght;
	if(!iph.tot_len) iph.tot_len = htons(pac_lenght);

	pac_lenght = h_lenght_eth + h_lenght_ip + h_lenght_udp + data_lenght;


	buffer = (char *)calloc(pac_lenght, sizeof(char));
	ip_h_check = (char *)malloc(h_lenght_ip);

	memcpy(buffer, &ethh, h_lenght_eth);
	memcpy(ip_h_check, &iph, h_lenght_ip);
	iph.check = csum((unsigned short *) ip_h_check, IP_HEAD_LEN);
	//memcpy(buffer, &iph, h_lenght_ip);
	memcpy(buffer + h_lenght_eth, &iph, h_lenght_ip);
	memcpy(buffer + h_lenght_eth + h_lenght_ip, &udph, h_lenght_udp);

	if(data) memcpy(buffer + h_lenght_eth + h_lenght_udp + h_lenght_ip, data, data_lenght);
	//iph.check = csum((unsigned short *) buffer, ntohs(iph.tot_len));
	printf("Check Sum: %d\n", iph.check);

	res = sendto(s, buffer, pac_lenght, 0, (struct sockaddr *) &serv, serv_lenght); 
	for(int i = 0; i < pac_lenght; i++)
		printf("%x ", buffer[i]);
	return res;


} 

int main(int argc, char **argv) {
	int s, read_bytes, send_udp, str_len, str_len_r_buf;
	socklen_t client_lenght, server_lenght;
	//struct sockaddr_in server_addr;
	struct sockaddr_ll socket_address;
	struct udphdr udp_h;
	struct iphdr ip_h;
	struct ether_header ether_h;
	struct in_addr ip_addr;

	char inter_name[IFNAMSIZ];
	struct ifreq if_idx, if_mac;

	char buf[6], *a;
	char ip_s[32], ip_d_char[32];
	unsigned char msg1[ETH_HEAD_LEN + IP_HEAD_LEN + UDP_HEAD_LEN + 20];
	char msg2[] = "Hello!";
	unsigned short port = 0;
	uint32_t ip_d = 0;
	uint8_t mac_addr[6];
	int values[6];
	

	strcpy(ip_s, IP_SOURCE);
	memset(buf, 0, sizeof(buf));

	if (argc > 1) {
		strncpy(inter_name, argv[1], IFNAMSIZ);
		if(argc > 2)
			if(6 == sscanf(argv[2], "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]))
				for(int i = 0; i < 6; i++)
					mac_addr[i] = (uint8_t)values[i];
	}
	else strncpy(inter_name, INT_DEF, IFNAMSIZ);

	printf("Sending to MAC %02x:%02x:%02x:%02x:%02x:%02x from %s interface\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], inter_name);





	s = socket (AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}


	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, inter_name, IFNAMSIZ - 1);
	if (ioctl(s, SIOCGIFINDEX, &if_idx) < 0)
		perror("SIOCGIFINDEX");

	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, inter_name, IFNAMSIZ - 1);
	if (ioctl(s, SIOCGIFHWADDR, &if_mac) < 0)
		perror("SIOCGIFHWADDR");


	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	socket_address.sll_addr[0] = mac_addr[0];
	socket_address.sll_addr[1] = mac_addr[1];
	socket_address.sll_addr[2] = mac_addr[2];
	socket_address.sll_addr[3] = mac_addr[3];
	socket_address.sll_addr[4] = mac_addr[4];
	socket_address.sll_addr[5] = mac_addr[5];

	server_lenght = sizeof(struct sockaddr_ll);	

	ether_h.ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	ether_h.ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	ether_h.ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	ether_h.ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	ether_h.ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	ether_h.ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	ether_h.ether_dhost[0] = mac_addr[0];
	ether_h.ether_dhost[1] = mac_addr[1];
	ether_h.ether_dhost[2] = mac_addr[2];
	ether_h.ether_dhost[3] = mac_addr[3];
	ether_h.ether_dhost[4] = mac_addr[4];
	ether_h.ether_dhost[5] = mac_addr[5];
	ether_h.ether_type = htons(ETH_P_IP);

	ip_h.ihl = 5;
	ip_h.version = 4;
	ip_h.tos = 0;
	ip_h.tot_len = 0;
	ip_h.id = htons(54321);
	ip_h.frag_off = 0;
	ip_h.ttl = 255;
	ip_h.protocol = IPPROTO_UDP;
	ip_h.check = 0;
	ip_h.saddr = inet_addr(ip_s);
	ip_h.daddr = inet_addr("192.168.1.6");



	udp_h.source = htons(65533);
	udp_h.dest = htons(65531);
	udp_h.len = 0;
	udp_h.check = 0;


	while(1) {
		scanf("%s", buf);
		send_udp = sendETHIPUDP(s, udp_h, ip_h, ether_h, buf, sizeof(buf), socket_address, server_lenght);

		while(1) {
			read_bytes = recvfrom(s, &msg1, 1024, 0, (struct sockaddr *)&socket_address, &server_lenght);
			printf("read_bytes: %d\n", read_bytes);
			

			struct ether_header *eth_h_rec = (struct ether_header *)(msg1);
			struct iphdr *ip_h_rec = (struct iphdr *)(msg1 + ETH_HEAD_LEN);
			struct udphdr *udp_h_rec = (struct udphdr *)(msg1 + ETH_HEAD_LEN + IP_HEAD_LEN);
			

			port = ntohs(udp_h_rec->dest);
			ip_d = ntohl(ip_h_rec->daddr);
			ip_addr.s_addr = ip_d;

			printf("port: %d  ",port);
			print_ip(ip_d);

			if(read_bytes >= 0)
				if(port == ntohs(udp_h.source))
					if(ip_d == ntohl(ip_h.saddr))
						printf("Msg: %s\n", msg1 + ETH_HEAD_LEN + IP_HEAD_LEN + UDP_HEAD_LEN);		
		}
		
				
	}
	close(s);
	free(msg1);
	
	exit(EXIT_SUCCESS);
}
