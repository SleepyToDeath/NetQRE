#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux_compat.h>
#include <unordered_map>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define ETHERNET_LINK_OFFSET 14

FILE *file = NULL;
int sockfd = 0;
struct sockaddr_in server_addr;
int addr_len = sizeof(server_addr);
long packet_cnt = 0;
bool debug_mode = false;

// leaf level
class Node_3 {
public:
  unsigned long sum;
  int state;
};

// now-5
class Node_2 {
public:
  std::unordered_map<unsigned long, Node_3> state_map;
};

// y
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
};

// x
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  int state;
};

Node_0 state;
Node_1 node_1_default;
Node_2 node_2_default;
Node_3 node_3_default;

int result = 0;
int window_time_sec = 5;
int alert_bw_threshold = 5000000; // bps

void _check_state(u_char *packet, unsigned long time_sec) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  unsigned long total_bytes = 0;

  std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(srcIP);
  if (it1 == state.state_map.end()) { 
    it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  } 
  Node_1 *state_1 = &(it1->second);

  std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
  if (it2 == state_1->state_map.end()) { 
    it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, node_2_default)).first;
  }
  Node_2 *state_2 = &(it2->second);

  std::unordered_map<unsigned long, Node_3>::iterator it3 = state_2->state_map.begin();
  while(it3 != state_2->state_map.end()) {
    if (it3->first > time_sec - window_time_sec) {
      total_bytes += it3->second.sum;
    }
    it3++;
  }

  if (total_bytes * 8 > alert_bw_threshold * window_time_sec) {
    struct sockaddr_in sa, da;
    char src_str[INET_ADDRSTRLEN], dst_str[INET_ADDRSTRLEN];
    sa.sin_addr.s_addr = srcIP;
    da.sin_addr.s_addr = dstIP;
    inet_ntop(AF_INET, &(sa.sin_addr), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(da.sin_addr), dst_str, INET_ADDRSTRLEN);
    fprintf(file, "%s %s %ld\n", src_str, dst_str, long(total_bytes*8/window_time_sec));

    char buffer[100];
    int n = sprintf(buffer, "%s %s\r\n", src_str, dst_str);
    sendto(sockfd, buffer, n, 0, (sockaddr *)&server_addr, (socklen_t)addr_len);
  }
}

void _update_state(u_char *packet, unsigned long time_sec) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(srcIP);
  if (it1 == state.state_map.end()) { 
    it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, node_1_default)).first;
  } 
  Node_1 *state_1 = &(it1->second);

  std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
  if (it2 == state_1->state_map.end()) { 
    it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, node_2_default)).first;
  }
  Node_2 *state_2 = &(it2->second);

  std::unordered_map<unsigned long, Node_3>::iterator it3 = state_2->state_map.begin();
  while(it3 != state_2->state_map.end()) {
    if (it3->first <= time_sec - window_time_sec) {
      it3 = state_2->state_map.erase(it3);
    } else {
      it3++;
    }
  }

  it3 = state_2->state_map.find(time_sec);
  if (it3 == state_2->state_map.end()) { 
    it3 = state_2->state_map.insert(std::pair<unsigned long, Node_3>(time_sec, node_3_default)).first;
  }
  Node_3 *state_3 = &(it3->second);

  if (state_3->state == 0) {
    state_3->state = 1;
    state_3->sum += ntohs(iph->ip_len) + ETHERNET_LINK_OFFSET;
    state_3->state = 0;
  }
}

static void close() {
  fflush(file);
  fclose(file);
  close(sockfd);
  //158.130.56.11 = 188252830
  //158.130.56.12 = 205030046
  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 100 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  unsigned long time_sec = header->ts.tv_sec + header->ts.tv_usec/1000000;
  _update_state(packet, time_sec);
  _check_state(packet, time_sec);
}

void openSocket() {
  struct sockaddr_in myaddr;
  int port = 50000;
  // TODO: make this configurable
  std::string server_ip = "158.130.56.12";

  /*---Open socket for streaming---*/
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(file, "Socket can not be opened\n");
    fflush(file);
    return;
  }

  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(0);
  bind(sockfd, (struct sockaddr *) &myaddr, sizeof(myaddr));

  /*---Initialize server address/port struct---*/
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

  /*
  std::string data = "Start\r\n";
  int len = sendto(sockfd, data.c_str(), strlen(data.c_str()), 0, (sockaddr *)&server_addr, (socklen_t)addr_len);
  fprintf(file, "sent message of length %d to %s\n", len, server_ip.c_str());
  fflush(file);
  */
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

  file = fopen("heavy_hitter_timeinterval.log", "w");
  openSocket();

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

  return(0);
}


