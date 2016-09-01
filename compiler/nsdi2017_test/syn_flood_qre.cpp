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
class Node_6 {
public:
  int state;
};

// y
class Node_5 {
public:
  std::unordered_map<unsigned int, Node_6> state_map;
  Node_6 default_state;
};

// x
class Node_4 {
public:
  std::unordered_map<unsigned int, Node_5> state_map;
  Node_5 default_state;
};

// srcPort
class Node_3 {
public:
  std::unordered_map<unsigned short, Node_4> state_map;
  Node_4 default_state;
};

// dstPort
class Node_2 {
public:
  std::unordered_map<unsigned short, Node_3> state_map;
  Node_3 default_state;
  unsigned int total_sum;
};

// srcIP
class Node_1 {
public:
  std::unordered_map<unsigned long, Node_2> state_map;
  Node_2 default_state;
};

// dstIP
class Node_0 {
public:
  std::unordered_map<unsigned long, Node_1> state_map;
  Node_1 default_state;
};

Node_0 state;

int alert_count_threshold = 10000;

void _check_state(u_char *packet, unsigned long time_sec) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP)
    return;

  std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(dstIP);
  if (it1 == state.state_map.end()) { 
    it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(dstIP, state.default_state)).first;
  } 
  Node_1 *state_1 = &(it1->second);

  std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(srcIP);
  if (it2 == state_1->state_map.end()) { 
    it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(srcIP, state_1->default_state)).first;
  } 
  Node_2 *state_2 = &(it2->second);

  if (state_2->total_sum > alert_count_threshold) {
    struct sockaddr_in sa, da;
    char src_str[INET_ADDRSTRLEN], dst_str[INET_ADDRSTRLEN];
    sa.sin_addr.s_addr = srcIP;
    da.sin_addr.s_addr = dstIP;
    inet_ntop(AF_INET, &(sa.sin_addr), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(da.sin_addr), dst_str, INET_ADDRSTRLEN);
    fprintf(file, "block traffic from %s to %s because syn count is %u\n", src_str, dst_str, state_2->total_sum);

    char buffer[100];
    int n = sprintf(buffer, "%s %s\r\n", src_str, dst_str);
    sendto(sockfd, buffer, n, 0, (sockaddr *)&server_addr, (socklen_t)addr_len);
  }
}

void _update_state(u_char *packet, unsigned long time_sec) {
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  if (iph->ip_p != IPPROTO_TCP)
    return;

  struct lin_tcphdr* tcph = (struct lin_tcphdr *) (packet + ETHERNET_LINK_OFFSET + iph->ip_hl * 4);
  unsigned short srcPort = tcph->source;
  unsigned short dstPort = tcph->dest;
  unsigned int seq = ntohl(tcph->seq);
  unsigned int ack_seq = ntohl(tcph->ack_seq);

	// SYN
	if (tcph->syn == 1 && tcph->ack == 0) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(dstIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(dstIP, state.default_state)).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(srcIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(srcIP, state_1->default_state)).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(dstPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(dstPort, state_2->default_state)).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, Node_4>::iterator it4 = state_3->state_map.find(srcPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, Node_4>(srcPort, state_3->default_state)).first;
    }
    Node_4 *state_4 = &(it4->second);

    std::unordered_map<unsigned int, Node_5>::iterator it5 = state_4->state_map.find(seq);
    if (it5 == state_4->state_map.end()) { 
      it5 = state_4->state_map.insert(std::pair<unsigned int, Node_5>(seq, state_4->default_state)).first;
    }
    Node_5 *state_5 = &(it5->second);

    std::unordered_map<unsigned int, Node_6>::iterator it6 = state_5->state_map.begin();
    while(it6 != state_5->state_map.end()) {
      if (it6->second.state == 0) {
        it6->second.state = 1;
      }
      it6++;
    }
    
    state_2->total_sum += 1;
    Node_6 *state_6 = &(state_5->default_state);
    if (state_6->state == 0) {
      state_6->state = 1;
      //fprintf(file, "%lu:%hu->%lu:%hu is %d\n", srcIP, srcPort, dstIP, dstPort, state_2->total_sum);
    }
	}

	// SYN-ACK
	if (tcph->syn == 1 && tcph->ack == 1) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(srcIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(srcIP, state.default_state)).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(dstIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(dstIP, state_1->default_state)).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(srcPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(srcPort, state_2->default_state)).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, Node_4>::iterator it4 = state_3->state_map.find(dstPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, Node_4>(dstPort, state_3->default_state)).first;
    }
    Node_4 *state_4 = &(it4->second);

    std::unordered_map<unsigned int, Node_5>::iterator it5 = state_4->state_map.find(ack_seq - 1);
    if (it5 == state_4->state_map.end()) { 
      it5 = state_4->state_map.insert(std::pair<unsigned int, Node_5>(ack_seq - 1, state_4->default_state)).first;
    }
    Node_5 *state_5 = &(it5->second);

    std::unordered_map<unsigned int, Node_6>::iterator it6 = state_5->state_map.find(seq);
    if (it6 == state_5->state_map.end()) { 
      it6 = state_5->state_map.insert(std::pair<unsigned int, Node_6>(seq, state_5->default_state)).first;
    }
    Node_6 *state_6 = &(it6->second);

    if (state_6->state == 1) {
      state_6->state = 2;
      //state_2->total_sum += 1;
    }
	}

	// ACK
	if (tcph->syn == 0 && tcph->ack == 1) {
    std::unordered_map<unsigned long, Node_1>::iterator it1 = state.state_map.find(dstIP);
    if (it1 == state.state_map.end()) {
      it1 = state.state_map.insert(std::pair<unsigned long, Node_1>(dstIP, state.default_state)).first;
    } 
    Node_1 *state_1 = &(it1->second);

    std::unordered_map<unsigned long, Node_2>::iterator it2 = state_1->state_map.find(srcIP);
    if (it2 == state_1->state_map.end()) { 
      it2 = state_1->state_map.insert(std::pair<unsigned long, Node_2>(srcIP, state_1->default_state)).first;
    }
    Node_2 *state_2 = &(it2->second);

    std::unordered_map<unsigned short, Node_3>::iterator it3 = state_2->state_map.find(dstPort);
    if (it3 == state_2->state_map.end()) { 
      it3 = state_2->state_map.insert(std::pair<unsigned short, Node_3>(dstPort, state_2->default_state)).first;
    }
    Node_3 *state_3 = &(it3->second);

    std::unordered_map<unsigned short, Node_4>::iterator it4 = state_3->state_map.find(srcPort);
    if (it4 == state_3->state_map.end()) { 
      it4 = state_3->state_map.insert(std::pair<unsigned short, Node_4>(srcPort, state_3->default_state)).first;
    }
    Node_4 *state_4 = &(it4->second);

    std::unordered_map<unsigned int, Node_5>::iterator it5 = state_4->state_map.begin();
    while(it5 != state_4->state_map.end()) {
      Node_5 *state_5 = &(it5->second);

      std::unordered_map<unsigned int, Node_6>::iterator it6 = state_5->state_map.find(ack_seq - 1);
      if (it6 == state_5->state_map.end()) { 
        it6 = state_5->state_map.insert(std::pair<unsigned int, Node_6>(ack_seq - 1, state_5->default_state)).first;
      }
      Node_6 *state_6 = &(it6->second);

      if (state_6->state == 2) {
        state_6->state = 3;
        state_2->total_sum -= 1;
      }
      it5++;
    }
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

  file = fopen("syn_flood.log", "w");
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
