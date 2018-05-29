#include <unordered_map>
#include <iostream>
#include <tuple>

using namespace std;

typedef int IP;

struct Packet {
    int src = 0;
    int dst = 0;
};
// leaf
struct Node_agg0_leaf{
int state_re0 = 0;
};
// x
struct Node_agg0_x{
int sum_agg0 = 0;
unordered_map<int, Node_agg0_leaf> state_map;
Node_agg0_leaf default_state;
};
Node_agg0_x *node_agg0_x = new Node_agg0_x();

bool allip_update(Packet *last) {
unordered_map<int, Node_agg0_leaf>::iterator it_agg0_x;
Node_agg0_leaf *node_agg0_leaf;

it_agg0_x = node_agg0_x->state_map.find(srcip);
if (it_agg0_x == node_agg0_x->state_map.end()) { 
it_agg0_x = node_agg0_x->state_map.insert({srcip, node_agg0_x->default_state}).first;
}
if (true) {
node_agg0_leaf = &(it_agg0_x->second);

switch (node_agg0_leaf->state_re0) {
case 0: 
node_agg0_leaf->state_re0 = 1;
node_agg0_x->sum_agg0 += 1 - 0;
break;
case 1: 
node_agg0_leaf->state_re0 = 1;
break;
default:
node_agg0_leaf->state_re0 = -1;
break;
}
}
return true;
}

int allip_eval(Packet* last) {
unordered_map<int, Node_agg0_leaf>::iterator it_agg0_x;
Node_agg0_leaf *node_agg0_leaf;

it_agg0_x = node_agg0_x->state_map.find(x);
if (it_agg0_x == node_agg0_x->state_map.end()) { 
node_agg0_leaf = &node_agg0_x->default_state;
} else {
node_agg0_leaf = &(it_agg0_x->second);

}
int ret_agg0 = node_agg0_leaf->sum_agg0;
return ret_agg0;
}

Packet pkt;

int main() {
    while (true) {
	cout << "src = ";
	cin >> pkt.src;
	
	cout << "dst = ";
	cin >> pkt.dst;

	smain_update(&pkt);
	cout << "result is: \t" << smain_eval(&pkt) << endl;
    }
    return 1;
}
