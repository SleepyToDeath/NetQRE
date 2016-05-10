#ifndef MAIN_PROCESS_H
#define MAIN_PROCES_H

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include "../compiler/library/linux_compat.h"
#include <unordered_map>
#include <unistd.h>

#define ETHERNET_LINK_OFFSET 14

extern bool debug_mode;

void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet);
void close();
void _output_result();
void _update_state(u_char *packet);
void _update_result(unsigned long srcIP, unsigned long dstIP);

#endif
