#include <sys/time.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include "netqre.h"

//#define ETHERNET_LINK_OFFSET 14
//
long packet_cnt = 0;
bool debug_mode = false;
int loop_num = 1;

std::vector<u_char*> pkt_queue;
void init() {}

void clear() {
  long len = pkt_queue.size();
  for (long i=0; i<len; i++) {
    free(pkt_queue[i]);
  }
}

void run() {
  printf("Starting ...\n");

  long len = pkt_queue.size();

  struct timeval start, end;
  gettimeofday(&start, NULL);

  for (int j=0; j<loop_num; j++)
    for (long i=0; i<len; i++) {
      update(pkt_queue[i]);
    }

  gettimeofday(&end, NULL);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
    - (start.tv_sec * 1000000 + start.tv_usec);

  double per_packet_time = (double)(time_spent)/(len);

  printf("Runtime: %ld useconds, processes %ld packets. Each packet takes %f us.\n",
      time_spent, len, per_packet_time);
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }

  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);

  // only push IPv4 packets into the queue
  if (iph->ip_v == 4) {
    update(packet);
  }
}

int main(int argc, char *argv[]) {
  pcap_t *handle;			/* Session handle */
  char *dev;			/* The device to sniff on */
  char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
  struct bpf_program fp;		/* The compiled filter */
  bpf_u_int32 mask;		/* Our netmask */
  bpf_u_int32 net;		/* Our IP */
  struct pcap_pkthdr header;	/* The header that pcap gives us */
  const u_char *packet;		/* The actual packet */
  bool is_offline = true;

  if (argc < 3) {
    printf("Usage: ./main mode path_to_file [loop_num] [debug_mode]\n");
    return 0;
  }

  if (argc >= 4) {
    loop_num = atoi(argv[3]);
  }

  if (argc >= 5) {
    debug_mode = strcmp(argv[4], "debug") == 0;
  }

  if (strcmp(argv[1], "offline") == 0) {
    is_offline = true;
  } else if (strcmp(argv[1], "live") == 0) {
    is_offline = false;
  } else {
    printf("Mode %s is not recognized\n", argv[1]);
    return 0;
  }

  if (is_offline) {
      handle = pcap_open_offline(argv[2], errbuf);
      if (handle == NULL) {
        fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
        return(1);
      }

      // set the l2 offset for ethernet frames
      if (pcap_datalink(handle)==DLT_EN10MB)
	l2offset = 14;
      else 
	// if not ethernet, assuem the offset is 0
	l2offset = 0;

      if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
        fprintf(stderr, "pcap_loop exited with error.\n");
        exit(1);
      }

      // initialize the application code
  //    init();

      // run the application code
   //   run();

  } else {
     handle = pcap_open_live(argv[2], 65535, 1, 10, errbuf);
     if (handle == NULL) {
       fprintf(stderr, "Couldn't open device %s: %s\n", argv[2], errbuf);
       return(0);
     }
     if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
       fprintf(stderr, "pcap_loop exited with error.\n");
       exit(1);
     }
  }

  close();

  /* And close the session */
  pcap_close(handle);

  clear();

  int n;
   std::cin >> n;

  return(0);
}

