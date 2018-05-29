#include <unordered_map>
#include "netqre.h"

// leaf level
class Node_4 {
public:
  int state;
  int sum;
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
  Node_1 default_state;
};

Node_0 state;

void update(u_char * packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP) {
    return;
  }

  struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + l2offset + iph->ip_hl * 4);
  unsigned short srcPort = tcph->source;
  unsigned short dstPort = tcph->dest;

  // SYN
  if (tcph->syn == 1) {
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

    state_4->state = 1;
  }

  // FIN
  if (tcph->fin == 1) {
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

    if (state_4->state == 1) {
      state_4->sum++;
      state_4->state = 0;
    }
  }
}

void close() {
  printf("Unique srcIP:  %lu\n", state.state_map.size());
}
