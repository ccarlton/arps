#define RECV         0
#define SEND         1
#define NUM_THREADS  2
#define ETH_P_RARP   0x8035
#define LEN_ARP      14
#define LEN_ETH      28
#define ETH_TYPE     0x0806
#define ARP_REQUEST  0x0001
#define ARP_REPLY		0x0002

void print_usage();
unsigned char *build_request(); 
unsigned char *build_reply(); 
int send_arp(unsigned char *packet);

struct eth_header {
	uint16_t dmac;
	uint16_t smac;
	unsigned char type[2];
};

struct arp_header {
	uint16_t htype;
	uint16_t ptype;
	unsigned char hlen;
	unsigned char plen;
	uint16_t oper;
	uint32_t smac;
	uint16_t smacx;
	uint32_t sip;
	uint32_t dmac;
	uint16_t dmacx;
	uint32_t dip;
};
