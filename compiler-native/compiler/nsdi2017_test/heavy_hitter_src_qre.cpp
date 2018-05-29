#include <unordered_map>
#include "netqre.h"

// y
class Node_1 {
public:
  long sum = 0;
  int state = 0;
};

// x
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  Node_1 node_1_default;
};

Node_0 state;

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it = state.state_map.find(srcIP);
  if (it == state.state_map.end()) { 
    it = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, state.node_1_default)).first;
  } 
  Node_1 *state_1 = &(it->second);

  if (state_1->state == 0) {
    state_1->state = 1;
    state_1->sum += 1;
    state_1->state = 0;
  }
}

void close() {
  printf("Unique srcIP:  %lu\n", state.state_map.size());
  printf("Exit now.\n");
}

