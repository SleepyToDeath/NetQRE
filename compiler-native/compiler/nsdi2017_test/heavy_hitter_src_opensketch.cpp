#include "netqre.h"

#include "common.h"
#include "taskHeavyHitters.h"
#include "dataPlane.h"

using namespace std;

TaskHeavyHitters hh;
DataPlane *dp;
Packet p;

void init() {
  dp = new DataPlane();
  hh.getHashSeedsFromDataPlane(*dp);

  int field = FIELD_SRCIP;
  int numRows = 3;
  int countersPerRow = 10000;

  hh.setUserPreferencesDirectly(field, numRows, countersPerRow);
  hh.configureDataPlane(*dp);
  dp->getHashByField();
}

void close() {
  hh.updateCountersFromDataPlane(*dp);
  inet_aton("219.46.141.122", &tmp);

  printf("%s or %d: %d\n", "219.46.141.122", tmp.s_addr, hh.queryGivenKey(tmp.s_addr));
  printf("%d: %d\n", 2056072923, hh.queryGivenKey(2056072923));

  printf("done.\n");
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
  dp->processPacket(p, hh.getTaskId());
}



