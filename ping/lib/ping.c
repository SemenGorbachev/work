#include "../head/ping.h"

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

int sendIPICMP(int s, struct icmphdr icmph, struct iphdr iph, struct sockaddr_in serv, socklen_t serv_lenght){
	unsigned char * buffer;
	char * ip_h_check;
	char * icmp_h_check;
	int res;
	unsigned char h_lenght_icmp;
	unsigned char h_lenght_ip;
	unsigned int pac_lenght;

	h_lenght_icmp = sizeof(icmph);
	h_lenght_ip = sizeof(iph);
	icmph.checksum = 0;

	pac_lenght = h_lenght_ip + h_lenght_icmp;
	if(!iph.tot_len) iph.tot_len = htons(pac_lenght);

	buffer = (unsigned char *)calloc(pac_lenght, sizeof(char));
	ip_h_check = (char *)malloc(h_lenght_ip);
	icmp_h_check = (char *)malloc(h_lenght_icmp);

	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(ip_h_check, &iph, h_lenght_ip);
	memcpy(icmp_h_check, &icmph, h_lenght_icmp);
	iph.check = csum((unsigned short *) ip_h_check, IP_HEAD_LEN);
	icmph.checksum = csum((unsigned short *) icmp_h_check, ICMP_HEAD_LEN);

	memcpy(buffer, &iph, h_lenght_ip);
	memcpy(buffer + h_lenght_ip, &icmph, h_lenght_icmp);
	
	res = sendto(s, buffer, pac_lenght, 0, (struct sockaddr *) &serv, serv_lenght); 

	return res;
} 
