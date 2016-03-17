#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux_compat.h>
#include <unordered_map>

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;

// leaf level
class Node_4 {
public:
  int sum;
  int state_1;
};

// dstPort
class Node_3 {
public:
  std::unordered_map<unsigned short, Node_4> state_map;
  Node_4 default_state;
};
// srcPort
class Node_2 {
public:
  std::unordered_map<unsigned short, Node_3> state_map;
  Node_3 default_state;
};
// dstIP
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
  Node_2 default_state;
};

// srcIP
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  unsigned int total_sum;
  Node_1 default_state;
};

Node_0 state;


void _update_state(u_char *packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP)
    return;

  struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + ETHERNET_LINK_OFFSET + iph->ip_hl * 4);
  unsigned short srcPort = tcph->source;
  unsigned short dstPort = tcph->dest;

  std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(srcIP);
  if (it1 == state.state_map.end()) {
    it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, state.default_state)).first;
  } 
  Node_1 *state_1 = &(it1->second);

  std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
  if (it2 == state_1->state_map.end()) { 
    it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, state_1->default_state)).first;
  }
  Node_2 *state_2 = &(it2->second);

  std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(srcPort);
  if (it3 == state_2->state_map.end()) { 
    it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(srcPort, state_2->default_state)).first;
  }
  Node_3 *state_3 = &(it3->second);

  std::unordered_map<unsigned short, Node_4>::iterator it4 = state_3->state_map.find(dstPort);
  if (it4 == state_3->state_map.end()) { 
    it4 = state_3->state_map.insert(std::pair<unsigned short, Node_4>(dstPort, state_3->default_state)).first;
  }
  Node_4 *state_4 = &(it4->second);

  if (tcph->fin == 1 && state_4->state_1 == 1) {
    state_4->sum += 1;
    state.total_sum += 1;
    state_4->state_1 = 0;
  }

  if (tcph->syn == 1 && state_4->state_1 == 0) {
    state_4->state_1 = 1;
  }
}

static void close() {
  //158.130.56.11 = 188252830
  //158.130.56.12 = 205030046
  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 100 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  _update_state(packet);
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
  int loop_num = 1;
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
    for (int i=0; i<loop_num; i++) {
      handle = pcap_open_offline(argv[2], errbuf);
      if (handle == NULL) {
        fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
        return(1);
      }

      if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
        fprintf(stderr, "pcap_loop exited with error.\n");
        exit(1);
      }
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

  close();

  /* And close the session */
  pcap_close(handle);

  return(0);
}
