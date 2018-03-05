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



int sendUDP(int s, struct udphdr udph, char * data, int data_lenght, struct sockaddr_in serv, socklen_t serv_lenght){
	char * buffer;
	int res;
	unsigned char h_lenght;
	unsigned int pac_lenght;

	h_lenght = sizeof(udph);
	pac_lenght = h_lenght + data_lenght;
	udph.check = 0;

	if(!udph.len) udph.len = htons(pac_lenght);

	buffer = (char *)calloc(pac_lenght, sizeof(char));
	memcpy(buffer, &udph, h_lenght);

	if(data) memcpy(buffer + h_lenght, data, data_lenght);

	res = sendto(s, buffer, pac_lenght, 0, (struct sockaddr *) &serv, serv_lenght); 
	return res;


} 

int main(int argc, char **argv) {
	int s, read_bytes, send_udp, str_len, str_len_r_buf;
	socklen_t client_lenght, server_lenght;
	struct sockaddr_in client_addr, server_addr;
	struct udphdr udp_h;
	char buf[6], *a;
	unsigned char msg1[50];
	char msg2[] = "Hello!";
	unsigned short port = 0;
	unsigned char port1[2];

	s = socket (AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}

	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_addr.s_addr = inet_addr (argv[2]);
	
	client_lenght = sizeof(client_addr);
	server_lenght = sizeof(server_addr);	

	udp_h.source = htons(65533);
	udp_h.dest = htons(atoi(argv[1]));
	udp_h.len = 0;
	udp_h.check = 0;


	while(1) {
		scanf("%s", buf);
		send_udp = sendUDP(s, udp_h, buf, sizeof(buf), server_addr, server_lenght);

		while(1) {
			read_bytes = recvfrom(s, &msg1, 1024, 0, (struct sockaddr *)&server_addr, &server_lenght);
			printf("read_bytes: %d\n", read_bytes);
			
			//strncpy(port1, msg1 + 22, 2);
			port = msg1[22] <<8|msg1[23];
			printf("%d\n",port );
			printf("%s\n",port1 );

			//strncpy(&port, msg1 + 22, 2);
			//strncpy(port1, msg1 + 22, 2);
			if(read_bytes >= 0)
				if(port == ntohs(udp_h.source))
				//if(port == 65533)
				//if(msg1[22]==0xff) 
					//if(msg1[23]==0xfd) 
						printf("%s\n", msg1 + 28);		
		}
		
				
	}
	close(s);
	free(msg1);
	
	exit(EXIT_SUCCESS);
}