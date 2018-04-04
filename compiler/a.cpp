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
struct Node_re0_leaf{
    int state_re0 = 0;
};
// x
struct Node_re0_x{
    unordered_map<int, Node_re0_leaf> state_map;
    Node_re0_leaf default_state;
};
Node_re0_x *node_re0_x = new Node_re0_x();

bool test1_update(Packet *last) {
    unordered_map<int, Node_re0_leaf>::iterator it_re0_x;
    Node_re0_leaf *node_re0_leaf;

    it_re0_x = node_re0_x->state_map.find(src);
    if (it_re0_x == node_re0_x->state_map.end()) { 
	it_re0_x = node_re0_x->state_map.insert({src, node_re0_x->default_state}).first;
    }
    node_re0_leaf = &(node_re0_x->default_state);

    switch (node_re0_leaf->state_re0) {
	default:
	    node_re0_leaf->state_re0 = -1;
	    break;
    }
    for (it_re0_x = node_re0_x->state_map.begin(); it_re0_x != node_re0_x->state_map.end(); it_re0_x++) {
	if (true && it_re0_x->first != src) {
	    node_re0_leaf = &(it_re0_x->second);

	    switch (node_re0_leaf->state_re0) {
		default:
		    node_re0_leaf->state_re0 = -1;
		    break;
	    }
	}
    }
    return true;
}

bool test1_eval(int x, Packet* last) {
    unordered_map<int, Node_re0_leaf>::iterator it_re0_x;
    Node_re0_leaf *node_re0_leaf;

    it_re0_x = node_re0_x->state_map.find(x);
    if (it_re0_x == node_re0_x->state_map.end()) { 
	node_re0_leaf = &node_re0_x->default_state;
    } else {
	node_re0_leaf = &(it_re0_x->second);

    }
    bool ret_re0;
    switch (node_re0_leaf->state_re0) {
	case 0: 
	    ret_re0 = true;
	    break;
	default:
	    ret_re0 = false;
    }
    return ret_re0;
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
