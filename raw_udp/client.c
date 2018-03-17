#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char **argv) {
	int s, read_bytes, msg_size, check, count_c = 1, in;
	socklen_t client_lenght, server_lenght;
	struct sockaddr_in client_addr, server_addr;
	char buf[80], *a;
	char *msg1;
	char msg2[] = "Hello!";

	msg1 = (char *)malloc(1024);
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}

	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);
	
	client_lenght = sizeof(client_addr);
	server_lenght = sizeof(server_addr);	
	
	
	while(1) {
		scanf("%s", buf);
		sendto(s, buf, sizeof(buf), 0, (struct sockaddr *) &server_addr, server_lenght);
		
		while(1) {		
			read_bytes = recvfrom(s, msg1, 1024, 0, (struct sockaddr *)&server_addr, &server_lenght);

			if(read_bytes >= 0) printf("%s\n", msg1);			
			else perror("Msg not delivered \n");
	
		}		
	}
	close(s);
	free(msg1);
	
	exit(EXIT_SUCCESS);
}