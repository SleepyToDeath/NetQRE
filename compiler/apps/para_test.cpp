#include <unistd.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <list>
#include <vector>
#include <arpa/inet.h>
#include <linux_compat.h>
#include <unordered_map>
#include <pthread.h>
#include <iostream>

using namespace std;

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

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

long result = 0;

vector<unsigned long> queue;

void* thread_run(void *threadid) {
  struct timeval start, end;

  gettimeofday(&start, NULL);

  long tid = (long)threadid;

  long length = queue.size();

  Node_0 state;
  unsigned long  count = 0;
  
  for (long l=0; l<length; l++) {

      unsigned long srcIP = (queue)[l];

      if (srcIP % num_threads != tid)
	continue;

      count++;
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
 

  gettimeofday(&end, NULL);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
	      - (start.tv_sec * 1000000 + start.tv_usec);

  printf("Thread %ld takes %ld seconds, processes %ld packets. Each packet takes %f us.\n", 
	tid, time_spent, count, (double)(time_spent)/(count));
  cout << "thread " << tid << " exit." << endl;
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

  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;

  queue.push_back(srcIP);
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
  int loop_num = 1;
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

  struct timeval start, end;

  gettimeofday(&start, NULL);

  // create joinable threads
  vector<pthread_t> threads(num_threads);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int rc;
  for(int i=0; i < num_threads; i++ ){
      std::cout << "main() : creating thread, " << i << std::endl;
      rc = pthread_create(&(threads[i]), NULL, 
	      thread_run, (void *)i);
      if (rc){
	  std::cout << "Error:unable to create thread," << rc 
		    << std::endl;
	  exit(-1);
      }
  }

  pthread_attr_destroy(&attr);

  void *status;
  for (int tid = 0; tid < num_threads; tid++) {
    rc = pthread_join(threads[tid], &status);
    if (rc){
	std::cout << "Error:unable to join," << rc << std::endl;
	exit(-1);
    }
    std::cout << "Main: completed thread id :" << tid << std::endl;
    std::cout << "  exiting with status :" << status << std::endl;
  }

  gettimeofday(&end, NULL);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
	      - (start.tv_sec * 1000000 + start.tv_usec);

  printf("Total time:  %ld us.\n", time_spent);


  close();

  /* And close the session */
  pcap_close(handle);

//pthread_exit(NULL);
  return 0;
}
