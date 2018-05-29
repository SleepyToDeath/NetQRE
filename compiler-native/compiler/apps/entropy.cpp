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

/*
 * offline computation for sum
 * */
// leaf level
class Node_1 {
public:
  long sum;
  int state;
};

class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  long sum;
  int state;
};

Node_0 state;
Node_1 node_1_default;

// entropy
double result = 0;

long total() {
  return state.sum;
}

long count_src(unsigned long ip) {
  return state.state_map[ip].sum;
}

double frequency(unsigned long ip) {
  return ((double)count_src(ip))/total();
}

void _update_result() {
  result = 0;
  for (auto it = state.state_map.begin(); it != state.state_map.end(); it++) {
    unsigned long ip = it->first;
    result -= frequency(ip) * log(frequency(ip));
  }
}

void total_update(u_char *packet) {
  if (state.state == 0) {
    state.state = 1;
    state.sum += 1;
    state.state = 0;
  }
}

void count_src_update(u_char *packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it = state.state_map.find(srcIP);
  if (it == state.state_map.end()) { 
    it = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  }
  Node_1 *state_1 = &(it->second);

  if (state_1->state == 0) {
    state_1->state = 1;
    state_1->sum += 1;
    state_1->state = 0;
  }
}

void _update_state(u_char *packet) {
  count_src_update(packet);
  total_update(packet);
}

void _output_result() {
  printf("Processed %ld packets. Result is %lf\n", packet_cnt, result);
}

static void close() {
  _update_result();
  _output_result();
  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
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
