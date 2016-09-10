#include <unordered_map>
#include <math.h>
#include "netqre.h"

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

inline bool total_update(lin_ip *packet) {
  state1.sum += 1;
  return true;
}

inline bool count_src_update(lin_ip *packet) {
  unsigned long srcip = packet->ip_src.s_addr;

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

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  total_update(iph);
  count_src_update(iph);
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

void close() {
  eval();
  printf("entropy is : %f\n", result);
  printf("Exit now.\n");
}

