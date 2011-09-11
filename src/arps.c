/* ccarlton */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <getopt.h>
#include "headers.c"

int main(int argc, char **argv) {
   
	int opt;
   char *target = NULL;
   char *host = NULL;
   char *interface = NULL;
	int arp_type = 0;
	int c_pid = 0;
	int do_fork = 0;

	unsigned char *packet;

	if (argc < 6) {
		print_usage();
	}

	/* Command line arguments */
	while ((opt = getopt(argc, argv, "i:rqft:")) != -1) {
		switch (opt) {
			case 'i':
				interface = optarg;
				break;
			case 'f':
				if ( (arp_type != ARP_REPLY) ) {
					print_usage();
				} else {
					do_fork = 1;
				}
				break;				
			case 'r':
				if (!arp_type) {
					arp_type = ARP_REPLY;
				}else { 
					print_usage();
				}		
				break;
			case 'q':
				if (!arp_type) {
					arp_type = ARP_REQUEST;
				}else {
					print_usage();
				}
				break;
			case 't':
				target = optarg;
				break;
			default:
				break;
		}
	}		

	if (optind < argc) {
		host = argv[optind];
	}

	if(DEBUG) { 
		printf("Host: %s\nTarget: %s\nInterface: %s\nType: %x\nFork: %d\n", host, target, interface, arp_type, do_fork);
	}
	if (!host || !target || !interface || !arp_type) {
		print_usage();
	}

	/* if we must fork, build reply */
	if (do_fork) {
		switch ((c_pid = fork())) {
			case -1: 
				fprintf(stderr, "Fork failed\n");
				exit(EXIT_FAILURE);
				break;
			case 0:
				/* Child process executes and switches host and target */
				if ( (packet = arp_type == ARP_REPLY ? build_reply(interface, host, target) : build_request()) != 0) { 
	   			return send_arp(packet, interface);
				}else {
					exit(EXIT_FAILURE);	
				}
				break;
		}
	}

	/* Parent and single process execute this */	
	if ( (packet = arp_type == ARP_REPLY ? build_reply(interface, target, host) : build_request()) != 0){ 
		return send_arp(packet, interface);
	}else {
	   exit(EXIT_FAILURE);	
	}
}

void print_usage() {
	printf("Usage: arps -i interface -r|q -t target host\n");
	exit(EXIT_FAILURE);
}

unsigned char *build_reply(char *interface, char *target, char *host) {
	
	int sockfd;
	char *strbuf = calloc(256, 1);
	FILE *fp;
	struct ifreq ifr;
   struct eth_header *eth;
	struct arp_header *arp;
	unsigned char *packet = (unsigned char *)calloc(sizeof(struct arp_header) + sizeof(struct eth_header), 1);


   struct sockaddr_in *ip_addr;
   struct in_addr addr;
   char target_mac[18];
   char src_ip[4];
   char *src_ip_str;

	
	/* Create a datagram socket to find interface IP */
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("sockfd");
      exit(EXIT_FAILURE);
   }

   /* Set our ifr struct family to IPV4 */
   ifr.ifr_addr.sa_family = AF_INET;
   /* Copy interface name to ifr_name */
   strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

   /* IOCTL:
      SIOGIFADDR gets an interface addr
      SIOCSIFADDR sets an interface addr
      SIOCGHWADDR gets locak HW addr
    */
   if (ioctl(sockfd, SIOCGIFADDR, (char *)&ifr) <  0) {
      perror("icotl");
      exit(EXIT_SUCCESS);
   }

   ip_addr = (struct sockaddr_in*)&ifr.ifr_addr;
   memcpy(src_ip, &ip_addr->sin_addr, 4);
   src_ip_str = inet_ntoa(ip_addr->sin_addr);

   if (ioctl(sockfd, SIOCGIFHWADDR, (char *)&ifr) < 0) {
      perror("ioctl");
      exit(EXIT_SUCCESS);
   }

	close(sockfd);

	send_ping(target);

	/* Get info from /proc/net/arp */
   if ((fp = fopen("/proc/net/arp", "r")) == 0) {
      fprintf(stderr, "Error opening /proc/net/arp");
      exit(EXIT_FAILURE);
   }

   /* Search for the arp entry containing our target ip */
	strcat(target, " ");
	while (fgets(strbuf, 256, fp) != NULL) {
		if (strstr(strbuf, target) != 0) {
         /* Parse out fields */
         strncpy(target_mac, strstr(strbuf, ":")-2, 18);
			break;
      }
   }
	target[strlen(target)-1] = 0;
	
	if(!inet_pton(AF_INET, target, &addr)) {
      fprintf(stderr, "Error converting IP Address\n");
      exit(EXIT_FAILURE);
   }

	eth = (struct eth_header *)packet;
	arp = (struct arp_header *)(packet + sizeof(struct eth_header));


   /*
   memcpy(args->dip,  &addr.s_addr,  4);
   memcpy(args->sip,  src_ip, 4);
   memcpy(args->dmac, formatConvertMac(target_mac), 6);
   memcpy(args->smac, ifr.ifr_hwaddr.sa_data, 6);
	*/

	if(DEBUG) {
      printf("Target Mac: %.14s\n", target_mac);
      printf("Source Mac: ");
      print_mac((unsigned char *)ifr.ifr_hwaddr.sa_data);
      printf("Network address: %s\n", src_ip_str);
      printf("Target address: %s\n\n", target);
   }

	return 0;
}

int send_ping(char *target) {
	char *strbuf = (char *)calloc(256,1);

	/* Send Ping/Subnet check */
   strcpy(strbuf, "ping -c 1 ");
   strcat(strbuf, target);
   strcat(strbuf, " > /dev/null");

   if (system(strbuf) != 0) {
      fprintf(stderr, "Error executing ping system call\n");
      exit(EXIT_FAILURE);
   }
	
	return 0;

}

unsigned char *build_request() {
	return 0;
}

int send_arp(unsigned char *packet, char *interface) {

	struct sockaddr sa;
	int sockfd;
	   /* Raw socket to send */
   if ((sockfd = socket(AF_INET, SOCK_PACKET, htons(ETH_P_RARP))) < 0) {
      perror("socket");
      exit(EXIT_FAILURE);
   }

   if (DEBUG) {
      print_buf((unsigned char *)packet, sizeof(struct eth_header) + sizeof(struct arp_header));
   }

   strcpy(sa.sa_data, interface);
   if(sendto(sockfd, packet, sizeof(struct eth_header) + sizeof(struct arp_header), 0, &sa, sizeof(sa)) < 0) {
      perror("Error sending packet");
      exit(EXIT_FAILURE);
   }

		
	return 0;
}



char *formatConvertMac(char *asciiMac) {
   int i, j;
   char *mac = (char*)calloc(6, 1);
   char *subString = (char*)calloc(13,1);
   unsigned int byte = 0;

   /* Format */
   for(i=0; i<strlen(asciiMac); i++) {
      if (asciiMac[i] != 58) {
         memcpy(&subString[strlen(subString)], &asciiMac[i], 1);
      }
   }

   /* Convert */
   for (i=0; i<=12; i+=2) {
      sscanf(subString+i, "%2x", &byte);
      j = i == 0 ? 0 : i-(i/2);
      mac[j] = byte;
   }

   return mac;
}

void print_mac(unsigned char *macPtr) {
   int i;

   for (i=0; i<6; i++) {
      if (i<5) {
         printf("%x:", macPtr[i]);
      }else
         printf("%x\n", macPtr[i]);
   }
}


void print_buf(unsigned char *packet, uint32_t size) {
   int ctr;

   printf("Packet: \n");
   for (ctr=0; ctr<size; ctr++) {
      printf("%x ", packet[ctr]);
   }
   printf("\n");
}


