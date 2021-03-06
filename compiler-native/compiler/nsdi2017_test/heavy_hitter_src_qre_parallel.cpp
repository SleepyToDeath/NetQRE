#include <unistd.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <list>
#include <vector>
#include <arpa/inet.h>
#include <unordered_map>
#include <pthread.h>
#include <iostream>
#include "linux_compat.h"

using namespace std;

#define ETHERNET_LINK_OFFSET 14

int l2offset = 0;

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

int loop_num = 1;

std::vector<u_char*> pkt_queue;
void clear() {
  long len = pkt_queue.size();
  for (long i=0; i<len; i++) {
    free(pkt_queue[i]);
  }
}

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

double avg_per_packet_time = 0;
long max_time = 0;

pthread_mutex_t start_mutex;
pthread_mutex_t finish_mutex;

int to_start = 0;
int finish_count = 0;

pthread_cond_t start_cv;
pthread_cond_t finish_cv;

void* thread_run(void *threadid) {

  long tid = (long)threadid;
  long length = pkt_queue.size();

  Node_0 state;
  unsigned long  count = 0;

  struct timeval start, end;

  pthread_mutex_lock(&start_mutex);
  while (to_start==0) {
    pthread_cond_wait(&start_cv, &start_mutex);
  }
  pthread_mutex_unlock(&start_mutex);


  gettimeofday(&start, NULL);

  for (int i=0; i<loop_num; i++) {
    for (long l=0; l<length; l++) {

      u_char* packet = pkt_queue[l];
      struct lin_ip* iph = (struct lin_ip*) (packet);
      unsigned long srcIP = iph->ip_src.s_addr;

      if (srcIP % num_threads != tid)
	continue;

      //count++;
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
  }

  gettimeofday(&end, NULL);

  pthread_mutex_lock(&finish_mutex);
  finish_count++;
  pthread_cond_signal(&finish_cv);
  pthread_mutex_unlock(&finish_mutex);


  //  printf("Processed %ld packets. \n", packet_cnt);
  // printf("34435 : %lu\n 3014787072 : %lu\n", state.state_map[34435].sum, state.state_map[3014787072].sum);
  // printf("Unique srcIP:  %lu\n", state.state_map.size());
  printf("Exit now.\n");
  //
  //
  long time_spent = end.tv_sec * 1000000 + end.tv_usec
    - (start.tv_sec * 1000000 + start.tv_usec);
  //
  //  printf("Thread runtime:  %ld us.\n", time_spent);

  if (time_spent > max_time)
    max_time = time_spent;

  pthread_exit(NULL);
}

static void close() {
  printf("Processed %ld packets. \n", packet_cnt);

  //  long count = 0;
  //  for (int tid=0; tid<num_threads; tid++) 
  //      for (auto it = state.state_map.begin(); it != state.state_map.end(); it++) {
  //	count += it->second.sum;
  //      }
  //
  //  printf("Total number of packets in map: %ld \n", count);

  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1000000 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }

  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);

  // only push IPv4 packets into the queue
  if (iph->ip_v == 4) {
    u_char* pkt = (u_char *)malloc(header->caplen);
    memcpy(pkt, packet, header->caplen);

    pkt_queue.push_back(pkt);
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

  struct timeval start, end;


  // create joinable threads
  vector<pthread_t> threads(num_threads);

  pthread_mutex_init(&start_mutex, NULL);
  pthread_mutex_init(&finish_mutex, NULL);
  pthread_cond_init (&finish_cv, NULL);
  pthread_cond_init (&start_cv, NULL);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  cpu_set_t cpus;

  int rc;
  for(int i=0; i < num_threads; i++ ){
    std::cout << "main() : creating thread, " << i << std::endl;

    CPU_ZERO(&cpus);
    CPU_SET(i+1, &cpus);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

    rc = pthread_create(&(threads[i]), &attr, 
	thread_run, (void *)i);
    //    if (rc){
    //        std::cout << "Error:unable to create thread," << rc 
    //      	    << std::endl;
    //        exit(-1);
    //    }
  }

  pthread_attr_destroy(&attr);

  pthread_mutex_lock(&start_mutex);
  to_start = 1;
  gettimeofday(&start, NULL);
  pthread_cond_broadcast(&start_cv);
  pthread_mutex_unlock(&start_mutex);

  pthread_mutex_lock(&finish_mutex);
  while (finish_count < num_threads) {
    pthread_cond_wait(&finish_cv, &finish_mutex);
  }
  gettimeofday(&end, NULL);
  pthread_mutex_unlock(&finish_mutex);

  void *status;
  for (int tid = 0; tid < num_threads; tid++) {
    rc = pthread_join(threads[tid], &status);
    //    if (rc){
    //	std::cout << "Error:unable to join," << rc << std::endl;
    //	exit(-1);
    //    }
    //    std::cout << "Main: completed thread id :" << tid << std::endl;
    //    std::cout << "  exiting with status :" << status << std::endl;
  }


  long time_spent = end.tv_sec * 1000000 + end.tv_usec
    - (start.tv_sec * 1000000 + start.tv_usec);

  printf("Average per-packet processing time:  %f us.\n", avg_per_packet_time/num_threads);
  printf("Max processing time:  %ld us.\n", max_time);
  printf("Total time:  %ld us.\n", time_spent);


  close();

  /* And close the session */
  pcap_close(handle);

  clear();
  //pthread_exit(NULL);
  return 0;
}

