#include "netqre.h"

#include "common.h"
#include "taskSuperSpreaders.h"
#include "dataPlane.h"

using namespace std;

TaskSuperSpreaders ss;
DataPlane *dp;
Packet p;

void init() {
  dp = new DataPlane();
  ss.getHashSeedsFromDataPlane(*dp);

  int field1 = FIELD_SRCIP;
  int numRows = 3;
  int countersPerRow = 109226;

  int field2 = FIELD_DSTIP;
  int numBits = 46;

  ss.setUserPreferencesDirectly(field1, numRows, countersPerRow,\
                                field2, numBits, 0, 44.83/200);

  ss.configureDataPlane(*dp);
  dp->getHashByField();
}

void close() {
  printf("done.\n");
  ss.updateCountersFromDataPlane(*dp);

  printf("%d: %u\n", 34435, ss.queryGivenKey(34435));

  // max src
  printf("%lu: %lu\n", 3014787072, ss.queryGivenKey(3014787072));
}

void update(u_char *packet) {
  struct lin_ip* iph = (struct lin_ip*) (packet + l2offset);
  unsigned long srcIP = iph->ip_src.s_addr;
  unsigned long dstIP = iph->ip_dst.s_addr;

  p.srcip = srcIP;
  p.dstip = dstIP;
  p.srcport = 0;
  p.dstport = 0;
  p.proto = 0;

  //os_dietz_thorup32(x, tmp.range, dp->getHashA, dp->getHashB);
  //printf("Calling dp to process packet\n");
  dp->processPacket(p, ss.getTaskId());
}



