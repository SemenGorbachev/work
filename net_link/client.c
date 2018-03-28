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
	struct sockaddr_in server_addr;
	struct msghdr msg;
	struct iovec vec[1];
	char buf[80], *a;
	char *msg1;

	memset(&msg, 0, sizeof(struct msghdr));
	memset(&vec, 0, sizeof(struct iovec));
	msg1 = (char *)malloc(1024);


	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}

	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);
	server_lenght = sizeof(server_addr);

	while(1) {
		scanf("%s", buf);
		vec[0].iov_base = buf;
		vec[0].iov_len = sizeof(buf);

		msg.msg_name = &server_addr;
		msg.msg_namelen = server_lenght;
		msg.msg_iov = vec;
		msg.msg_iovlen = 1;
		sendmsg(s, &msg, 0);	
		read_bytes = recvmsg(s, &msg, 0);
		if(read_bytes >= 0) { 
			memcpy(buf, msg.msg_iov->iov_base, msg.msg_iov->iov_len);
			printf("%s\n", buf);
		}			
		else perror("Msg not delivered \n");
			
	}
	close(s);
	free(msg1);
	
	exit(EXIT_SUCCESS);
}