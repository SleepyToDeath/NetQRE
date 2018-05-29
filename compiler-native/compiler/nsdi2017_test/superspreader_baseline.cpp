#include <unordered_map>
#include <unordered_set>
#include "netqre.h"

class Node_1 {
public:
  std::unordered_set<unsigned long> IPset;
  long sum = 0;
};

Node_1 node_1_default;

std::unordered_map<unsigned long, Node_1> state;

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it = state.find(srcIP);

  if (it == state.end()) { 
    it = state.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  }
  Node_1 *state_1 = &(it->second);

  if (state_1->IPset.find(dstIP) == state_1->IPset.end()) { 
    state_1->IPset.insert(dstIP);
    state_1->sum += 1;
  }
}

void close() {
  printf("Unique srcIP:  %lu\n", state.size());
  printf("Exit now.\n");
}
