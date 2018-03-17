#include "../head/raw_ip.h"

void print_ip(int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    printf("ip: %d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);        
}


unsigned short csum(unsigned short *ptr, int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    unsigned short answer;
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
    return(answer);
}

int sendIPUDP(int s, struct udphdr udph, struct iphdr iph, char * data, int data_lenght, struct sockaddr_in serv, socklen_t serv_lenght){
	unsigned char * buffer;
	char * ip_h_check;
	int res;
	unsigned char h_lenght_udp;
	unsigned char h_lenght_ip;
	unsigned int pac_lenght;

	h_lenght_udp = sizeof(udph);
	h_lenght_ip = sizeof(iph);
	pac_lenght = h_lenght_udp + data_lenght;
	udph.check = 0;
	if(!udph.len) udph.len = htons(pac_lenght);

	pac_lenght = h_lenght_ip + h_lenght_udp + data_lenght;
	if(!iph.tot_len) iph.tot_len = htons(pac_lenght);

	buffer = (unsigned char *)calloc(pac_lenght, sizeof(char));
	ip_h_check = (char *)malloc(h_lenght_ip);

	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(ip_h_check, &iph, h_lenght_ip);
	iph.check = csum((unsigned short *) ip_h_check, IP_HEAD_LEN);
	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(buffer + h_lenght_ip, &udph, h_lenght_udp);

	if(data) memcpy(buffer + h_lenght_udp + h_lenght_ip, data, data_lenght);
	for(int i = 0; i < pac_lenght; i++)
		printf("%x ", buffer[i]);

	printf("\n");
	res = sendto(s, buffer, pac_lenght, 0, (struct sockaddr *) &serv, serv_lenght); 

	return res;
} 