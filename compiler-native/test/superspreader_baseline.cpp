#include "main_process.h"
#include <unistd.h>
#define ETHERNET_LINK_OFFSET 14

void print_stats();

long packet_cnt = 0;
//bool debug_mode = false;

// y
class Node_1 {
public:
  std::unordered_map<unsigned long, int> state_map;
  long sum;
};

// x
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
};

Node_0 state;
Node_1 node_1_default;

long result = 0;

void _update_result(unsigned long srcIP) {
  Node_1 &state_1 = node_1_default;
  if (state.state_map.find(srcIP) != state.state_map.end()) {
    state_1 = state.state_map[srcIP];
  }
  
  result = state_1.sum;
}

void _update_state(u_char *packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it = state.state_map.find(srcIP);

  if (it == state.state_map.end()) { 
    it = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  }
  Node_1 *state_1 = &(it->second);

  if (state_1->state_map.find(dstIP) == state_1->state_map.end()) {
    state_1->state_map[dstIP] = 0;
    state_1->sum += 1;
  }
}

void _output_result() {
  printf("Processed %ld packets. Result is %ld\n", packet_cnt, result);
}

void close() {
  Node_1 * dest_node;
  //158.130.56.11 = 188252830
  //158.130.56.12 = 205030046
  _update_result(188252830);
  _output_result();

  printf("\nRESULT: Super spreader detection app:\n");
  print_stats();

  printf("Exit now.\n");
}

void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  Node_1 * dest_node;
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  _update_state(packet);

  if(packet_cnt % 1 == 0)
  {
      printf("\nStatus update on super spreader app:\n");
      print_stats();
  }
//  _update_result(packet.src, dst);
//  _output_result();
}

/* To print the status/statistics of the app */
void print_stats()
{
  Node_1 * node; 
  printf("Printing Source IP and destination count if count > 1\n");
  for (std::unordered_map<unsigned long, Node_1>::iterator iter1 = state.state_map.begin();
          iter1 != state.state_map.end(); iter1++)
  {
      unsigned long ulSrcIP = iter1->first;
      node = &(iter1->second);
      if(node->sum > 1)
          printf("Source IP '%lu' = %ld\n", ulSrcIP, node->sum);
  }
}
