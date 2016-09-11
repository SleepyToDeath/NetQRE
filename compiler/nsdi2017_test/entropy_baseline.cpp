#include <unordered_map>
#include <math.h>
#include "netqre.h"

std::unordered_map<unsigned long, long> IPcount;
long sum;

// entropy
double result = 0;

void update(u_char* packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;

  sum += 1;

  std::unordered_map<unsigned long, long>::iterator it = IPcount.find(srcIP);
  if (it == IPcount.end()) { 
    IPcount.insert(std::pair<unsigned long, long>(srcIP, 1));
  } else {
    it->second += 1;
  }
}

double frequency(unsigned long ip) {
  return ((double)IPcount[ip])/sum;
}

void _update_result() {
  result = 0;
  for (auto it = IPcount.begin(); it != IPcount.end(); it++) {
    unsigned long ip = it->first;
    result -= frequency(ip) * log(frequency(ip));
  }
}

void close() {
  _update_result();
  printf("entropy is : %f\n", result);
  printf("Exit now.\n");
}

