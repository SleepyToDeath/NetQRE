#include <sys/time.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include "linux_compat.h"
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <iostream>

using namespace std;

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

int loop_num = 1;

std::vector<lin_ip> pkt_queue;

// leaf level
class Node_2 {
public:
  int state = 0;
};

// y
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
  Node_2 node_2_default;
  long sum = 0;
};

// x
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  Node_1 node_1_default;
};

double avg_per_packet_time = 0;
long max_time = 0;

pthread_mutex_t start_mutex;
pthread_mutex_t finish_mutex;

int to_start = 0;
int finish_count = 0;

pthread_cond_t start_cv;
pthread_cond_t finish_cv;

// Node_0 state;

// void update(lin_ip &packet) {
//     unsigned long srcIP = packet.ip_src.s_addr;
//     unsigned long dstIP = packet.ip_dst.s_addr;

//     std::unordered_map<unsigned long, Node_1>::iterator it = state.state_map.find(srcIP);
//     if (it == state.state_map.end()) { 
// 	it = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, state.node_1_default)).first;
//     }
//     Node_1 *state_1 = &(it->second);

//     std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
//     if (it2 == state_1->state_map.end()) { 
// 	it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, state_1->node_2_default)).first;
//     }
//     Node_2 *state_2 = &(it2->second);

//     if (state_2->state == 0) {
// 	state_2->state = 1;
// 	state_1->sum += 1;
//     }
// }

static void close() {
  printf("Processed %ld packets. \n", packet_cnt);
  // printf("34435 : %lu\n 3014787072 : %lu\n", state.state_map[34435].sum, state.state_map[3014787072].sum);
  // printf("Unique srcIP:  %lu\n", state.state_map.size());
  printf("Exit now.\n");
}

void* thread_run(void *threadID) {

  long tid = (long) threadID;
  long len = pkt_queue.size();

  struct timeval start, end;

  pthread_mutex_lock(&start_mutex);
  while(to_start==0) {
    pthread_cond_wait(&start_cv, &start_mutex);
  }
  pthread_mutex_unlock(&start_mutex);

  Node_0 state;
  gettimeofday(&start, NULL);

  for (int i=0; i<loop_num; i++) {
  	for (long l=0; l<len; l++) {

      unsigned long srcIP = pkt_queue[l].ip_src.s_addr;

      if (srcIP % num_threads != tid) {
        continue;
      }

      // if  (tid == 1 && l == 12489593) {
      //   int sm= 1;
      // }
      // if ( tid == 0 && l == 11244582) {
      //   int sm = 1;
      // }
      // cout << "T" << tid << ": " << l << endl;

      // update(pkt_queue[l]);

      // unsigned long srcIP = pkt_queue[l].ip_src.s_addr;
      unsigned long dstIP = pkt_queue[l].ip_dst.s_addr;

      std::unordered_map<unsigned long, Node_1>::iterator it = state.state_map.find(srcIP);
      if (it == state.state_map.end()) { 
        it = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, state.node_1_default)).first;
      }
      Node_1 *state_1 = &(it->second);

      std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
      if (it2 == state_1->state_map.end()) { 
        it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, state_1->node_2_default)).first;
      }
      Node_2 *state_2 = &(it2->second);

      if (state_2->state == 0) {
        state_2->state = 1;
        state_1->sum += 1;
      }
    }
  }

  gettimeofday(&end, NULL);

  pthread_mutex_lock(&finish_mutex);
  finish_count++;
  pthread_cond_signal(&finish_cv);
  pthread_mutex_unlock(&finish_mutex);

  // printf("Exit now.\n");

  long time_spent = end.tv_sec * 1000000 + end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec);

  // double per_packet_time = (double)(time_spent)/(len);

  // printf("Runtime: %ld seconds, processes %ld packets. Each packet takes %f us.\n",
        // time_spent, len, per_packet_time);

  if (time_spent > max_time) {
    max_time = time_spent;
  }

  pthread_exit(NULL);
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }

  struct lin_ip ip_pkt;

  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  memcpy(&ip_pkt, iph, sizeof(lin_ip));

  pkt_queue.push_back(ip_pkt);
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

  if (argc >= 6) {
    num_threads = atoi(argv[5]);
  }

  if (strcmp(argv[1], "offline") == 0) {
    is_offline = true;
  } else if (strcmp(argv[1], "live") == 0) {
    is_offline = false;
  } else {
    printf("Mode %s is not recognized\n", argv[1]);
    return 0;
  }

  handle = pcap_open_offline(argv[2], errbuf);
  if (handle == NULL) {
    fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
    return(1);
  }

  if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
    fprintf(stderr, "pcap_loop exited with error.\n");
    exit(1);
  }

  // run();
  struct timeval start, end;

  //create threads
  std::vector<pthread_t> threads(num_threads);

  pthread_mutex_init(&start_mutex, NULL);
  pthread_mutex_init(&finish_mutex, NULL);
  pthread_cond_init(&start_cv, NULL);
  pthread_cond_init(&finish_cv, NULL);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int rc;
  for (int i=0; i<num_threads; i++) {
    // std::cout << "main() : creating thread, " << i << std::endl;
    rc = pthread_create(&(threads[i]), NULL, thread_run , (void *)i);
  }

  pthread_attr_destroy(&attr);

  pthread_mutex_lock(&start_mutex);
  to_start = 1;
  gettimeofday(&start, NULL);
  pthread_cond_broadcast(&start_cv);
  pthread_mutex_unlock(&start_mutex);

  pthread_mutex_unlock(&finish_mutex);
  while (finish_count < num_threads) {
    pthread_cond_wait(&finish_cv, &finish_mutex);
  }

  gettimeofday(&end, NULL);
  pthread_mutex_unlock(&finish_mutex);

  void *status;
  for (int tid = 0; tid<num_threads; tid++) {
    rc = pthread_join(threads[tid], &status);
  }

  long time_spent = end.tv_sec * 1000000 + end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec);

  // printf("Average per-packet processing time:  %f us.\n", avg_per_packet_time/num_threads);
  printf("Max processing time:  %ld us.\n", max_time);
  printf("Total time:  %ld us.\n", time_spent);


  close();

  /* And close the session */
  pcap_close(handle);

  return(0);
}


