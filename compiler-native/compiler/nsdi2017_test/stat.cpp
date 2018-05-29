#include <unordered_set>
#include "netqre.h"

long ipv4 = 0;
long ipv6 = 0;
long tcp = 0;
long udp = 0;
long unknowl3 = 0;
long unknownl4 = 0;

std::unordered_set<unsigned long> ips;
std::unordered_set<unsigned long> srcips;
std::unordered_set<unsigned long> dstips;

// unique ips among tcp packets
std::unordered_set<unsigned long> tcpsrcips;

std::unordered_set<unsigned short> tcpsrcs;
std::unordered_set<unsigned short> tcpdsts;

std::unordered_set<unsigned short> udpsrcs;
std::unordered_set<unsigned short> udpdsts;

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  if (iph->ip_v == 4) {
    ipv4++;

    if (iph->ip_p == IPPROTO_TCP) {
      tcp++;
      struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + l2offset + iph->ip_hl * 4);
      unsigned short srcPort = tcph->source;
      unsigned short dstPort = tcph->dest;

      tcpsrcs.insert(srcPort);
      tcpdsts.insert(dstPort);

      tcpsrcips.insert(iph->ip_src.s_addr);

    } else if (iph->ip_p == IPPROTO_UDP) {
      udp++;
      struct lin_udphdr* udph = (struct lin_udphdr *) (packet + l2offset + iph->ip_hl * 4);
      unsigned short srcPort = udph->source;
      unsigned short dstPort = udph->dest;

      udpsrcs.insert(srcPort);
      udpdsts.insert(dstPort);
    } else {
      unknownl4++;
    }

    srcips.insert(iph->ip_src.s_addr);
    dstips.insert(iph->ip_dst.s_addr);

    ips.insert(iph->ip_src.s_addr);
    ips.insert(iph->ip_dst.s_addr);

  } else if (iph->ip_v == 6) {
    ipv6++;
  } else {
    unknowl3 ++;
  }
}

void close() {
  printf("ipv4:		    %ld\n",ipv4);
  printf("ipv6:		    %ld\n", ipv6);
  printf("TCP:		    %ld\n", tcp);
  printf("UDP:		    %ld\n",  udp);


  printf("unknown l3:	    %ld\n", unknowl3);
  printf("unknown l4:	    %ld\n", unknownl4);

  printf("unique ips:	    %ld\n", ips.size());
  printf("unique srcips:    %ld\n", srcips.size());
  printf("unique dstips:    %ld\n", dstips.size());

  printf("unique tcp srcips:  %ld\n", tcpsrcips.size());

  printf("unique tcp srcs:  %ld\n", tcpsrcs.size());
  printf("unique tcp dsts:  %ld\n", tcpdsts.size());

  printf("unique udp srcs:  %ld\n", udpsrcs.size());
  printf("unique udp dsts:  %ld\n", udpdsts.size());
}




