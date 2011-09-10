/* arps
	Caleb Carlton */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <netinet/in.h>
#include <getopt.h>
#include "headers.c"

int main(int argc, char **argv) {
   
	int opt;
   char *target = NULL;
   char *host = NULL;
   char *interface = NULL;
	int arp_type = 0;

   struct arg_struct *arpArgs;

	if(argc < 6) {
		print_usage();
	}

	/* Command line arguments */
	while ((opt = getopt(argc, argv, "i:r:q:t")) != -1) {
		switch (opt) {
			case 'i':
				interface = optarg;
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

	if (!host || !target || !interface || !arp_type) {
		print_usage();
	}
	
}

void print_usage() {
	printf("Usage: arps -i interface -r|q -t target host\n");
	exit(EXIT_FAILURE);
}

unsigned char *build_reply() {
}

unsigned char *build_request() {
}

unsigned char *send_arp() {
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


void print_buf(u_char *packet, uint32_t size) {
   int ctr;

   printf("Packet: \n");
   for (ctr=0; ctr<size; ctr++) {
      printf("%x ", packet[ctr]);
   }
   printf("\n");
}


