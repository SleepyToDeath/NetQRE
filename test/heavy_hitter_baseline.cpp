#include <unistd.h>
#include "main_process.h"
#define ETHERNET_LINK_OFFSET 14

void print_stats();

long packet_cnt = 0;
//bool debug_mode = false;

class Node_1 {
public:
  std::unordered_map<unsigned long, long> state_map;
};

std::unordered_map<unsigned long, Node_1> state;
Node_1 node_1_default;

long result;

void _update_result(unsigned long srcIP, unsigned long dstIP) {
  result = state[srcIP].state_map[dstIP];
}

void _update_state(u_char *packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;
  std::unordered_map<unsigned long, Node_1>::iterator it = state.find(srcIP);
  Node_1 *state_1 = NULL;

  if (it == state.end()) { 
    it = state.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  }
  state_1 = &(it->second);

  std::unordered_map<unsigned long, long>::iterator it1 = state_1->state_map.find(dstIP);
  if (it1 == state_1->state_map.end()) { 
    state_1->state_map[dstIP] = ntohs(iph->ip_len);
  } else {
    it1->second += ntohs(iph->ip_len);
  }
}

void _output_result() {
  printf("Processed %ld packets. Result is %ld\n", packet_cnt, result);
}


void close() {
  Node_1 * dest_node;

  //158.130.56.11 = 188252830
  //158.130.56.12 = 205030046
  _update_result(188252830, 205030046);
  _output_result();

  printf("\nRESULT: Heavy hitter app:\n");
  print_stats();
  printf("Exit now.\n");
}

void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  
  packet_cnt += 10;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  _update_state(packet);

  if(packet_cnt % 1 == 0)
  {
      printf("\nStatus update on heavy hitter app:\n");
      print_stats();
  }
}

/* To print the statistics/ status of the app */
void print_stats()
{
  Node_1 * dest_node;
  printf("Bandwidth consumption of Source IP if consumption > 10000\n");
  for (std::unordered_map<unsigned long, Node_1>::iterator iter1 = state.begin();
          iter1 != state.end(); iter1++)
  {
      unsigned long ulSrcIP = iter1->first;
      long long llBandwidth = 0;
      dest_node = &(iter1->second);
      std::unordered_map<unsigned long, long> dest_map = dest_node->state_map;

      for(std::unordered_map<unsigned long, long>::iterator iter2 = dest_map.begin();
              iter2 != dest_map.end(); iter2++)
      {
          unsigned long ulDestIP = iter2->first;
          llBandwidth += iter2->second;
      }
      if(llBandwidth > 10000)
          printf("Source IP '%lu' = %lld\n", ulSrcIP, llBandwidth);
  }
}
