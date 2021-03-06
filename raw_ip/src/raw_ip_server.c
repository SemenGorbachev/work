#include "../head/raw_ip.h"

int main(int argc, char **argv) {
	int s, read_bytes, portClient;
	socklen_t client_lenght, serv_lenght;
	struct sockaddr_in serv_addr, client_addr;
	char buf[1024];
	char *some_addr;
	char recv[] = "hi!";

	some_addr = (char *)malloc(6);

	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(EXIT_FAILURE);	
	}

	serv_addr.sin_port = htons(65531);
	serv_addr.sin_family = AF_INET;
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_addr.s_addr = inet_addr (argv[1]);
	
	if (bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	} 
	
	serv_lenght = sizeof(serv_addr);
	if (getsockname(s, (struct sockaddr *)&serv_addr, &serv_lenght) == -1) {
		perror("ERROR on getsockname \n");
	}
	
	some_addr = inet_ntoa(serv_addr.sin_addr);
	printf("%s : %d  \n", some_addr, htons(serv_addr.sin_port));
	
	while(1) {
		client_lenght = sizeof(client_addr);
		read_bytes = recvfrom(s, buf, 1024, 0, (struct sockaddr *)&client_addr, &client_lenght);

		portClient = ntohs(client_addr.sin_port); 
		printf("%d\n", portClient);		
		
		if(read_bytes >= 0) {
			if(strncmp(buf, "hello", 5) == 0) {
				sendto(s, recv, sizeof(recv), 0, (struct sockaddr *)&client_addr, client_lenght); 
			}		
		}
	}
	
	close(s);
	exit(EXIT_SUCCESS);
}
