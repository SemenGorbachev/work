#include "../head/raw_eth.h"

int main(int argc, char **argv) {
	int s, read_bytes, send_udp;
	socklen_t server_lenght;
	struct sockaddr_ll socket_address;
	struct udphdr udp_h;
	struct iphdr ip_h;
	struct ether_header ether_h;
	struct in_addr ip_addr;

	char inter_name[IFNAMSIZ];
	struct ifreq if_idx, if_mac;

	char buf[6];
	char ip_s[32];
	unsigned char msg1[ETH_HEAD_LEN + IP_HEAD_LEN + UDP_HEAD_LEN + 20];
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