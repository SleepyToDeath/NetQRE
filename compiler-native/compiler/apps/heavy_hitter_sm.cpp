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
//#define BUF_SIZE 2685075

// 1 threads
//#define BUF_SIZE 37306496

// 4 threads
//#define BUF_SIZE 10503169

// 8 threads
#define BUF_SIZE 6064873

// 10 threads
//#define BUF_SIZE 3900523

// 12 threads
//#define BUF_SIZE 3535575

// 16 threads
//#define BUF_SIZE 3788257

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

//vector<Node_0> state(num_threads);

long result = 0;


//vector<list<u_char*>> queues;
vector<int> len;

//vector<unsigned long*> queues;
vector<vector<unsigned long>*> queues;

//mutex and buffer size for all the queues
vector<pthread_mutex_t> mutex_arr;
vector<int> buffer_size;
vector<pthread_cond_t> condc;

void _update_state(u_char *packet);


void* thread_run(void *threadid) {
  struct timeval start, end;

  gettimeofday(&start, NULL);

  
  long tid = (long)threadid;

  //unsigned long *queue = queues[tid];
  vector<unsigned long> *queue = queues[tid];

  // int length = len[tid];

  int val=0;

  Node_0 state;
  unsigned long  count;
  
  // for (int l=0; l<length; l++) {
  while(true) {
//      struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
//      unsigned long srcIP = iph->ip_src.s_addr;
//      unsigned long dstIP = iph->ip_dst.s_addr;

      pthread_mutex_lock(&mutex_arr[tid]);                  //lock the variable buffer[i] and queue
      while ( buffer_size[tid] == 0 ) {
        pthread_cond_wait(&condc[tid], &mutex_arr[tid]);    //wait if it is 0

      }

      unsigned long srcIP = (*queue)[val++];
      buffer_size[tid]--;                               //decrement count by 1
      pthread_mutex_unlock(&mutex_arr[tid]);                //release variable

      //TODO : come up with a better exit condition
      if ( srcIP == 123456 ) {
        break;
      }


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

  // cout << "Out of Loop in thread " << tid << endl;
 

  gettimeofday(&end, NULL);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
	      - (start.tv_sec * 1000000 + start.tv_usec);

  printf("Thread %ld takes %ld seconds, processes %d packets. Each packet takes %f us.\n", tid, time_spent, val, (double)(time_spent)/(val));
  // printf("Thread %ld takes %ld seconds.\n", tid, time_spent);

//cout << count << endl;
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

  long tid = srcIP % num_threads;
  // long tid = packet_cnt % num_threads;
  // put this packet into the thread's queue

  //u_char* packet2 = 
  //memcpy(dest, const void* src, std::size_t count );

  //queues[tid].push_back(packet);



  pthread_mutex_lock(&mutex_arr[tid]);
  queues[tid]->push_back(srcIP);
  buffer_size[tid]++;
  pthread_cond_signal(&condc[tid]);
  pthread_mutex_unlock(&mutex_arr[tid]);
  // len[tid]++;

  //queues[tid][len[tid]++] = srcIP;
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

  queues.resize(num_threads);                         //resizes queues (actual queues to each processor) to hold only num_threads elements
  len.resize(num_threads);                            //resizes len (size of each queue) to hold only num_threads elements
  mutex_arr.resize(num_threads);                          //resized mutex to hold only num_threads elements
  buffer_size.resize(num_threads);
  condc.resize(num_threads);

  // init queues
  for (int i=0; i < num_threads; i++) {
    //queues[i] = new vector<unsigned long>(BUF_SIZE);
    queues[i] = new vector<unsigned long>;
    // queues[i] = new unsigned long[BUF_SIZE];
    len[i] = 0;
  
    //init mutex[i] (mutex for thread i)
    pthread_mutex_init(&mutex_arr[i],NULL);
    buffer_size[i]=0;   
    pthread_cond_init(&condc[i], NULL);
  }

  //Just print init data 
  for (int tid=0; tid<num_threads; tid++) {
    cout << "queue " << tid << ":  " << queues[tid] << " size: " << len[tid] << std::endl;
  }

  // create joinable threads
  int numberOfProcessors = sysconf(_SC_NPROCESSORS_ONLN);         //gets nums of online/available CPUs
  printf("Number of processors: %d\n", numberOfProcessors);

  vector<pthread_t> threads(num_threads);
  pthread_attr_t attr;
  cpu_set_t cpus;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  struct timeval start, end;
  gettimeofday(&start, NULL);

  //set a processor to a given thread and start thread 
  int rc;
  for(int i=0; i < num_threads; i++ ) {
      std::cout << "main() : creating thread, " << i << std::endl;
      
      CPU_ZERO(&cpus);
      CPU_SET(i, &cpus);
      pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

      rc = pthread_create(&(threads[i]), &attr, thread_run, (void *)i);
      if (rc){
        std::cout << "Error:unable to create thread," << rc << std::endl;
        exit(-1);
      }
  }

  //pthread_attr_destroy(&attr);
  for (int i=0; i<loop_num; i++) {
    handle = pcap_open_offline(argv[2], errbuf);

    if (handle == NULL) {
      fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
      return(1);
    }

    if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
      fprintf(stderr, "pcap_loop exited with error.\n");
      exit(1);
    }
  }

  cout << "join" << endl;

  //ending all threads
  for (int i=0 ; i< num_threads ; i++) {
    pthread_mutex_lock(&mutex_arr[i]);
    queues[i]->push_back(123456);
    buffer_size[i]++;
    pthread_cond_signal(&condc[i]);
    pthread_mutex_unlock(&mutex_arr[i]);
  }

  //join all threads
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
  close();

  gettimeofday(&end, NULL);
  long time_spent = end.tv_sec * 1000000 + end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec);
  printf("Thread Main takes %ld useconds.\n", time_spent);


  /* And close the session */
  pcap_close(handle);

//pthread_exit(NULL);
  return 0;
}
