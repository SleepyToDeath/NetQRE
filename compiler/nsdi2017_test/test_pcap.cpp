#include <sys/time.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include "linux_compat.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int loop_num = 1;
int offset = 12;

long ipv4 = 0;
long ipv6 = 0;
long tcp = 0;
long udp = 0;
long unknowl3 = 0;

std::unordered_set<unsigned long> ips;
std::unordered_set<unsigned short> tcpsrcs;

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  struct lin_ip* iph = (struct lin_ip*) (packet + offset);
  if (iph->ip_v == 4) {
    ipv4++;

    if (iph->ip_p == IPPROTO_TCP) {
      tcp++;
      struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + offset + iph->ip_hl * 4);
      unsigned short srcPort = tcph->source;
      unsigned short dstPort = tcph->dest;

      tcpsrcs.insert(srcPort);

    } else if (iph->ip_p == IPPROTO_UDP) {
      udp++;
    }

    ips.insert(iph->ip_src.s_addr);
    ips.insert(iph->ip_dst.s_addr);

  } else if (iph->ip_v == 6) {
    ipv6++;
  } else {
    unknowl3 ++;
  }
}

void close() {
  printf("ipv4: %ld\n ipv6: %ld\n TCP: %ld\n", ipv4, ipv6, tcp);
  printf("unknown: %ld\n", unknowl3);
  printf("unique ips: %ld\n", ips.size());
  printf("unique tcp srcs: %ld\n", tcpsrcs.size());
}

int main(int argc, char *argv[]) {
  pcap_t *handle;			/* Session handle */
  char *dev;			/* The device to sniff on */
  char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
  struct bpf_program fp;		/* The compiled filter */
  bpf_u_int32 mask;		/* Our netmask */
  bpf_u_int32 net;		/* Our IP */
  struct pcap_pkthdr header;	/* The header that pcap gives us */
  const u_char *packet;		/* The actual packet */
  bool is_offline = true;

  if (argc < 3) {
    printf("Usage: ./main mode path_to_file [loop_num] [debug_mode]\n");
    return 0;
  }

  if (argc >= 4) {
    loop_num = atoi(argv[3]);
  }

  if (argc >= 5) {
    debug_mode = strcmp(argv[4], "debug") == 0;
  }

  if (strcmp(argv[1], "offline") == 0) {
    is_offline = true;
  } else if (strcmp(argv[1], "live") == 0) {
    is_offline = false;
  } else {
    printf("Mode %s is not recognized\n", argv[1]);
    return 0;
  }

  if (is_offline) {
      handle = pcap_open_offline(argv[2], errbuf);
      if (handle == NULL) {
        fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
        return(1);
      }

      printf("Data link type: %d\n", pcap_datalink(handle));

      if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
        fprintf(stderr, "pcap_loop exited with error.\n");
        exit(1);
      }
  } else {
     handle = pcap_open_live(argv[2], 65535, 1, 10, errbuf);
     if (handle == NULL) {
       fprintf(stderr, "Couldn't open device %s: %s\n", argv[2], errbuf);
       return(0);
     }
     if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
       fprintf(stderr, "pcap_loop exited with error.\n");
       exit(1);
     }
  }


  /* And close the session */
  pcap_close(handle);

  close();

  return(0);
}

