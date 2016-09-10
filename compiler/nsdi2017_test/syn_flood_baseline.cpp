#include <unordered_map>
#include "netqre.h"

// srcPort
class Node_3 {
public:
  std::unordered_map<unsigned short, int> state_map;
};

// dstPort
class Node_2 {
public:
  std::unordered_map<unsigned short, Node_3> state_map;
  //unsigned int total_sum;
};

// srcIP
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
};

// dstIP
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
};

Node_0 state;

void update(u_char * packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP)
    return;

  struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + l2offset + iph->ip_hl * 4);
  unsigned short srcPort = tcph->source;
  unsigned short dstPort = tcph->dest;

  // SYN
  if (tcph->syn == 1 && tcph->ack == 0) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(dstIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(dstIP, Node_1())).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(srcIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(srcIP, Node_2())).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(dstPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(dstPort, Node_3())).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, int>::iterator it4 = state_3->state_map.find(srcPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, int>(srcPort, 0)).first;
    }

    it4->second = 1;
  }

  // SYN-ACK
  if (tcph->syn == 1 && tcph->ack == 1) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(srcIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, Node_1())).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, Node_2())).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(srcPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(srcPort, Node_3())).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, int>::iterator it4 = state_3->state_map.find(dstPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, int>(dstPort, 0)).first;
    }

    if (it4->second == 1)
      it4->second = 2;
  }

  // ACK
  if (tcph->syn == 0 && tcph->ack == 1) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(dstIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(dstIP, Node_1())).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(srcIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(srcIP, Node_2())).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(dstPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(dstPort, Node_3())).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, int>::iterator it4 = state_3->state_map.find(srcPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, int >(srcPort, 0)).first;
    }

    if (it4->second == 2)
      it4->second= 0;
  }
}

void close() {
  printf("Unique srcIP:  %lu\n", state.state_map.size());
  printf("Exit now.\n");
}

