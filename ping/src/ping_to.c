#include "../head/ping.h"

int main(int argc, char **argv) {
	int s, read_bytes, send_icmp, rc, packet_check_send = 0, packet_check_recv = 0;
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
	struct timeval timeout = {5, 0}; 
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
	ip_h.ttl = 255;
	ip_h.protocol = IPPROTO_ICMP;
	ip_h.check = 0;
	ip_h.saddr = inet_addr(ip_s);
	ip_h.daddr = server_addr.sin_addr.s_addr;

	icmp_h.type = ICMP_ECHO;
	icmp_h.code = 0;
	icmp_h.checksum = 0;

	printf("ping %s\n", argv[1]);
	unsigned int start_time =  clock();
	for(int i = 0; i < 5; i++){
		rc = sendIPICMP(s, icmp_h, ip_h, server_addr, server_lenght);
		if(rc < 0) {
			perror("sendto");
			break;
		}
		else packet_check_send ++;

		FD_SET(s, &read_set);
		rc = select(s + 1, &read_set, NULL, NULL, &timeout);
		if (rc == 0) {
            printf(". ");
            continue;
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
		//else packet_check_recv ++;

		ip_h_rec = (struct iphdr *)(msg1);
		icmp_h_rec = (struct icmphdr *)(msg1 + IP_HEAD_LEN);
		
		ip_d = ntohl(ip_h_rec->daddr);
		
		if(icmp_h_rec->type == ICMP_ECHOREPLY)
			if(ip_d == ntohl(ip_h.saddr))
				packet_check_recv ++;
				printf("! ");	
	}
	unsigned int end_time =  clock();

	printf("\n--------------- %s ping statistics ---------------\n", argv[1]);
	printf("Packet send: %d \tpacket recv: %d \t %d%%loss\t time: %f\n", packet_check_send, packet_check_recv, (1 - packet_check_recv / packet_check_send) * 100, ((double)(end_time - start_time)) / CLOCKS_PER_SEC);
	close(s);
	exit(EXIT_SUCCESS);
}
