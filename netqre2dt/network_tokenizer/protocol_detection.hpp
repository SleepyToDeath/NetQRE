#ifndef PROTOCOL_DETECTION_HPP
#define PROTOCOL_DETECTION_HPP

#include "feature_vector.hpp"
#include <memory>
#include <iostream>

#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define LLC_LENGTH_BYTE 8

using std::shared_ptr;
using std::cout;

void packetHandler_pd(u_char* arg, const struct pcap_pkthdr *pkthdr, u_char *packet);

class DetectionParser {
	public:

	std::shared_ptr<TokenStream> stream;
	int llc_len;
	int pkt_count;
	int protocol_bits;
	int granularity;

	std::shared_ptr<TokenStream> parse_pcap(std::string file_name, bool with_llc, int protocol_bits, int granularity) {

		stream = shared_ptr<TokenStream>(new TokenStream());
		llc_len = with_llc?LLC_LENGTH_BYTE:0;
		this->protocol_bits = protocol_bits;
		this->granularity = granularity;
		pkt_count = 0;

		char *dev, errbuf[PCAP_ERRBUF_SIZE];
		pcap_t *handle;
		struct pcap_pkthdr header;	// The header that pcap gives us 
		const u_char *packet;		 // The actual packet 
		handle = pcap_open_offline(file_name.c_str(), errbuf);
		if (handle == NULL) {
			throw (std::string("Couldn't open file ") + file_name + " : " + errbuf + "\n" );
		}
		// start packet processing loop
		const clock_t begin_time = clock();
		if (pcap_loop(handle, 0, (pcap_handler)packetHandler_pd, NULL) < 0) {
			throw (std::string("pcap_loop() failed : ") + pcap_geterr(handle) + "\n" );
		}
		// close the session
		pcap_close(handle);

		return stream;

	}

	void parse_packet(u_char* pkt) {
		/* parse packet */
		pkt_count ++;
		const struct ether_header* eth_hdr;
		const struct ip* ip_hdr;
		const struct tcphdr* tcp_hdr;
		eth_hdr = (struct ether_header*) pkt;
		u_char* ptr;
//		cout<<sizeof(struct ether_header)<<endl;
//		cout<<sizeof(struct ip)<<endl;
//		cout<<sizeof(struct tcphdr)<<endl;
		ip_hdr = (struct ip*)(pkt + sizeof(struct ether_header) + llc_len);
//		std::cout<< (int)(ip_hdr->ip_p) <<std::endl;
		if (ip_hdr->ip_p == IPPROTO_TCP) 
		{
			tcp_hdr = (tcphdr*)(pkt + sizeof(struct ether_header) + llc_len + sizeof(struct ip));
			ptr = ((u_char*)tcp_hdr) + sizeof(struct tcphdr);
		}
		else
		{
			tcp_hdr = NULL;
			ptr = pkt + sizeof(struct ether_header) + llc_len + sizeof(struct ip) + sizeof(struct udphdr);
		}

//		cout<<endl;

		/* extract feature */
		FeatureVector fv;
		for (int i=0; i<protocol_bits/granularity; i++)
		{
			if (granularity < 8)
			{
				unsigned int mask = ((1 << granularity) - 1) << (i*granularity%8);
				FeatureSlot field(32, true, ptr[i] & mask); // 0
				fv.push_back(field);
			}
			else if (granularity == 8)
			{
				FeatureSlot field(32, true, ptr[i]); // 0
				fv.push_back(field);
			}
			else
			{
				unsigned int mask = ((1 << granularity) - 1);
				FeatureSlot field(32, true, (*((unsigned int*)(ptr + i*granularity/8))) & mask); // 0
				fv.push_back(field);
			}
		}

		stream->push_back(fv);
	}
};

std::shared_ptr<DetectionParser> the_detection_parser;

void packetHandler_pd(u_char* arg, const struct pcap_pkthdr *pkthdr, u_char *packet) { 
	the_detection_parser->parse_packet(packet);
}


#endif





