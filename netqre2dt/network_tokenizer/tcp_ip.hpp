#ifndef TCP_IP_HPP
#define TCP_IP_HPP

#include "feature_vector.hpp"
#include <memory>

#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

void packetHandler(u_char* arg, const struct pcap_pkthdr *pkthdr, u_char *packet);

class TcpIpParser {
	public:

	std::shared_ptr<TokenStream> stream;
	int pkt_count;

	std::shared_ptr<TokenStream> parse_pcap(std::string file_name) {

		stream = shared_ptr<TokenStream>(new TokenStream());
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
		if (pcap_loop(handle, 0, (pcap_handler)packetHandler, NULL) < 0) {
			throw (std::string("pcap_loop() failed : ") + pcap_geterr(handle) + "\n" );
		}
		// close the session
		pcap_close(handle);

		return stream;

	}

	void parse_packet(u_char* pkt) {
		/* parse packet */
		char tmp[65];
		pkt_count ++;
		const struct ether_header* eth_hdr;
		const struct ip* ip_hdr;
		const struct tcphdr* tcp_hdr;
		eth_hdr = (struct ether_header*) pkt;
		cout<<sizeof(struct ether_header)<<endl;
		cout<<sizeof(struct ip)<<endl;
		cout<<sizeof(struct tcphdr)<<endl;
		ip_hdr = (struct ip*)(pkt + sizeof(struct ether_header));
		std::cout<< (int)(ip_hdr->ip_p) <<std::endl;
		if (ip_hdr->ip_p == IPPROTO_TCP) 
			tcp_hdr = (tcphdr*)(pkt + sizeof(struct ether_header) + sizeof(struct ip));
		else
			tcp_hdr = NULL;

		cout<<endl;

		/* extract feature */
		FeatureVector fv;
		FeatureSlot src_ip(32, true, ip_hdr->ip_src.s_addr);
		FeatureSlot dst_ip(32, true, ip_hdr->ip_dst.s_addr);

		fv.push_back(src_ip);
		fv.push_back(dst_ip);

		if (tcp_hdr!=NULL)
		{
			FeatureSlot src_port(1, true, tcp_hdr->th_sport);
			FeatureSlot dst_port(1, true, tcp_hdr->th_dport);
			FeatureSlot syn_flag(1, false, tcp_hdr->syn);
			FeatureSlot ack_flag(1, false, tcp_hdr->ack);
			fv.push_back(src_port);
			fv.push_back(dst_port);
			fv.push_back(syn_flag);
			fv.push_back(ack_flag);
		}
		else
		{
			FeatureSlot src_port(1, true, 0);
			FeatureSlot dst_port(1, true, 0);
			FeatureSlot syn_flag(1, false, 0);
			FeatureSlot ack_flag(1, false, 0);
			fv.push_back(src_port);
			fv.push_back(dst_port);
			fv.push_back(syn_flag);
			fv.push_back(ack_flag);
		}

		stream->push_back(fv);
	}
};

std::shared_ptr<TcpIpParser> the_tcp_ip_parser;

void packetHandler(u_char* arg, const struct pcap_pkthdr *pkthdr, u_char *packet) { 
	the_tcp_ip_parser->parse_packet(packet);
}


#include <iostream>

void test_tcp_ip_parser(std::string file_name)
{
	the_tcp_ip_parser = shared_ptr<TcpIpParser>(new TcpIpParser());
	auto stream = the_tcp_ip_parser->parse_pcap(file_name);
	for (size_t i=0; i<stream->size(); i++)
	{
		cout<<"Pkt#"<<i<<": \n";
		for (size_t j=0; j<(*stream)[i].size(); j++)
		{
			cout<<(*stream)[i][j].size<<" ";
			cout<<(*stream)[i][j].iterative<<" ";
			cout<<(*stream)[i][j].value<<"\n";
		}
	}
}

#endif




/* paste here for reference */
#if false

struct	ether_header {
	u_int8_t  ether_dhost[ETHER_ADDR_LEN];
	u_int8_t  ether_shost[ETHER_ADDR_LEN];
	u_int16_t ether_type;
} __attribute__((__packed__));

struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;		/* header length */
    unsigned int ip_v:4;		/* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_v:4;		/* version */
    unsigned int ip_hl:4;		/* header length */
#endif
    uint8_t ip_tos;			/* type of service */
    unsigned short ip_len;		/* total length */
    unsigned short ip_id;		/* identification */
    unsigned short ip_off;		/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
    uint8_t ip_ttl;			/* time to live */
    uint8_t ip_p;			/* protocol */
    unsigned short ip_sum;		/* checksum */
    struct in_addr ip_src, ip_dst;	/* source and dest address */
};

typedef uint32_t in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};

struct tcphdr
{
    __extension__ union
    {
      struct
      {
	uint16_t th_sport;	/* source port */
	uint16_t th_dport;	/* destination port */
	tcp_seq th_seq;		/* sequence number */
	tcp_seq th_ack;		/* acknowledgement number */
# if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t th_x2:4;	/* (unused) */
	uint8_t th_off:4;	/* data offset */
# endif
# if __BYTE_ORDER == __BIG_ENDIAN
	uint8_t th_off:4;	/* data offset */
	uint8_t th_x2:4;	/* (unused) */
# endif
	uint8_t th_flags;
# define TH_FIN	0x01
# define TH_SYN	0x02
# define TH_RST	0x04
# define TH_PUSH	0x08
# define TH_ACK	0x10
# define TH_URG	0x20
	uint16_t th_win;	/* window */
	uint16_t th_sum;	/* checksum */
	uint16_t th_urp;	/* urgent pointer */
      };
      struct
      {
	uint16_t source;
	uint16_t dest;
	uint32_t seq;
	uint32_t ack_seq;
# if __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t res1:4;
	uint16_t doff:4;
	uint16_t fin:1;
	uint16_t syn:1;
	uint16_t rst:1;
	uint16_t psh:1;
	uint16_t ack:1;
	uint16_t urg:1;
	uint16_t res2:2;
# elif __BYTE_ORDER == __BIG_ENDIAN
	uint16_t doff:4;
	uint16_t res1:4;
	uint16_t res2:2;
	uint16_t urg:1;
	uint16_t ack:1;
	uint16_t psh:1;
	uint16_t rst:1;
	uint16_t syn:1;
	uint16_t fin:1;
# else
#  error "Adjust your <bits/endian.h> defines"
# endif
	uint16_t window;
	uint16_t check;
	uint16_t urg_ptr;
      };
    };
};

#endif



