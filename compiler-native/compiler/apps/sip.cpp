#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include "../library/linux_compat.h"
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
//#include <regex>
//#include <sstream>

#define ETHERNET_LINK_OFFSET 14

using namespace std;
FILE *file = NULL;
int sockfd = 0;
struct sockaddr_in server_addr;
int addr_len = sizeof(server_addr);
long packet_cnt = 0;
bool debug_mode = false;

// level
struct Node_3 {
	int phase;
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

struct DataChannelState {
	int state;
};

// ip+port
struct DataChannel {
	int total;
	unordered_map<string, DataChannelState> state_map;
	DataChannelState default_state;
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
		if (0 == strncmp(token, "Call-ID", 7)) {
			token = strtok(NULL, "<>:\n\"");
			callID = token;
			break;
		}
		token = strtok(NULL, "<>:\n\"");
	}
}

void getInfo(char* payload) {
	char* token;
	token = strtok(payload, "<>:\n\"");

	while (token != NULL) {
		if (0 == strncmp(token, "Call-ID", 7)) {
			token = strtok(NULL, "<>:\n\"");
			callID = token;
		} else if (0 == strncmp(token, "From", 4)) {
			cout << token << endl;
			token = strtok(NULL, "<>:\n\"");
			token = strtok(NULL, "<>:\n\"");
			user = token;
		} else if (0 == strncmp(token, "m=", 2)) {
			char* save = strtok(token, " ");
			save = strtok(NULL, " ");
			port = save;
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

void getSIPHeader(u_char* u_payload) {
	char* payload = (char*) u_payload;
	// response
	if (payload[0]=='S' && payload[1]=='I') {
		char response[4];
		for (int i=0; i<3; i++)
		response[i] = payload[i+8];
		response[3] = '\0';

		command = response;
		getCallID(payload);
	} else if (payload[0]=='I' && payload[1]=='N') {
		// INVITE
		fprintf(file, "INVITE\n");
		fflush(file);
		command = "INVITE";
		getInfo(payload);
	} else if (payload[0]=='A' && payload[1]=='C') {
		command = "ACK";
		fprintf(file, "ACK\n");
		fflush(file);
		getCallID(payload);
	} else if (payload[0]=='B' && payload[1]=='Y') {
		fprintf(file, "BYE\n");
		fflush(file);
		command = "BYE";
		getInfo((char*)payload);
	} else if (payload[0]=='R' && payload[1]=='E') {
		command = "REGISTER";
	}
}

void _update_state(u_char *packet) {
	int k;
	struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
	unsigned long srcIP = iph->ip_src.s_addr;
	unsigned long dstIP = iph->ip_dst.s_addr;

	if (iph->ip_p != IPPROTO_UDP) {
		return;
	}

	struct lin_udphdr* udph = (struct lin_udphdr*)(packet + sizeof(struct lin_ip) + ETHERNET_LINK_OFFSET);

	if (udph->source == 50195 || udph->dest == 50195) {
		u_char* payload = (u_char *)(packet + ETHERNET_LINK_OFFSET + iph->ip_hl*4 + sizeof(struct lin_udphdr));
		getSIPHeader(payload);

		if (command=="INVITE") {
			stringstream converter;
			converter << srcIP << dstIP << udph->source << udph->dest << callID;
			string srcDst = converter.str();

			auto it = srcDstState.state_map.find(srcDst);
			if (it == srcDstState.state_map.end()) {
				it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
			}
			Node_1& node1 = it->second;

			auto it1 = node1.state_map.find(user);
			if (it1 == node1.state_map.end()) {
				it1 = node1.state_map.insert(pair<string, Node_2>(user, node1.default_state)).first;
			}
			Node_2& node2 = it1->second;

			auto it2 = node2.state_map.find(port);
			if (it2 == node2.state_map.end()) {
				it2 = node2.state_map.insert(pair<string, Node_3>(port, node2.default_state)).first;
			}
			Node_3& node3 = it2->second;

			node3.phase = 1;
		}

		if (command == "ACK") {
			stringstream converter;
			converter << srcIP << dstIP << udph->source << udph->dest << callID;
			string srcDst = converter.str();

			auto it = srcDstState.state_map.find(srcDst);
			if (it == srcDstState.state_map.end()) {
				it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
			}
			Node_1& node1 = it->second;

			for (auto it1=node1.state_map.begin(); it1!=node1.state_map.end(); it1++) {
				string user = it1->first;
				Node_2& node2 = it1->second;

				for (auto it2 = node2.state_map.begin(); it2!=node2.state_map.end(); it2++) {
					string port = it2->first;
					Node_3& node3 = it2->second;

					if (node3.phase == 2) {
						node3.phase = 3;

						auto itu = user_state.state_map.find(user);
						if (itu == user_state.state_map.end()) {
							itu = user_state.state_map.insert({user,user_state.default_state}).first;
						}
						DataChannel& dataChannel = itu->second;

						stringstream converter;
						converter << srcIP << port;
						string srcIpPort = converter.str();

						auto itc = dataChannel.state_map.find(srcIpPort);
						if (itc==dataChannel.state_map.end()) {
							itc = dataChannel.state_map.insert( {srcIpPort, dataChannel.default_state}).first;
						}
						DataChannelState& dataChannelState = itc->second;

						dataChannelState.state = 1;
					} 
				}
			}
		}

		if (command == "200") {

			printf("200\n");
			stringstream converter;
			converter << dstIP << srcIP << udph->dest << udph->source << callID;
			string srcDst = converter.str();

			auto it = srcDstState.state_map.find(srcDst);
			if (it == srcDstState.state_map.end()) {
				it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
			}
			Node_1& node1 = it->second;

			for (auto it1=node1.state_map.begin(); it1!=node1.state_map.end(); it1++) {
				string user = it1->first;
				Node_2& node2 = it1->second;

				for (auto it2=node2.state_map.begin(); it2!=node2.state_map.end(); it2++) {
					Node_3& node3 = it2->second;
					if (node3.phase == 1) {
						node3.phase=2;
					}
				}

				Node_3& node3 = node2.default_state;

				if (node3.phase == 1) {
					node3.phase=2;
				}
			}
		}

		if (command == "BYE") {
			stringstream converter;
			converter << srcIP << dstIP << udph->source << udph->dest << callID;
			string srcDst = converter.str();

			auto it = srcDstState.state_map.find(srcDst);
			if (it == srcDstState.state_map.end()) {
				it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
			}

			Node_1& node1 = it->second;

			for (auto it1=node1.state_map.begin(); it1!=node1.state_map.end(); it1++) {
				string user = it1->first;
				Node_2& node2 = it1->second;

				for (auto it2=node2.state_map.begin(); it2!=node2.state_map.end(); it2++) {
					string port = it2->first;
					Node_3& node3 = it2->second;

					if (node3.phase == 3) {
						node3.phase = 4;
						auto itu = user_state.state_map.find(user);
						if (itu==user_state.state_map.end()) {
							itu = user_state.state_map.insert({user,user_state.default_state}).first;
						}
						DataChannel& dataChannel = itu->second;

						stringstream converter;
						converter << srcIP << port;
						string srcIpPort = converter.str();

						auto itc = dataChannel.state_map.find(srcIpPort);
						if (itc == dataChannel.state_map.end()) {
							itc = dataChannel.state_map.insert({srcIpPort, dataChannel.default_state}).first;
						}
						itc->second.state = 0;
					}
				}
			}
		}
	} else {
		stringstream converter;

		converter << srcIP << ntohs(udph->source);
		string srcIpPort = converter.str();

		for (auto it=user_state.state_map.begin(); it!=user_state.state_map.end(); it++) {
			DataChannel& dataChannel = it->second;

			auto itc = dataChannel.state_map.find(srcIpPort);
			if (itc == dataChannel.state_map.end()) {
				itc = dataChannel.state_map.insert({srcIpPort, dataChannel.default_state}).first;
			}
			DataChannelState& dataChannelState = itc->second;

			if (dataChannelState.state == 1) {
				dataChannel.total += ntohs(iph->ip_len) + ETHERNET_LINK_OFFSET;
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
		cout << "Usage :" << it->second.total << endl;
	}
}

static void close() {
	//158.130.56.11 = 188252830
	//158.130.56.12 = 205030046
	fflush(file);
	fclose(file);
	_output_result();
	printf("Exit now.\n");
}

int alert_count_threshold = 1000;

void _check_state(u_char *packet) {
	int k;
	struct lin_ip* iph = (struct lin_ip*) (packet + ETHERNET_LINK_OFFSET);
	unsigned long srcIP = iph->ip_src.s_addr;
	unsigned long dstIP = iph->ip_dst.s_addr;

	if (iph->ip_p != IPPROTO_UDP) {
		return;
	}

	struct lin_udphdr* udph = (struct lin_udphdr*)(packet + sizeof(struct lin_ip) + ETHERNET_LINK_OFFSET);
	stringstream converter;
	converter << srcIP << dstIP << udph->source << udph->dest << callID;
	string srcDst = converter.str();

	auto it = srcDstState.state_map.find(srcDst);
	if (it == srcDstState.state_map.end()) {
		it = srcDstState.state_map.insert(pair<string, Node_1>(srcDst, srcDstState.default_state)).first;
	}
	Node_1& node1 = it->second;

	for (auto it1=node1.state_map.begin(); it1!=node1.state_map.end(); it1++) {
		string user = it1->first;
		Node_2& node2 = it1->second;

		for (auto it2 = node2.state_map.begin(); it2!=node2.state_map.end(); it2++) {
			string port = it2->first;
			Node_3& node3 = it2->second;

			if (node3.phase == 2) {
				node3.phase = 3;

				auto itu = user_state.state_map.find(user);
				if (itu == user_state.state_map.end()) {
					itu = user_state.state_map.insert({user,user_state.default_state}).first;
				}
				DataChannel& dataChannel = itu->second;


				if (dataChannel.total > alert_count_threshold) {
					struct sockaddr_in sa, da;
					char src_str[INET_ADDRSTRLEN], dst_str[INET_ADDRSTRLEN];
					sa.sin_addr.s_addr = srcIP;
					da.sin_addr.s_addr = dstIP;
					inet_ntop(AF_INET, &(sa.sin_addr), src_str, INET_ADDRSTRLEN);
					inet_ntop(AF_INET, &(da.sin_addr), dst_str, INET_ADDRSTRLEN);
					fprintf(file, "block traffic from %s to %s because traffic is %u\n", src_str, dst_str, dataChannel.total);

					char buffer[100];
					int n = sprintf(buffer, "%s %s\r\n", src_str, dst_str);
					sendto(sockfd, buffer, n, 0, (sockaddr *)&server_addr, (socklen_t)addr_len);
				}
			}
		}
	}
}



static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
	packet_cnt += 1;
	if (packet_cnt % 100 == 0 && debug_mode) {
		printf("In progress: %ld packets\n", packet_cnt);
	}
	_update_state(packet);
	_check_state(packet);
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

	file = fopen("sip.log", "w");

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


