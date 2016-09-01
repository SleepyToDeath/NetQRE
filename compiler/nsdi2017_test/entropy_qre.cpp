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

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;

int loop_num = 1;


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
  Node_1 default_state;
};

class Node1_0 {
public:
  long sum;
  int state;
};

// per ip
Node_0 state;
//total 
Node1_0 state1;

// entropy
double result = 0;

std::vector<lin_ip> pkt_queue;


inline bool total_update(lin_ip &packet) {
    state1.sum += 1;
  return true;
}

inline bool count_src_update(lin_ip &packet) {
  unsigned long srcip = packet.ip_src.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it_iter1_x;
  Node_1 *node_iter1_leaf;

  it_iter1_x = state.state_map.find(srcip);
  if (it_iter1_x == state.state_map.end()) {
    it_iter1_x = state.state_map.insert({srcip, state.default_state}).first;
  }
  if (true) {
    node_iter1_leaf = &(it_iter1_x->second);

	node_iter1_leaf->sum += 1;
    }
  return true;
}

void update(lin_ip &packet) {
  total_update(packet);
  count_src_update(packet);
}

double frequency(unsigned long ip) {
  return ((double)state.state_map[ip].sum)/state1.sum;
}

void eval() {
  result = 0;
  for (auto it = state.state_map.begin(); it != state.state_map.end(); it++) {
    unsigned long ip = it->first;
    result -= frequency(ip) * log(frequency(ip));
  }
}

static void close() {
  printf("Processed %ld packets. \n", packet_cnt);
  eval();
  printf("entropy is : %f\n", result);
  printf("Exit now.\n");
}

void run() {
    long len = pkt_queue.size();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int j=0; j<loop_num; j++) {
	for (long i=0; i<len; i++) {
	    update(pkt_queue[i]);
	}
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

  struct lin_ip ip_pkt;

  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  memcpy(&ip_pkt, iph, sizeof(lin_ip));

  pkt_queue.push_back(ip_pkt);
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
