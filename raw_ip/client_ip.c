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

#define UDP_HEAD_LEN 8
#define IP_HEAD_LEN 20

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

int sendIPUDP(int s, struct udphdr udph, struct iphdr iph, char * data, int data_lenght, struct sockaddr_in serv, socklen_t serv_lenght){
	unsigned char * buffer;
	char * ip_h_check;
	int res;
	unsigned char h_lenght_udp;
	unsigned char h_lenght_ip;
	unsigned int pac_lenght;

	h_lenght_udp = sizeof(udph);
	h_lenght_ip = sizeof(iph);
	pac_lenght = h_lenght_udp + data_lenght;
	udph.check = 0;
	if(!udph.len) udph.len = htons(pac_lenght);

	pac_lenght = h_lenght_ip + h_lenght_udp + data_lenght;
	if(!iph.tot_len) iph.tot_len = htons(pac_lenght);

	buffer = (char *)calloc(pac_lenght, sizeof(char));
	ip_h_check = (char *)malloc(h_lenght_ip);

	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(ip_h_check, &iph, h_lenght_ip);
	iph.check = csum((unsigned short *) ip_h_check, IP_HEAD_LEN);
	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(buffer + h_lenght_ip, &udph, h_lenght_udp);

	if(data) memcpy(buffer + h_lenght_udp + h_lenght_ip, data, data_lenght);
	//iph.check = csum((unsigned short *) buffer, ntohs(iph.tot_len));
	printf("Check Sum: %d\n", iph.check);
	//memcpy(buffer, &iph, h_lenght_ip);
	for(int i = 0; i < pac_lenght; i++)
		printf("%x ", buffer[i]);

	res = sendto(s, buffer, pac_lenght, 0, (struct sockaddr *) &serv, serv_lenght); 

	return res;


} 

int main(int argc, char **argv) {
	int s, read_bytes, send_udp, str_len, str_len_r_buf;
	socklen_t client_lenght, server_lenght;
	struct sockaddr_in client_addr, server_addr;
	struct udphdr udp_h;
	struct iphdr ip_h;
	struct in_addr ip_addr;
	char buf[6], *a;
	char ip_s[32], ip_d_char[32];
	unsigned char msg1[IP_HEAD_LEN + UDP_HEAD_LEN + 20];
	char msg2[] = "Hello!";
	unsigned short port = 0;
	uint32_t ip_d = 0;

	memset(buf, 0, sizeof(buf));




	s = socket (AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}

	int one = 1;
    const int *val = &one;
     
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }

	strcpy(ip_s, "192.168.1.3");

	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr (argv[2]);
	
	client_lenght = sizeof(client_addr);
	server_lenght = sizeof(server_addr);	

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
	ip_h.daddr = server_addr.sin_addr.s_addr;



	udp_h.source = htons(65533);
	udp_h.dest = htons(atoi(argv[1]));
	udp_h.len = 0;
	udp_h.check = 0;


	while(1) {
		scanf("%s", buf);
		send_udp = sendIPUDP(s, udp_h, ip_h, buf, sizeof(buf), server_addr, server_lenght);

		while(1) {
			read_bytes = recvfrom(s, &msg1, 1024, 0, (struct sockaddr *)&server_addr, &server_lenght);
			printf("read_bytes: %d\n", read_bytes);
			


			struct iphdr *ip_h_rec = (struct iphdr *)(msg1);
			struct udphdr *udp_h_rec = (struct udphdr *)(msg1 + IP_HEAD_LEN);
			

			port = ntohs(udp_h_rec->dest);
			ip_d = ntohl(ip_h_rec->daddr);
			ip_addr.s_addr = ip_d;

			printf("port: %d  ",port);
			print_ip(ip_d);

			if(read_bytes >= 0)
				if(port == ntohs(udp_h.source))
					if(ip_d == ntohl(ip_h.saddr))
						printf("Msg: %s\n", msg1 + IP_HEAD_LEN + UDP_HEAD_LEN);		
		}
		
				
	}
	close(s);
	free(msg1);
	
	exit(EXIT_SUCCESS);
}
