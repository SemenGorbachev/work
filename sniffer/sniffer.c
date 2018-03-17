#include <pcap.h>             
#include <stdio.h>       
#include <stdlib.h>         
#include <errno.h>                
#include <sys/socket.h>               
#include <sys/types.h>                
#include <netinet/in.h>               
#include <arpa/inet.h>                
#include <netinet/if_ether.h>         
#include <net/ethernet.h>         
#include <netinet/tcp.h>   
#include <netinet/udp.h>          
#include <ctype.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <string.h>

//Вывод заголовка канального уровная
u_short handle_ethernet(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet){
	struct ether_header *ethernet;
	ethernet = (struct ether_header*)packet;
	printf("LINK LAYER:\n");
	printf("\tMAC destination: %s\tMAC source: %s", ether_ntoa(ethernet->ether_dhost), ether_ntoa(ethernet->ether_shost));
	printf("\n\tType of overlying protocol: ");
	return ntohs(ethernet->ether_type);
}

//Вывод заголовка транспортного уровня, если протокол TCP
void handle_tcp(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet){
	struct tcphdr* tcp;

	printf("\nTRANSPORT LAYER:\n");
	int iplen = sizeof(struct ether_header) + sizeof(struct ip);
	int tcplen = iplen + sizeof(struct tcphdr);
	printf("\tSource port: %d  ", ntohs(tcp->th_dport));
	printf("Destination port: %d\n", ntohs(tcp->th_sport));
}

//Вывод заголовка транспортного уровня, если протокол UDP
void handle_udp(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet){
	struct udphdr* udp;

	printf("\nTRANSPORT LAYER:\n");
	int iplen = sizeof(struct ether_header) + sizeof(struct ip);
	int udplen = iplen + sizeof(struct udphdr);
	printf("\tSource port: %d  ", ntohs(udp->uh_sport));
	printf("Destination port: %d\n", ntohs(udp->uh_dport));
}

//Вывод заголовка сетевого уровня, если протокол IP
void handle_ip(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet){
	struct ip *ip_p;

	printf("\nINTERNET LAYER:\n");
	ip_p = (struct ip *)(packet + sizeof(struct ether_header));
	printf("\tSource address: %s", inet_ntoa(ip_p->ip_src));
	printf("\tDestination address: %s\n", inet_ntoa(ip_p->ip_dst));
	printf("\tTOS:%d  ", ip_p->ip_tos);
	printf("Lenght:%d  ", ip_p->ip_len);
	printf("ID:%d  ", ip_p->ip_id);
	printf("CheckSum:%d  ", ip_p->ip_sum);
	printf("TTL:%d ", ip_p->ip_ttl);
	printf("\n\tType of overlying protocol: ");

	if(ip_p->ip_p == IPPROTO_TCP) {
		printf("(TCP)\n");
		handle_tcp(args, pkthdr, packet);
	}
	else if (ip_p->ip_p == IPPROTO_UDP) {
		printf("(UDP)\n");
		handle_udp(args, pkthdr, packet);
	}
	else if (ip_p->ip_p == IPPROTO_ICMP) {
		printf("(ICMP)\n");
	}
	else {
		printf("(UNKNOWN)\n");
	}

}

//Вывод информации о пакете в HEX и в символах ASCII(если символ непечатаемый - заменяется на ".")
void print_mes(const struct pcap_pkthdr* pkthdr, const u_char* packet){

	int line_width = 16;
	int len = pkthdr->len;

	printf("\nPAYLOAD (%d bytes):\n\n", pkthdr->len);

	for(int i=0; i<len; i+=line_width) {
	    
	    for(int j=0; j<line_width; j++) {
	    	if(i + j >= len) printf("   ");
	    	else printf("%02X ", packet[i+j]);
	    }
	    printf("\t");


	    for(int j=0; j<line_width; j++) { 
	    	if(i+j>=len) break;
			if(isprint(packet[i+j]))            
				printf("%c",packet[i+j]);       
			else 
				printf(".");       
		}
		printf("\n");
	}
	printf("\n*******************************************************************************");
}

//Поиск введенных пользотелем данных в пакетах
int is_found(u_char *cnt, int c_len, u_char *data, int d_len) {
	int j = 0;
	
	if(d_len > c_len || cnt == NULL || data == NULL)
		return 0;
	
	for(int i = 0; i < c_len; ++i)
	{
		if(c_len - i < d_len - j)
			return 0;
			
		if(cnt[i] == data[j])
			++j;
		else
		{
			i -= j;
			j = 0;
		}
		if(j == d_len)
			return 1;
	}
	
	return 0;
}

//Обработчик полученных сообщений
void callback(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet) {	
	printf("\n*******************************************************************************\n");
	u_int16_t etype = handle_ethernet(args, pkthdr, packet);
	if(etype == ETHERTYPE_IP) {
		printf("(IP)\n");
		handle_ip(args, pkthdr, packet);
	}
	else if(etype == ETHERTYPE_ARP)
		printf("(ARP)\n");
	else if(etype == ETHERTYPE_REVARP)
		printf("(RARP)\n");
	else 
		printf("(UNKNOWN)\n");

//Если нет введенных пользователем подстроки или нашлось то, что введенно пользователем
	if(args == NULL || is_found(packet, pkthdr->len, args, strlen((char *)args)) == 1){
		print_mes(pkthdr, packet);
	}
}



int main(int argc, char **argv) {
	int pcapComp = 0, pcapSetFil = 0, pcapLoop = 0;
	char *somedev, errbuf[PCAP_ERRBUF_SIZE];
	char *filter, *datafind; 
	bpf_u_int32 net, mask;
	pcap_t *handle;
	struct in_addr addr;
	struct bpf_program fp;
	struct pcap_pkthdr *header;
	const u_char *packet;

//Обработка введенных аргументов(если введенно некоректно)
//Ввод дополнительных аргументов - не обязательно
// -f указывает на фильтр. Фильтр должен быть введен в соответсвии со стандартов pcap
// -i указывает на каком интерфейсе будет осуществляться перехваты пакетов
// -df указывает какие данные надо искать в пакете

	if(argc % 2 == 0){
		printf("Usage: sudo ./sniffer [-f \"filter\"] [-i interface] [-df \"datafind\"]\n");
		exit(1);
	}

	filter = datafind = somedev = NULL;
	for(int i = 1; i < argc; i += 2) {
		if(strncmp(argv[i], "-f", 2) == 0){
			filter = argv[i + 1];
		}
		if(strncmp(argv[i], "-i", 2) == 0){
			somedev = argv[i + 1];
		}
		if(strncmp(argv[i], "-df", 3) == 0){
			datafind = argv[i + 1];
		}
	}



	if(somedev == NULL){
		somedev = pcap_lookupdev(errbuf);		//поиск прослушающегося устройства
		
		if(somedev == NULL){
			printf("pcap_lookupdev %s\n", errbuf);
			exit(1);
		}
	}
	pcap_lookupnet(somedev, &net, &mask, errbuf);		//запрашивает сетевую карту о классе сети и маске
	handle = pcap_open_live(somedev, BUFSIZ, 1, 0, errbuf);			//функция создает сессию для прослушивания и возвращает ее идентификатор
	if(handle == NULL){
		printf("pcap_open_live %s\n", errbuf);
		exit(1);
	}
	pcapComp = pcap_compile(handle, &fp, filter, 0, net);		//компилирует фильтр отбора пакетов из текстового вида, во внутреннее представление
	if(pcapComp == -1){
		printf("pcap_compile\n");
		exit(1);
	}
	pcapSetFil =  pcap_setfilter(handle, &fp);			//после того как фильтр был скомпилирован применяем его
	if(pcapSetFil == -1){
		printf("pcap_setfilter\n");
		exit(1);
	}

	printf("DEV: %s\n", somedev);
	addr.s_addr=net;
	printf("NET: %s ", inet_ntoa(addr));
	addr.s_addr=mask;
	printf("MASK: %s\n", inet_ntoa(addr));
	
	pcapLoop = pcap_loop(handle, -1, callback, (u_char *)datafind);			//организация цикла для отлова пакетов
	if(pcapLoop < 0) {
		printf("pcap_loop\n");
		exit(1);
	}
	pcap_close(handle);


	return 0;
}