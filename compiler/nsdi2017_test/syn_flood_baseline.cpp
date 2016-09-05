#include <sys/time.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include "linux_compat.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

using namespace std;

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int loop_num = 1;

std::unordered_map<string, int> stateMap;

std::vector<u_char*> pkt_queue;

void update(u_char * packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP)
    return;

  struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + ETHERNET_LINK_OFFSET + iph->ip_hl * 4);
  unsigned short srcPort = tcph->source;
  unsigned short dstPort = tcph->dest;

  // SYN
  if (tcph->syn == 1 && tcph->ack == 0) {
 std::string key = std::to_string(srcIP) 
	+ std::to_string(srcPort) + std::to_string(dstIP)
	+ std::to_string(dstPort);

    std::unordered_map<string, int>::iterator it1 = stateMap.find(key);
    if (it1 == stateMap.end()) {
      it1 = stateMap.insert(std::pair<string, int>(key, 1)).first;
    } 

    it1->second = 1;
  }

  // SYN-ACK
  if (tcph->syn == 1 && tcph->ack == 1) {
    string key =  (std::to_string(dstIP))
	+ (std::to_string(dstPort)) + (std::to_string(srcIP) )
	+ (std::to_string(srcPort));

    std::unordered_map<string, int>::iterator it1 = stateMap.find(key);
    if (it1 != stateMap.end()) {
      if (it1->second == 1)
	it1->second == 2;
    } 
  }

  // ACK
  if (tcph->syn == 0 && tcph->ack == 1) {
    string key = std::to_string(srcIP)
	+ std::to_string(srcPort) + std::to_string(dstIP)
	+ std::to_string(dstPort);

    std::unordered_map<string, int>::iterator it1 = stateMap.find(key);
    if (it1 != stateMap.end()) {
      if (it1->second == 2)
	it1->second == 0;
    } 
  }
}

static void close() {
  printf("Processed %ld packets. \n", packet_cnt);
  //printf("34435 : %lu\n 3014787072 : %lu\n", stateMap[34435], stateMap[3014787072]);
  printf("Unique srcIP:  %lu\n", stateMap.size());
  printf("Exit now.\n");
}



void run() {
  printf("Starting ...\n");
  
  long len = pkt_queue.size();

  struct timeval start, end;
  gettimeofday(&start, NULL);

  for (int j=0; j<loop_num; j++)
    for (long i=0; i<len; i++) {
      update(pkt_queue[i]);
    }

  gettimeofday(&end, NULL);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
    - (start.tv_sec * 1000000 + start.tv_usec);

  double per_packet_time = (double)(time_spent)/(len);

  printf("Runtime: %ld seconds, processes %ld packets. Each packet takes %f us.\n",
      time_spent, len, per_packet_time);
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }

  u_char* pkt = (u_char *)malloc(header->caplen);
  memcpy(pkt, packet, header->caplen);

  pkt_queue.push_back(pkt);
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

      if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
        fprintf(stderr, "pcap_loop exited with error.\n");
        exit(1);
      }

      run();
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

  close();

  /* And close the session */
  pcap_close(handle);

  return(0);
}

