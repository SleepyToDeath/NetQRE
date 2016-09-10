#ifndef NETQRE_H
#define NETQRE_H

#include <vector>
#include "linux_compat.h"

#define ETHERNET_LINK_OFFSET 14

static int l2offset = 0;

typedef std::vector<u_char*> PKT_QUEUE;

void update(u_char * packet);
void close();
 

#endif 
