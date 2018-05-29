#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux_compat.h>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <netinet/in.h>
//#include <regex>
//#include <sstream>

#define ETHERNET_LINK_OFFSET 14

using namespace std;
long packet_cnt = 0;
bool debug_mode = false;

// level
struct Node_3 {
  int phase = 0;
};

// port
struct Node_2{
    unordered_map<string, Node_3> state_map;
    Node_3 default_state;
};

// user
struct Node_1 {
    unordered_map<string, Node_2> state_map;
    Node_2 default_state;
};

// sip, dip, sp, dp, callId
struct Node_0 {
    unordered_map<string, Node_1> state_map;
    Node_1 default_state;
};

// ip+port
struct DataChannel {
    int call_num = 0;
};

struct User {
    unordered_map<string, DataChannel> state_map;
    DataChannel default_state;
};

User user_state;
Node_0 srcDstState;

#define BEGIN 0
#define DATA 1
#define END 2
int phase = 0;

string callID;
string user;
string ip;
string port;
string command;
string code;

////regex sip_patter("(\\S+) .*");
//regex sip_pattern("INVITE [.|\xD|\xA]*From:[.|\xD|\xA]*");//[.|\xD|\xA]* <(.+)>[.|\xD|\xA]*");
////regex sip_pattern("[.|\xD|\xA]*From: <(.+)>[.|\xD|\xA]*");
//regex single_pattern(".|\xD|\xA");
//
//void getSIPHeader(u_char* payload) {
//  char* data = (char*)payload;
////  if (((char)payload[0])=='R')
////      cout << payload[0] << endl;
////  for (int i=0; i<strlen(data); i++) {
////      cout << data[i];
////  }
////
////  cout << endl;
////
////  for (int i=0; i<strlen(data); i++) {
////      stringstream converter;
////      converter << data[i];
////      cout << converter.str() << endl;
////      bool mat = regex_match(converter.str(), single_pattern);
////      cout << "mathced : " <<  mat;
////      cout << endl;
////
////      if (mat==false)
////        printf("%x", data[i]);
////        cout << (int)data[i] << endl;
////  }
////
////  cout <<"asdfasf" << endl;
////
////  stringstream converter;
////  for (int i=0; i<strlen(data); i++)
////      converter << data[i];
////  string s = converter.str();
////  //((const char*)payload);
////
////  cout << "converted string is:\n" << s << endl;
//  string s(data);
//  smatch m;
//  bool matched = regex_search(s, m, sip_pattern);
//  cout << "matched: " << matched << endl;
//
//  if (m.size()>=2) {
////    command = m[1].str();
////    cout << "command: " << command << endl;
//
//    user = m[1].str();
//    cout << "user: " << user << endl;
//  }
//}

void getCallID(char* payload) {
    char* token;
    token = strtok(payload, "<>:\n\"");

    while (token!=NULL) {
	if (0==strncmp(token, "Call-ID", 7)) {
	    //cout << token << endl;
	    token = strtok(NULL, "<>:\n\"");
	    callID = token;
//	    cout << callID << endl;
//	    cout << "DFDFDFDFDFDFDFDF" << endl;
	    break;
	} 
	token = strtok(NULL, "<>:\n\"");
    }
}

void getInfo(char* payload) {
    char* token;
    token = strtok(payload, "<>:\n\"");

    while (token!=NULL) {
	if (0==strncmp(token, "Call-ID", 7)) {
	    //cout << token << endl;
	    token = strtok(NULL, "<>:\n\"");
	    callID = token;
//	    cout << callID << endl;
//	    cout << "DFDFDFDFDFDFDFDF" << endl;
	} else if (0==strncmp(token, "From", 4)) {
	    token = strtok(NULL, "<>:\n\"");
	    token = strtok(NULL, "<>:\n\"");
	    user = token;
//	    cout << user << endl;
//	    cout << "DFDFDFDFDFDFDFDF" << endl;
	} else if (0==strncmp(token, "m=", 2)) {
	    char* save = strtok(token, " ");
	    save = strtok(NULL, " ");
	    port = save;
//	    cout << "port: " << port << endl;
//	    cout << "DFDFDFDFDFDFDFDF" << endl;
	} 
//	else if (0==strncmp(token, "c=", 2)) {
//	    cout << "token" << endl;
//	    cout << token << endl;
//	    char* save = strtok(token, " ");
//	    save = strtok(NULL, " ");
//	    save = strtok(NULL, " ");
//	    ip = save;
//	    cout << "ip= " << ip << endl;
//	    cout << "DFDFDFDFDFDFDFDF" << endl;
//	    cout << token << endl;
//	} 

	token = strtok(NULL, "<>:\n\"");
    }

}

void handleResponse(char* payload) {
    char response[4];
    for (int i=8; i<11; i++)
	response[i-8] = payload[i];   
    response[3] = '\0';
    code = response;
    command = response;
    //cout << code << endl;
    getCallID(payload);
}

void handleInvite(char* data) {
    getInfo(data);
}

void getSIPHeader(u_char* u_payload) {
    char* payload = (char*) u_payload;
    // response
    if (payload[0]=='S' && payload[1]=='I') {
	handleResponse(payload);
    }
    // INVITE
    else if (payload[0]=='I' && payload[1]=='N') {
	command = "INVITE";
	handleInvite(payload);
    }
    else if (payload[0]=='A' && payload[1]=='C') {
	command = "ACK";
	getCallID(payload);
    }
    else if (payload[0]=='B' && payload[1]=='Y') {
	command = "BYE";
	getInfo((char*)payload);
    }
    else if (payload[0]=='R' && payload[1]=='E') {
	command = "REGISTER";
    }
}

void _update_state(u_char *packet) {
  int k;
  struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  //printf("%ld -> %ld, %d \n", srcIP, dstIP, iph->ip_p);
  // UDP
  if (iph->ip_p == IPPROTO_UDP) {
    struct lin_udphdr* udph = (struct lin_udphdr*)(packet + sizeof(struct lin_ip) + ETHERNET_LINK_OFFSET);
    //printf("UDP port %d -> port %d\n", ntohs(udph->source),ntohs(udph->dest));

    if (udph->source == 50195 || udph->dest == 50195) {
	u_char* payload = (u_char *)(packet + ETHERNET_LINK_OFFSET + iph->ip_hl*4 + sizeof(struct lin_udphdr));
	//printf("payload is : \n%s\n", packet + sizeof(struct lin_udphdr)+sizeof(struct lin_ip) + ETHERNET_LINK_OFFSET);
	getSIPHeader(payload);

	//cout << "command: " << command<<endl;
	//cout << "callID: " << callID<<endl;
	//cout << "user: " << user<<endl;
	//cout << "port: " << port<<endl;

	if (command=="INVITE") {

	    stringstream converter;
	    converter << srcIP << dstIP << udph->source << udph->dest << callID;
	    string srcDst = converter.str();

	    auto it = srcDstState.state_map.find(srcDst);
	    if (it==srcDstState.state_map.end()) {
		it = srcDstState.state_map.insert(
		    pair<string, Node_1>(
			srcDst, srcDstState.default_state
		    )
		    ).first;
	    }
	    Node_1& node1 = it->second;

	    auto it1 = node1.state_map.find(user);
	    if (it1 == node1.state_map.end()) {
		it1 = node1.state_map.insert(pair<string, Node_2>(
		    user, node1.default_state
		    )
		    ).first;
	    }

	    Node_2& node2 = it1->second;
	    auto it2 = node2.state_map.find(port);
	    if (it2 == node2.state_map.end()) {
		it2 = node2.state_map.insert(pair<string, Node_3>(
		    port, node2.default_state
		    )
		    ).first;
	    }
	    Node_3& node3 = it2->second;

	    //if (node3->phase==0) {
	    node3.phase=1;
	    //} 
		    //cout << "user: " << user << endl;
		    //cout << "port: " << port << endl;
		    //cout << "OK" << endl;
		    //cin >> k;

	}
	if (command=="ACK") {
//cout << "ACK" << endl;
//cin >> k;
	    stringstream converter;
	    converter << srcIP << dstIP << udph->source << udph->dest << callID;
	    string srcDst = converter.str();

//cout << "srcDst" << srcDst << endl;
	    auto it = srcDstState.state_map.find(srcDst);
	    if (it==srcDstState.state_map.end()) {
		it = srcDstState.state_map.insert(pair<string, Node_1>(
		    srcDst, srcDstState.default_state
		    )
		    ).first;
	    }
	    Node_1& node1 = it->second;

	    for (auto it1=node1.state_map.begin();
		it1!=node1.state_map.end();
		it1++) {
		string user = it1->first;
		Node_2& node2 = it1->second;

//cout << "user: " << user << endl;
		for (auto it2 = node2.state_map.begin();
		    it2!=node2.state_map.end();
		    it2++) {
		    string port = it2->first;
		    Node_3& node3 = it2->second;

//cout << "port: " << port << endl;
		    if (node3.phase==2) {
			node3.phase=3;
	    //cout << "Changing state to 3" << endl;
	    //cout << node3.phase << endl;
	    //cin >> k;

			auto itu = user_state.state_map.find(user);
			if (itu==user_state.state_map.end()) {
			    itu = user_state.state_map.insert(
				{user,user_state.default_state}
			    ).first;
			}
			DataChannel& dataChannel = itu->second;
			dataChannel.call_num += 1;
	//cout << user << endl;
	//cout << srcIpPort << endl;
	//cout << "starting.." << endl;
	//cin >> k;
		    } 
		}
	    }
	}
	if (command=="200") {
	    stringstream converter;
	    converter << dstIP << srcIP << udph->dest << udph->source << callID;
	    string srcDst = converter.str();

	    auto it = srcDstState.state_map.find(srcDst);
	    if (it==srcDstState.state_map.end()) {
		it = srcDstState.state_map.insert(pair<string, Node_1>(
		    srcDst, srcDstState.default_state
		    )
		    ).first;
	    }
	    Node_1& node1 = it->second;

	    for (auto it1=node1.state_map.begin();
		it1!=node1.state_map.end();
		it1++) {
		string user = it1->first;
		Node_2& node2 = it1->second;

		for (auto it2=node2.state_map.begin();
			it2!=node2.state_map.end();
			it2++) {
		    Node_3& node3 = it2->second;
		    if (node3.phase==1) {
			node3.phase=2;
		//cout << "Changing state to 2" << endl;
		//cout << "srcDst" << srcDst << endl;
		//cout << "user: " << user << endl;
		//cout << "port: " << port << endl;
		//cout << "OK" << endl;
		//cout << node3.phase << endl;
		//cin >> k;

		    }
		}

		Node_3& node3 = node2.default_state;

		if (node3.phase==1) {
		    node3.phase=2;
		    //cout << "user: " << user << endl;
		    //cout << "port: " << port << endl;
		    //cout << "OK" << endl;
		    //cin >> k;
		}
	    }
	}
	if (command=="BYE") {
	    stringstream converter;
	    converter << srcIP << dstIP << udph->source << udph->dest << callID;
	    string srcDst = converter.str();

	    auto it = srcDstState.state_map.find(srcDst);
	    if (it==srcDstState.state_map.end()) {
		it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
	    }

	    Node_1& node1 = it->second;

	    for (auto it1=node1.state_map.begin();
		it1!=node1.state_map.end();
		it1++) {
		string user = it1->first;
		Node_2& node2 = it1->second;

		for (auto it2=node2.state_map.begin();
		    it2!=node2.state_map.end();
		    it2++) {
		    string port = it2->first;
		    Node_3& node3 = it2->second;

		    if (node3.phase==3) {
			node3.phase=4;

//			auto itu = user_state.state_map.find(user);
//			if (itu==user_state.state_map.end()) {
//			    itu = user_state.state_map.insert(
//				{user,user_state.default_state}
//			    ).first;
//			}
//			DataChannel& dataChannel = itu->second;
//
//			stringstream converter;
//			converter << srcIP << port;
//			string srcIpPort = converter.str();
//
//			auto itc = dataChannel.state_map.find(srcIpPort);
//			if (itc==dataChannel.state_map.end()) {
//			    itc = dataChannel.state_map.insert(
//				{srcIpPort, dataChannel.default_state}
//				).first;
//			}
//			itc->second.state = 0;
		    }
		}
	    }
	}
    } 
  }
}

void _output_result() {
  printf("Processed %ld packets. \n", packet_cnt);
  for (auto it = user_state.state_map.begin();
    it != user_state.state_map.end();
    it++) {
    cout << "User :" << it->first << endl;
    cout << "Call :" << it->second.call_num << endl;
  }
}

static void close() {
  //158.130.56.11 = 188252830
  //158.130.56.12 = 205030046
  _output_result();
  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 100 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  _update_state(packet);
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


