#include <stdio.h>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#define __FAVOR_BSD
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <cstring>
#include <unordered_map>

using namespace std;
int totalPackets =0;
long length = 0;
double throughput = 0.0;

// leaf level
struct Node_4{
	int sum_0x203ed40 = 0;
	int state_0x203ec40 = 0;
	int state_0xad7b00 = 0xada610;
};
// dstport
struct Node_3 {
	unordered_map<u_int, Node_4> state_map;
	Node_4 default_state;
};
// srcport
struct Node_2 {
	unordered_map<u_int, Node_3> state_map;
	Node_3 default_state;
};
// dstip
struct Node_1 {
	unordered_map<char*, Node_2> state_map;
	Node_2 default_state;
};
// srcip
struct Node_0 {
	unordered_map<char*, Node_1> state_map;
	int total_sum = 0;
	Node_1 default_state;
};
Node_0 state;

int ret_sum_0x203ed40 = 0;
bool ret_state_0xad7b00 = false;


u_char* filter_tcp(char* srcip, char* dstip, u_int srcport, u_int dstport, u_char* last) {
	return (ret_state_0xad7b00) ? last : 0;
}

bool filter_tcp_update(u_char *last) {
	return true;
}

int count_flow(char* srcip, char* dstip, u_int srcport, u_int dstport, u_char* last) {
	return state.total_sum;
}


void packetHandler(u_char* arg, const struct pcap_pkthdr *pkthdr, u_char *packet) { 
	totalPackets++;	//keep packet count
	const struct ether_header* ethernetHeader;
	const struct ip* ipHeader;
	const struct tcphdr* tcpHeader;
	char sourceIp[INET_ADDRSTRLEN];
	char destIp[INET_ADDRSTRLEN];
	char buffer[100];
	u_int sourcePort, destPort;
	u_char *data;
	u_char flags;
	string dataStr = "";
	ipHeader = (struct ip*)(packet + sizeof(struct ether_header));
	inet_ntop(AF_INET, &(ipHeader->ip_src), sourceIp, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(ipHeader->ip_dst), destIp, INET_ADDRSTRLEN);
	if (ipHeader->ip_p == IPPROTO_TCP) {
		tcpHeader = (tcphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip));
		sourcePort = ntohs(tcpHeader->th_sport);
		destPort = ntohs(tcpHeader->th_dport);
		length += pkthdr->len;
		//code from count_flow_update(...)
		unordered_map<char*, Node_1>::iterator it0 = state.state_map.find(sourceIp);
		if (it0 == state.state_map.end()) { 
			it0 = state.state_map.insert({sourceIp, state.default_state}).first;
		}
		Node_1& state_1 = it0->second;
		auto it1 = state_1.state_map.find(destIp);
		if (it1 == state_1.state_map.end()) { 
			it1 = state_1.state_map.insert({destIp, state_1.default_state}).first;
		}
		Node_2& state_2 = it1->second;
		auto it2 = state_2.state_map.find(sourcePort);
		if (it2 == state_2.state_map.end()) { 
			it2 = state_2.state_map.insert({sourcePort, state_2.default_state}).first;
		}
		Node_3& state_3 = it2->second;
		auto it3 = state_3.state_map.find(destPort);
		if (it3 == state_3.state_map.end()) { 
			it3 = state_3.state_map.insert({destPort, state_3.default_state}).first;
		}
		Node_4& state_4 = it3->second;
		if (state_4.state_0xad7b00 == 0xada610) {
			state_4.state_0xad7b00 = 0xad9d20;
		}

		if ((flags = tcpHeader->th_flags) & (TH_SYN|TH_FIN)) {
			if (flags & TH_SYN) {
				if (state_4.state_0x203ec40 == 0) {
					state_4.state_0x203ec40 = 1;
				}
				if (state_4.state_0x203ec40 == 1) {
					state_4.state_0x203ec40 = 1;
				}
			}
			if (flags & TH_FIN) {
				if (state_4.state_0x203ec40 == 0) {
					state_4.state_0x203ec40 = 0;
				}
				if (state_4.state_0x203ec40 == 1) {
					state_4.state_0x203ec40 = 2;
					state_4.sum_0x203ed40+=1;
					state_4.state_0x203ec40 = 0;
					state.total_sum += 1;
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	char *dev, errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;
	struct pcap_pkthdr header;	// The header that pcap gives us 
	const u_char *packet;		 // The actual packet 
	string fileName = "/home/yifei/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap";//"bigFlows.pcap";//"/home/ankit/test2.pcap";//"mycap.pcap";
	cout<<"File: "<<fileName<<endl;
	handle = pcap_open_offline(fileName.c_str(), errbuf);
	if (handle == NULL) {
		cout<<"Couldn't open file "<<fileName<<" : "<<errbuf<<endl;
		return(2);
	}
	// start packet processing loop
	const clock_t begin_time = clock();
	if (pcap_loop(handle, 0, (pcap_handler)packetHandler, NULL) < 0) {
		cout<<"pcap_loop() failed : "<<pcap_geterr(handle)<<endl;
		return 1;
	}
	// close the session
	//post process packets

	pcap_close(handle);
	cout<<"capture finished with "<<totalPackets<<" packets\n";
	/*cout<<"The SYN count is as follows - "<<endl;
	for (const auto &element : srcDestSynSeen) {
		cout << element.first << " = " << element.second << '\n';
	}*/
	/* cout<<"The connection count is as follows - "<<endl;
	for (const auto &element : srcDestConnectionCount) {
		cout << element.first << " = " << element.second << '\n';
	} */
	float timeTaken = (float( clock () - begin_time ) /  CLOCKS_PER_SEC);
	cout<<"total length : "<<length<<" bytes"<<endl;
	cout<<"throughput : "<<(length/timeTaken/1000)<<" kbps"<<endl;
	cout<<"time taken : "<<timeTaken<<" sec"<<endl;
	return 0;
}

