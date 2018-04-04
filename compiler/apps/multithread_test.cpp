#include <unistd.h>
#include <sys/time.h>
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

using namespace std;

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

long result = 0;

double avg_per_packet_time = 0;
long max_time = 0;

void* thread_run(void *threadid) {
  struct timeval start, end;


  long tid = (long)threadid;

  //long length = 37306496/num_threads;
  long length = 4663312;

  int res = 0;

  sleep(5);
  
  gettimeofday(&start, NULL);

  for (long l=0; l<length; l++) {
    for (int i=1; i<500; i++) {
	res += 1000 % i;
	res = res % 1000;
    }
  }

  gettimeofday(&end, NULL);

  cout << "res = " << res << endl;

  long time_spent = end.tv_sec * 1000000 + end.tv_usec
	      - (start.tv_sec * 1000000 + start.tv_usec);

  double per_packet_time = (double)(time_spent)/(length);
  avg_per_packet_time += per_packet_time;
  if (max_time < time_spent)
      max_time = time_spent;

  printf("Thread %ld takes %ld seconds, processes %ld packets. Each packet takes %f us.\n", 
	tid, time_spent, length, (double)(time_spent)/(length));

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  num_threads = atoi(argv[1]);

  struct timeval start, end;

  gettimeofday(&start, NULL);

  // create joinable threads
  vector<pthread_t> threads(num_threads);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  cpu_set_t cpus;

  int rc;
  for(int i=0; i < num_threads; i++ ){
      std::cout << "main() : creating thread, " << i << std::endl;
      CPU_ZERO(&cpus);
      CPU_SET(i, &cpus); 
      pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus); 

      rc = pthread_create(&(threads[i]), &attr, 
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

  printf("Average per-packet processing time:  %f us.\n", avg_per_packet_time/num_threads);
  printf("Max processing time:  %ld us.\n", max_time);
  printf("Total time:  %ld us.\n", time_spent);

//pthread_exit(NULL);
  return 0;
}
