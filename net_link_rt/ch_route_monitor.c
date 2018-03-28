#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/rtnetlink.h>
#include <errno.h>
#include <memory.h>



int main(int argc, char **argv) {
	int s, b, read_bytes, msg_size, check, count_c = 1, in;
	socklen_t client_lenght, server_lenght;
	struct sockaddr_in server_addr;
	struct sockaddr_nl kernel, me;
	struct msghdr msg;
	struct nlmsghdr *nlmsg;
	struct iovec vec;
	char buf[8192];

	memset(&msg, 0, sizeof(struct msghdr));
	memset(&vec, 0, sizeof(struct iovec));
	memset(&nlmsg, 0, sizeof(struct nlmsghdr));
	memset(&kernel, 0, sizeof(struct sockaddr_nl));
	memset(&me, 0, sizeof(struct sockaddr_nl));


	s = socket (AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (s < 0) {
		perror("Couldn't creat socket \n");
		exit(EXIT_FAILURE);	
	}

	me.nl_family = AF_NETLINK;
	me.nl_groups = RTMGRP_IPV4_ROUTE;

	b = bind(s, (struct sockaddr *)&me, sizeof(me));
	if(b < 0) {
		perror("Couldn't bind socket \n");
		exit(EXIT_FAILURE);	
	}
	
	vec.iov_base = buf;
	vec.iov_len = sizeof(buf);

	msg.msg_name = &me;
	msg.msg_namelen = sizeof(me);
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;



	while(1) {
		read_bytes = recvmsg(s, &msg, 0);
		if(read_bytes < 0) { 
			perror("Msg not delivered \n");
		}			
		nlmsg = (struct nlmsghdr*) buf;
		if(nlmsg->nlmsg_type == RTM_NEWROUTE || nlmsg->nlmsg_type == RTM_DELROUTE) {
			printf("Сhanges to the routing table\n");
		}
	}
	close(s);
	
	exit(EXIT_SUCCESS);
}