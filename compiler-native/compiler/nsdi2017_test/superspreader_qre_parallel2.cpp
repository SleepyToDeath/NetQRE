#include <unordered_map>
#include <sys/time.h>
#include "netqre.h"

using namespace std;

extern int loop_num;
extern double avg_per_packet_time;
extern long max_time;

extern int to_start;
extern int finish_count;
extern pthread_mutex_t start_mutex;
extern pthread_mutex_t finish_mutex;

extern pthread_cond_t start_cv;
extern pthread_cond_t finish_cv;

extern vector<PKT_QUEUE *> thread_queue;
extern vector<long> len;

// leaf level
class Node_2 {
public:
  int state;
};

// y
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
  Node_2 node_2_default;
  long sum;
};

// x
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  Node_1 node_1_default;
};


void update(u_char * packet, Node_0 &state) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

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

// don't need to change 
void* thread_run(void *threadid) {
  long tid = (long)threadid;

  Node_0 state;
  unsigned long  count = 0;

  struct timeval start, end;

  pthread_mutex_lock(&start_mutex);
  while (to_start==0) {
    pthread_cond_wait(&start_cv, &start_mutex);
  }
  pthread_mutex_unlock(&start_mutex);

  gettimeofday(&start, NULL);

  PKT_QUEUE *pkt_queue = thread_queue[tid];
  long length = len[tid];

  for (int i=0; i<loop_num; i++) {
    for (long l=0; l<length; l++) {

      u_char* packet = (*pkt_queue)[l];
      update(packet, state);
    }
  }

  gettimeofday(&end, NULL);

  pthread_mutex_lock(&finish_mutex);
  finish_count++;
  pthread_cond_signal(&finish_cv);
  pthread_mutex_unlock(&finish_mutex);


  printf("Thread %ld Processed %ld packets. \n", tid, length);
  printf("Thread %ld has unique srcIP:  %lu\n", tid, state.state_map.size());
  //
  //
  long time_spent = end.tv_sec * 1000000 + end.tv_usec
    - (start.tv_sec * 1000000 + start.tv_usec);
  //
  printf("Thread %ld runtime:  %ld us.\n", tid, time_spent);

  if (time_spent > max_time)
    max_time = time_spent;

  pthread_exit(NULL);
}

