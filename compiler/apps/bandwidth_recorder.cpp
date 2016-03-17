#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux_compat.h>
#include <unordered_map>

#define ETHERNET_LINK_OFFSET 14

FILE *file = NULL;
long packet_cnt = 0;
bool debug_mode = false;

long bucket_bytes = 0;
long bucket_time_ms = 0; 

static void close() {
  fclose(file);
  fflush(file);
  printf("Processed %ld packets\n", packet_cnt);
  printf("Exit now.\n");
}

static void handleCapturedPacket(u_char* arg, const struct pcap_pkthdr *header, u_char *packet) { 
  packet_cnt += 1;
  if (packet_cnt % 1 == 0 && debug_mode) {
    printf("In progress: %ld packets\n", packet_cnt);
  }
  long time_ms = header->ts.tv_sec*1000 + header->ts.tv_usec/1000;

  if (bucket_time_ms == 0) {
    bucket_time_ms = time_ms/100*100;
  } else {
    while (time_ms - bucket_time_ms >= 100) {
      fprintf(file, "%ld %ld\n", bucket_time_ms, bucket_bytes*8); // bits from srcIP to dstIP in the last 100 ms
      bucket_bytes = 0;
      bucket_time_ms += 100;

      if (bucket_time_ms % 1000 == 0)
        fflush(file);
    }
  }
  bucket_bytes += header->caplen;
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

  file = fopen("bandwidth_record.log", "w");

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
