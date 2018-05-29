#include <unordered_map>
#include "netqre.h"

std::unordered_map<unsigned long, long> state;

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;

  std::unordered_map<unsigned long, long>::iterator it = state.find(srcIP);
  if (it == state.end()) { 
    it = state.insert(std::pair<unsigned long, long>(srcIP, 0)).first;
  }

  it->second += 1;
}

void close() {
  printf("Unique srcIP:  %lu\n", state.size());
  printf("Exit now.\n");
}

