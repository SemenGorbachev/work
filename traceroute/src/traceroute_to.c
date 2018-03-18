#include "../head/traceroute.h"

int main(int argc, char **argv) {
	int s, rc, ttl = 1;
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
	struct timeval timeout = {60, 0}; 
	fd_set read_set;

	memset(buf, 0, sizeof(buf));
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	memset(&ip_h, 0, sizeof(struct iphdr));
	memset(&icmp_h, 0, sizeof(struct icmphdr));
	memset(&read_set, 0, sizeof(read_set));



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
	ip_h.ttl = ttl;
	ip_h.protocol = IPPROTO_ICMP;
	ip_h.check = 0;
	ip_h.saddr = inet_addr(ip_s);
	ip_h.daddr = server_addr.sin_addr.s_addr;

	icmp_h.type = ICMP_ECHO;
	icmp_h.code = 0;
	icmp_h.checksum = 0;

	printf("traceroute %s\n", argv[1]);
	for(int i = 0; i < 255; i++){
		rc = sendIPICMP(s, icmp_h, ip_h, server_addr, server_lenght);
		if(rc < 0) {
			perror("sendto");
			break;
		}

		FD_SET(s, &read_set);
		rc = select(s + 1, &read_set, NULL, NULL, &timeout);
		if (rc == 0) {
            printf("destination ip unreached\n");
            break;
        } 
        else if (rc < 0) {
            perror("select");
            break;
        }

		rc = recvfrom(s, &msg1, 1024, 0, (struct sockaddr *)&server_addr, &server_lenght);
		if(rc <= 0){
			perror("recvfrom");
			break;
		}
		else if(rc < sizeof(icmp_h)){
			printf("Error, got short ICMP packet, %d bytes\n", rc);
            break;
		}

		ip_h_rec = (struct iphdr *)(msg1);
		icmp_h_rec = (struct icmphdr *)(msg1 + IP_HEAD_LEN);
		
		ip_d = ntohl(ip_h_rec->daddr);
		
		if(icmp_h_rec->type == ICMP_ECHOREPLY){
			if(ip_d == ntohl(ip_h.saddr))
				printf("--------------------Success!!!!!--------------------\n%d hop (", ttl);
				print_ip(ntohl(ip_h_rec->saddr));
				printf(") to destination (%s)\n", argv[1]);

				close(s);
				exit(EXIT_SUCCESS);

		}
		else if(icmp_h_rec->type == ICMP_DEST_UNREACH){
			if(ip_d == ntohl(ip_h.saddr))
				printf("Destination ip unreached\n");
				close(s);
				exit(EXIT_SUCCESS);
		}
		else if(icmp_h_rec->type == ICMP_TIME_EXCEEDED){
			if(ip_d == ntohl(ip_h.saddr))
				printf("%d hop (", ttl);
				print_ip(ntohl(ip_h_rec->saddr));
				printf(") to destination (%s)\n", argv[1]);;
				

		}
		ttl++;
		ip_h.check = 0;
		ip_h.ttl = ttl;

		icmp_h.checksum = 0;

	}

	close(s);
	exit(EXIT_SUCCESS);
}