#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <list>
#include <vector>
#include <arpa/inet.h>
#include "linux_compat.h"
#include <unordered_map>
#include <pthread.h>
#include <iostream>
#include <netdb.h>
#include <pcap.h>

// #include <pcap/pcap.h>

#define BASE_PORT_NUM 19104
#define SERVER_NAME "localhost"

using namespace std;

#define ETHERNET_LINK_OFFSET 14

long packet_cnt = 0;
bool debug_mode = false;
int num_threads = 5;

int loop_num = 1;

std::vector<lin_ip> pkt_queue;
std::vector<int> port_nums;
std::vector<int> cli_sockfd;

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
    // long length = pkt_queue.size();

    Node_0 state;
    unsigned long  count = 0;

    struct timeval start, end;

    //declaring socket stuff
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = BASE_PORT_NUM - 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error creating socket" << endl;
    }
    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
      cout << "Error connecting" << endl;      
    } 
    
    char buffer[256];
    unsigned long srcIP;
    gettimeofday(&start, NULL);

    while (true) {
      bzero(buffer,256);
      n = read(sockfd,buffer,255);
      if (n < 0) {
        cout << "ERROR reading from socket" << endl;
        break;
      }

      srcIP = *((unsigned long *) buffer);
      
      if (count % 1000000==0) {
        cout << "T" << threadid << ": " << srcIP << endl;
      }

      if (srcIP == 123456) {
        break;
      }

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
    close(sockfd);

    //  printf("Processed %ld packets. \n", packet_cnt);
    // printf("34435 : %lu\n 3014787072 : %lu\n", state.state_map[34435].sum, state.state_map[3014787072].sum);
    // printf("Unique srcIP:  %lu\n", state.state_map.size());
    printf("Exit now.\n");
    //
    //
    long time_spent = end.tv_sec * 1000000 + end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec);
    //
    //  printf("Thread runtime:  %ld us.\n", time_spent);

    if (time_spent > max_time) {
      max_time = time_spent;
    }
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

  struct lin_ip ip_pkt;

  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;

  int thread_num = srcIP % num_threads;
  // pkt_queue.push_back(ip_pkt);
  int n = write (cli_sockfd[thread_num], &srcIP, sizeof(srcIP));
  if (n < 0) {
    cout << "Error writing to socket" << endl;
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

  //declarations for sockets
  int sockfd, portno;
  socklen_t cli_len;
  struct sockaddr_in serv_addr, cli_addr;

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

  //generating port nums
  port_nums.resize(num_threads);
  for (int i=0; i<num_threads; i++) {
    port_nums[i]=BASE_PORT_NUM+i;
  }

  if (strcmp(argv[1], "offline") == 0) {
    is_offline = true;
  } else if (strcmp(argv[1], "live") == 0) {
    is_offline = false;
  } else {
    printf("Mode %s is not recognized\n", argv[1]);
    return 0;
  }

  //socket setup
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd<0) {
    cout << "Error creating socket" << endl;
    return(1);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));      //clearing memory
  portno = BASE_PORT_NUM - 1;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    cout << "Error binding socket" << endl;
  }
  listen(sockfd, num_threads);

  //resize sockfds for threads
  cli_sockfd.resize(num_threads);
  cli_len = sizeof(cli_addr);

  // create joinable threads
  vector<pthread_t> threads(num_threads);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc;
  for(int i=0; i < num_threads; i++ ){
      std::cout << "main() : creating thread, " << i << std::endl;
      rc = pthread_create(&(threads[i]), NULL, thread_run, (void *)i);

  }
  pthread_attr_destroy(&attr);
  
  //get sockfd for clients
  for (int i=0; i<num_threads; i++) {
    cli_sockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
  }
  //note there is no mapping from cli_sockfd to port_nums, it could be generated but currently it is not needed


  struct timeval start, end;
  gettimeofday(&start, NULL);

  handle = pcap_open_offline(argv[2], errbuf);
  if (handle == NULL) {
    fprintf(stderr, "Couldn't open file %s: %s\n", argv[2], errbuf);
    return(1);
  }

  if (pcap_loop(handle, -1, (pcap_handler) handleCapturedPacket, NULL) < 0) {
    fprintf(stderr, "pcap_loop exited with error.\n");
    exit(1);
  }
 
  int end_data = 123456;
  for (int i=0; i<num_threads; i++) {
    write (cli_sockfd[i], &end_data, sizeof(end_data));
  }

  void *status;
  for (int tid = 0; tid < num_threads; tid++) {
    rc = pthread_join(threads[tid], &status);
  }

  gettimeofday(&end, NULL);

  for (int i=0; i<num_threads; i++) {
    close(cli_sockfd[i]);
  }
  close(sockfd);

  long time_spent = end.tv_sec * 1000000 + end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec);

  printf("Average per-packet processing time:  %f us.\n", avg_per_packet_time/num_threads);
  printf("Max processing time:  %ld us.\n", max_time);
  printf("Total time:  %ld us.\n", time_spent);


  close();

  /* And close the session */
  pcap_close(handle);

//pthread_exit(NULL);
  return 0;
}
