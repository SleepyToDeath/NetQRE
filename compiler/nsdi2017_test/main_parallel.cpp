#include <unistd.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include "netqre.h"

using namespace std;

#define ETHERNET_LINK_OFFSET 14

#define BUF_SIZE 19086957

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

int loop_num = 1;

vector<PKT_QUEUE *> thread_queue;

vector<long> len;

vector<u_char*> input_queue;

void* thread_run(void*);

double avg_per_packet_time = 0;
long max_time = 0;

pthread_mutex_t start_mutex;
pthread_mutex_t finish_mutex;

int to_start = 0;
int finish_count = 0;

pthread_cond_t start_cv;
pthread_cond_t finish_cv;

void clear() {
  long len = input_queue.size();
  for (int i = 0; i < len; i++) {
      free(input_queue[i]);
  }
}

void close() {
  printf("Processed %ld packets. \n", packet_cnt);
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

    input_queue.push_back(pkt);
  }
}

void dispatch() {
  //cout << "size " << input_queue.size() << endl;
  for (long i = 0; i < input_queue.size(); i++) {
    u_char* pkt = input_queue[i];
    struct lin_ip* iph = (struct lin_ip*) (pkt + l2offset);
    unsigned long srcIP = iph->ip_src.s_addr;
    long tid = srcIP % num_threads;

    //thread_queue[srcIP % num_threads]->push_back(pkt);
    (*thread_queue[tid])[len[tid]++] = (pkt);
  }
  //cout << "done" << endl;
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




  // initialize queue for each thread
  vector<pthread_t> threads(num_threads);
  thread_queue.resize(num_threads);
  len.resize(num_threads);

  for(int i=0; i < num_threads; i++ ){
    std::cout << "main() : creating thread queue for thread " << i << std::endl;
    thread_queue[i] = new PKT_QUEUE(BUF_SIZE);
    len[i] = 0;
  }

  if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
    fprintf(stderr, "pcap_loop exited with error.\n");
    exit(1);
  }



  // create joinable threads
  struct timeval start, end;

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

  gettimeofday(&start, NULL);
  dispatch();

  to_start = 1;
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

