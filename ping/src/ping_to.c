#include "../head/ping.h"

int main(int argc, char **argv) {
	int s, read_bytes, send_icmp;
	socklen_t server_lenght;
	struct sockaddr_in server_addr;
	struct icmphdr icmp_h;
	struct iphdr ip_h;
	struct iphdr *ip_h_rec;
	struct icmphdr *icmp_h_rec;
	char buf[6];
	char ip_s[32];
	unsigned char msg1[IP_HEAD_LEN + UDP_HEAD_LEN + 20];
	uint32_t ip_d = 0;

	memset(buf, 0, sizeof(buf));
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	memset(&ip_h, 0, sizeof(struct iphdr));
	memset(&icmp_h, 0, sizeof(struct icmphdr));



	s = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
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

	strcpy(ip_s, "192.168.1.2");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr (argv[1]);
	
	server_lenght = sizeof(server_addr);	

	ip_h.ihl = 5;
	ip_h.version = 4;
	ip_h.tos = 0;
	ip_h.tot_len = 0;
	ip_h.id = htons(54321);
	ip_h.frag_off = 0;
	ip_h.ttl = 255;
	ip_h.protocol = IPPROTO_ICMP;
	ip_h.check = 0;
	ip_h.saddr = inet_addr(ip_s);
	ip_h.daddr = server_addr.sin_addr.s_addr;

	icmp_h.type = ICMP_ECHO;
	icmp_h.code = 0;
	icmp_h.checksum = 0;

	printf("ping %s\n", argv[1]);
	for(int i = 0; i < 5; i++){
		send_icmp = sendIPICMP(s, icmp_h, ip_h, server_addr, server_lenght);

		read_bytes = recvfrom(s, &msg1, 1024, 0, (struct sockaddr *)&server_addr, &server_lenght);

		ip_h_rec = (struct iphdr *)(msg1);
		icmp_h_rec = (struct icmphdr *)(msg1 + IP_HEAD_LEN);
		
		ip_d = ntohl(ip_h_rec->daddr);

		if(read_bytes >= 0){
			if(icmp_h_rec->type == ICMP_ECHOREPLY)
				if(ip_d == ntohl(ip_h.saddr))
					printf("! ");		
		}	
		else printf(". ");
	}
	printf("\n");	
	close(s);
	exit(EXIT_SUCCESS);
}
