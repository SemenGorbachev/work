#include "../head/raw_ip.h"

int main(int argc, char **argv) {
	int s, read_bytes, send_udp;
	socklen_t server_lenght;
	struct sockaddr_in server_addr;
	struct udphdr udp_h;
	struct iphdr ip_h;
	struct in_addr ip_addr;
	char buf[6];
	char ip_s[32];
	unsigned char msg1[IP_HEAD_LEN + UDP_HEAD_LEN + 20];
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

	strcpy(ip_s, "127.0.0.7");

	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr (argv[2]);
	
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